#include "CpuTrace.h"
#include "Serializer.h"
#include <cassert>
#include <string>
#include <vector>

#include "../contrib/murmur3/murmur3.h"
#include "../contrib/murmur3/murmur3.c"

namespace
{
    using namespace CpuTrace;
    using namespace CpuTrace::Impl;

    size_t to_size_t(uint64_t value)
    {
        assert(value <= SIZE_MAX);
        return static_cast<size_t>(value);
    }

    class MemoryStream : public IStream
    {
    public:
        const uint8_t* getData() const
        {
            return mBuffer.data();
        }

        uint8_t* getData()
        {
            return mBuffer.data();
        }

        virtual void seek(uint64_t offset) override
        {
            mPos = offset;
            ensure(mPos + 1);
        }

        virtual uint64_t size() const override
        {
            return mBuffer.size();
        }

        virtual uint64_t pos() const override
        {
            return mPos;
        }

        virtual uint64_t write(const void* data, uint64_t size) override
        {
            ensure(mPos + size);
            memcpy(mBuffer.data() + mPos, data, to_size_t(size));
            mPos += size;
            return size;
        }

        virtual uint64_t read(void* data, uint64_t size) override
        {
            uint64_t capacity = static_cast<uint64_t>(mBuffer.size());
            uint64_t maxSize = capacity - mPos;
            if (size > maxSize)
                size = maxSize;
            memcpy(data, mBuffer.data() + mPos, to_size_t(size));
            return size;
        }

        virtual void flush() override
        {
        }

    private:
        virtual void ensure(uint64_t size)
        {
            if (size > static_cast<uint64_t>(mBuffer.size()))
            {
                mBuffer.resize(to_size_t(size), 0);
            }
        }

        std::vector<uint8_t>    mBuffer;
        uint64_t                mPos;
    };

    using namespace CpuTrace;

    static const uint32_t Magic = 0x20160729;

    enum class Command : uint8_t
    {
        Header,
        Footer,
        SetState,
        Execute,
        Interrupt,
        Signal,
        Read8,
        Read16,
        Read32,
        Write8,
        Write16,
        Write32,
    };

    template <size_t alignment>
    size_t alignUp(size_t value)
    {
        static_assert((alignment & (alignment - 1)) == 0, "Argument is not a power of 2");
        size_t mask = alignment - 1;
        return (value + mask) & ~mask;
    }

    template <size_t alignment>
    size_t divideUp(size_t value)
    {
        static_assert((alignment & (alignment - 1)) == 0, "Argument is not a power of 2");
        size_t mask = alignment - 1;
        return (value + mask) / alignment;
    }

    class Trace : public ITrace
    {
    public:
        std::vector<uint32_t>& getBuffer()
        {
            return mBuffer;
        }

        const std::vector<uint32_t>& getBuffer() const
        {
            return mBuffer;
        }

        void load(IStream& stream)
        {
            auto size = stream.size();
            mBuffer.resize(to_size_t(size), 0);
            stream.read(mBuffer.data(), size);
        }

        void save(IStream& stream) const
        {
            stream.write(mBuffer.data(), mBuffer.size());
        }

    private:
        struct Chunk
        {
            uint64_t            mOffset;
            size_t              mSize;
        };

        std::vector<Chunk>      mChunks;
        MemoryStream            mData;
        std::vector<uint32_t>   mBuffer;
    };

    class Capture : public ICapture
    {
    public:
        Capture(ICaptureDevice& device, Trace& trace)
            : mDevice(device)
            , mTrace(trace)
            , mInvalidated(true)
        {
            mTrace.getBuffer().clear();
            mTrace.getBuffer().reserve(1024 * 1024);

            mDevice.startCapture(*this);

            emit(Command::Header, Magic, CpuTrace::Version, device.getVersion(), device.getStateSize());

            mState.resize(mDevice.getStateSize(), 0);
        }

        ~Capture()
        {
            emit(Command::Footer);

            mDevice.stopCapture(*this);

            FILE* file = fopen(mPath.c_str(), "wb");
            if (file)
            {
                const auto& buffer = mTrace.getBuffer();
                fwrite(buffer.data(), sizeof(uint32_t), buffer.size(), file);
                fclose(file);
            }
        }

        virtual void invalidateState() override
        {
            mInvalidated = true;
        }

        virtual void execute() override
        {
            mDevice.getState(mState.data(), mState.size());
            if (mInvalidated)
            {
                emit(Command::SetState, static_cast<const void*>(mState.data()), mState.size());
                mInvalidated = false;
            }

            uint32_t hash[4];
            MurmurHash3_x64_128(mState.data(), static_cast<int>(mState.size()), 0, hash);
            emit(Command::Execute, hash[0], hash[1], hash[2], hash[3]);
        }

