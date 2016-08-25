#pragma once

#include <cstdint>

namespace CpuTrace
{
    const uint32_t Version = 1;

    class IStream
    {
    public:
        virtual void seek(uint64_t offset) = 0;
        virtual uint64_t size() const = 0;
        virtual uint64_t pos() const = 0;
        virtual uint64_t write(const void* data, uint64_t size) = 0;
        virtual uint64_t read(void* data, uint64_t size) = 0;
        virtual void flush() = 0;
    };

    class ITrace
    {
    public:
    };

    class ICapture
    {
    public:
        virtual void invalidateState() = 0;
        virtual void execute() = 0;
        virtual void interrupt(uint32_t type) = 0;
        virtual void signal(uint32_t type) = 0;
        virtual void read8(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void read16(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void read32(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void write8(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void write16(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void write32(uint32_t addr, uint32_t value, uint32_t type) = 0;
    };

    class IReplay
    {
    public:
        virtual void syncState(const void* state, size_t size) = 0;
        virtual void read8(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void read16(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void read32(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void write8(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void write16(uint32_t addr, uint32_t value, uint32_t type) = 0;
        virtual void write32(uint32_t addr, uint32_t value, uint32_t type) = 0;
    };

    class IDevice
    {
    public:
        virtual const char* getName() = 0;
        virtual uint32_t getVersion() = 0;
        virtual size_t getStateSize() = 0;
        virtual void getState(void* state, size_t size) = 0;
    };

    class ICaptureDevice : public IDevice
    {
    public:
        virtual void startCapture(ICapture& capture) = 0;
        virtual void stopCapture(ICapture& capture) = 0;
    };

    class IReplayDevice : public IDevice
    {
    public:
        virtual void loadState(const void* state, size_t size) = 0;
        virtual bool canSkip(uint32_t addr, uint32_t value, uint32_t type) = 0;
    };

    class IContext
    {
    public:
        virtual ITrace& createTrace() = 0;
        virtual void destroyTrace(ITrace& trace) = 0;
        virtual void loadTrace(ITrace& trace, IStream& stream) = 0;
        virtual void saveTrace(const ITrace& trace, IStream& stream) = 0;
        virtual ICapture& startCapture(ICaptureDevice& device, ITrace& trace) = 0;
        virtual void stopCapture(ICapture& capture) = 0;
    };

    IContext& createContext();
    void destroyContext(IContext& context);
}