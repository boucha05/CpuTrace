# CpuTrace
Tool for capturing and replaying CPU instructions.

# Description

This is a tool that can be integrated into emulators to capture and replay sequences of CPU instructions.

When capturing a trace, the application generating the capture has access to various functions to report the following kind of activities:
* Memory access operations and types.
* When instructions are executed.
* When high level emulation is used instead of real CPU instructions.
* Interrupt and external signals that might be of some use.

Once a trace is captured, it can be used for replaying the captured sequence of operations. Captured CPU activity can be used to replay a sequence of CPU instructions without using the CPU in a real system integration. Here are some of the cases where traces might be useful:
* Writing tests to make sure that working CPU sequences are not broken by changes to source code.
* Measuring performance of CPU emulation without any noise of other system components.
* Validate that various emulators have CPU that produce similar results.

# Why this tool?

This is a tool I'm working on while I'm developing [DSEmu](https://github.com/boucha05/DSEmu), my own Nintendo DS emulator. I use it to validate that my ARM CPU interpreter is working well with ROM files that are know to work with DeSmuME.

Currently I am in injecting this as a submodule into a fork of DeSmuME to capture sequences of ARM instruction interpreted by this emulator when executing a ROM. Captured traces are then replayed with DSEmu by also injecting this submodule in DSEmu and implementing a replay device.

I use captured traces to validate that I produce similar results when executing the same instructions with the same memory access operations than DeSmuME. Any difference indicates a bug or an incomplete CPU feature in my emulation code or in DeSmuME, which can be interesting to look at in any case, even if it's a bug that's on DeSmuME's side. And the best part, is that I can use this tool very early in my project at a time where I am not able to emulate any subsystem that is necessary to run a ROM successfully.

I also plan to use it as a validation tool to verify that I am not breaking anything in my CPU emulation that used to work before.

# How to build

Currently, the code under the src folder should be part of your own project. The code in this folder should be added in your project from it's original location as it depends internally on external code contributions. The code contains the core of the CpuTrace library and some specialization for ARM CPU.

There is a Visual Studio 2015 solution under the build folder that can be used to generate static library but it's not intended to be used this way. This is just to validate that the code builds correctly. I might make official libraries later when the code base is more mature.
