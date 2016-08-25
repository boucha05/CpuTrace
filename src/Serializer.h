#pragma once

#include "CpuTrace.h"

namespace CpuTrace
{
    namespace Impl
    {
        class Serializer
        {
        public:
            enum class Mode : uint32_t
            {
                None,
                Read,
                Write,
            };

            Serializer(IStream& stream, Mode mode)
                : mStream(stream)
                , mIsReading(mode == Mode::Read)
                , mIsWriting(mode == Mode::Write)
            {
            }

            IStream& getStream()
            {
                return mStream;
            }

            bool isReading() const
            {
                return mIsReading;
            }

            bool isWriting() const
            {
                return mIsWriting;
            }

            template <typename T>
            Serializer& serialize(const char* name, T& value)
            {
                serialize(*this, value);
                return *this;
            }

            template <typename T>
            Serializer& serializeRaw(T& value)
            {
                if (isReading())
                {
                    mStream.read(&value, sizeof(T));
                }
                if (isWriting())
                {
                    mStream.write(&value, sizeof(T));
                }
                return *this;
            }

        private:
            IStream&    mStream;
            bool        mIsReading;
            bool        mIsWriting;
        };

        void serialize(Serializer& serializer, uint32_t& value)
        {
            serializer.serializeRaw(value);
        }
    }
}