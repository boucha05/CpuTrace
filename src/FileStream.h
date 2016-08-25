#pragma once

#include "CpuTrace.h"
#include <cstdio>

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

namespace CpuTrace
{
    class FileStream : public IStream
    {
    public:
        FileStream(FILE* file)
            : mFile(file)
            , mClose(false)
        {
        }

        FileStream(const char* path, const char* mode)
            : mFile(fopen(path, mode))
            , mClose(true)
        {
        }

        ~FileStream()
        {
            if (mClose)
                fclose(mFile);
        }

        void close()
        {
            if (mFile)
            {
                mFile = nullptr;
                fclose(mFile);
            }
        }

        virtual void seek(uint64_t offset) override
        {
            _fseeki64(mFile, offset, SEEK_SET);
        }

        virtual uint64_t size() const override
        {
            uint64_t curPos = pos();
            _fseeki64(mFile, 0, SEEK_END);
            uint64_t size = pos();
            _fseeki64(mFile, curPos, SEEK_SET);
            return size;
        }

        virtual uint64_t pos() const override
        {
            return _ftelli64(mFile);
        }

        virtual uint64_t write(const void* data, uint64_t size) override
        {
            uint64_t total = 0;
            while (size > 0)
            {
                size_t chunkSize = size > static_cast<uint64_t>(SIZE_MAX) ? SIZE_MAX : static_cast<size_t>(size);
                total += fwrite(data, 1, chunkSize, mFile);
                size -= chunkSize;
            }
            return total;
        }

        virtual uint64_t read(void* data, uint64_t size) override
        {
            uint64_t total = 0;
            while (size > 0)
            {
                size_t chunkSize = size > static_cast<uint64_t>(SIZE_MAX) ? SIZE_MAX : static_cast<size_t>(size);
                size_t ioSize = fread(data, 1, chunkSize, mFile);
                total += ioSize;
                data = static_cast<uint8_t*>(data) + ioSize;
                if (ioSize < chunkSize)
                    break;
                size -= ioSize;
            }
            return total;
        }

        virtual void flush() override
        {
            fflush(mFile);
        }

    private:
        FILE*   mFile;
        bool    mClose;
    };
}