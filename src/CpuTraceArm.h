#pragma once

#include "CpuTrace.h"

namespace CpuTrace
{
    namespace ARM
    {
        const uint32_t Version = 1;

        namespace Interrupt
        {
            enum
            {
                IRQ,
                FIQ,
                COUNT
            };
        }

        namespace Signal
        {
            enum
            {
                Unhalted,
                COUNT
            };
        }

        namespace MemoryAccess
        {
            enum
            {
                Unknown,
                Code,
                Data,
                GPU,
                DMA,
                Debug,
                COUNT
            };
        }

        struct State
        {
            struct
            {
                uint32_t    r[16];
                uint32_t    cpsr;
            }               usr;
            struct
            {
                uint32_t    r8;
                uint32_t    r9;
                uint32_t    r10;
                uint32_t    r11;
                uint32_t    r12;
                uint32_t    r13;
                uint32_t    r14;
                uint32_t    spsr;
            }               fiq;
            struct
            {
                uint32_t    r13;
                uint32_t    r14;
                uint32_t    spsr;
            }               svc;
            struct
            {
                uint32_t    r13;
                uint32_t    r14;
                uint32_t    spsr;
            }               abt;
            struct
            {
                uint32_t    r13;
                uint32_t    r14;
                uint32_t    spsr;
            }               irq;
            struct
            {
                uint32_t    r13;
                uint32_t    r14;
                uint32_t    spsr;
            }               und;
        };

        struct ICaptureHandler
        {
        public:
            virtual void start(ICapture& capture) = 0;
            virtual void stop(ICapture& capture) = 0;
            virtual void getState(State& state) = 0;
        };

        ICaptureDevice& createCaptureDevice(const char* name, ICaptureHandler& handler);
        void destroyCaptureDevice(ICaptureDevice& device);
    }
}