        virtual void interrupt(uint32_t type) override
        {
            emit(Command::Interrupt, type);
        }

        virtual void signal(uint32_t type) override
        {
            emit(Command::Signal, type);
        }

        virtual void read8(uint32_t addr, uint32_t value, uint32_t type) override
        {
            emit(CommandHeader::make(Command::Read8, 2, type));
            emitU32(addr);
            emitU32(value);
        }

        virtual void read16(uint32_t addr, uint32_t value, uint32_t type) override
        {
            emit(CommandHeader::make(Command::Read16, 2, type));
            emitU32(addr);
            emitU32(value);
        }

        virtual void read32(uint32_t addr, uint32_t value, uint32_t type) override
        {
            emit(CommandHeader::make(Command::Read32, 2, type));
            emitU32(addr);
            emitU32(value);
        }

        virtual void write8(uint32_t addr, uint32_t value, uint32_t type) override
        {
            emit(CommandHeader::make(Command::Write8, 2, type));
            emitU32(addr);
            emitU32(value);
        }

        virtual void write16(uint32_t addr, uint32_t value, uint32_t type) override
        {
            emit(CommandHeader::make(Command::Write16, 2, type));
            emitU32(addr);
            emitU32(value);
        }

        virtual void write32(uint32_t addr, uint32_t value, uint32_t type) override
        {
            emit(CommandHeader::make(Command::Write32, 2, type));
            emitU32(addr);
            emitU32(value);
        }

        Trace& getTrace()
        {
            return mTrace;
        }

    private:
        union CommandHeader
        {
            uint32_t        u32;
            struct
            {
                uint8_t     command;
                uint8_t     extra;
                uint16_t    blocks;
            }               fields;

            static CommandHeader make(Command command, size_t blocks = 0, uint32_t extra = 0)
            {
                CommandHeader header = { 0 };
                header.fields.command = static_cast<uint8_t>(command);
                header.fields.blocks = static_cast<uint16_t>(blocks);
                header.fields.extra = static_cast<uint8_t>(extra);
                return header;
            }
        };

        size_t blockCount(size_t size)
        {
            return divideUp<sizeof(uint32_t)>(size);
        }

        void emit(CommandHeader header)
        {
            mTrace.getBuffer().push_back(static_cast<uint32_t>(header.u32));
        }

        void emit(Command command)
        {
            emit(CommandHeader::make(command));
        }

        void emit(Command command, const void* data, size_t size)
        {
            auto count = blockCount(size);
            emit(CommandHeader::make(command, count));

            auto& buffer = mTrace.getBuffer();
            auto offset = buffer.size();
            buffer.resize(offset + count, 0);
            memcpy(buffer.data() + offset, data, size);
        }

        template <typename... TArgs>
        void emit(Command command, TArgs... args)
        {
            emit(CommandHeader::make(command, sizeof...(args)));
            emitU32(args...);
        }

        template <typename TArg0, typename... TArgs>
        void emitU32(TArg0 arg0, TArgs... args)
        {
            mTrace.getBuffer().push_back(static_cast<uint32_t>(arg0));
            emitU32(args...);
        }

        template <typename TArg>
        void emitU32(TArg value)
        {
            mTrace.getBuffer().push_back(static_cast<uint32_t>(value));
        }

        ICaptureDevice&         mDevice;
        Trace&                  mTrace;
        std::string             mPath;
        bool                    mInvalidated;
        std::vector<uint8_t>    mState;
    };

    class Context : public IContext
    {
    public:
        virtual ITrace& createTrace()
        {
            return *new Trace();
        }

        virtual void destroyTrace(ITrace& trace)
        {
            delete static_cast<Trace*>(&trace);
        }

        virtual void loadTrace(ITrace& trace, IStream& stream)
        {
            static_cast<Trace&>(trace).load(stream);
        }

        virtual void saveTrace(const ITrace& trace, IStream& stream)
        {
            static_cast<const Trace&>(trace).save(stream);
        }

        virtual ICapture& startCapture(ICaptureDevice& device, ITrace& trace) override
        {
            return *(new Capture(device, static_cast<Trace&>(trace)));
        }

        virtual void stopCapture(ICapture& capture) override
        {
            delete static_cast<Capture*>(&capture);
        }
    };
}

namespace CpuTrace
{
    IContext& createContext()
    {
        auto* context = new Context();
        return *context;
    }

    void destroyContext(IContext& context)
    {
        delete static_cast<Context*>(&context);
    }
}