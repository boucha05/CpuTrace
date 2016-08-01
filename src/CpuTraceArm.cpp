#include "CpuTraceArm.h"
#include <algorithm>
#include <string>

namespace
{
    using namespace CpuTrace;
    using namespace CpuTrace::ARM;

    class CaptureDevice : public ICaptureDevice
    {
    public:
        CaptureDevice(const char* name, ICaptureHandler& handler)
            : mName(name)
            , mHandler(handler)
        {
        }

        virtual const char* getName() override
        {
            return mName.c_str();
        }

        virtual uint32_t getVersion() override
        {
            return CpuTrace::ARM::Version;
        }

        virtual size_t getStateSize() override
        {
            return sizeof(CpuTrace::ARM::State);
        }

        virtual void getState(void* state, size_t size)
        {
            CpuTrace::ARM::State localState;
            mHandler.getState(localState);
            memcpy(state, &localState, std::min(size, sizeof(localState)));
        }

        virtual void startCapture(ICapture& capture)
        {
            mHandler.start(capture);
        }

        virtual void stopCapture(ICapture& capture)
        {
            mHandler.stop(capture);
        }

    private:
        std::string         mName;
        ICaptureHandler&    mHandler;
    };
}

namespace CpuTrace
{
    namespace ARM
    {
        ICaptureDevice& createCaptureDevice(const char* name, ICaptureHandler& handler)
        {
            return *new CaptureDevice(name, handler);
        }

        void destroyCaptureDevice(ICaptureDevice& device)
        {
            delete &static_cast<CaptureDevice&>(device);
        }
    }
}
