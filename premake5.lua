function configure()
    configurations { "Debug", "Release" }
    platforms { "Win32", "x64" }

    filter "platforms:*32"
        system "Windows"
        architecture "x86"

    filter "platforms:*64"
        system "Windows"
        architecture "x86_64"

    filter {}
    
    flags { "ExtraWarnings", "FatalWarnings" }
    includedirs { "." }
end

function baseProject(kind_name, name)
    project(name)
        kind(kind_name)
        language "C++"
        configure()
end

function staticLibrary(name)
    baseProject("StaticLib", name)
end

function dynamicLibrary(name, file_filters)
    baseProject("DynamicLib", name)
end

function application(name, file_filters)
    baseProject("ConsoleApp", name)
end

workspace "CpuTrace"
    location "build"
    startproject "CpuTrace"
    configure()

    filter "configurations:Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        flags { "Symbols", "Optimize" }

    filter {}

staticLibrary "CpuTrace"
    files
    {
        "src/**.h", "src/**.cpp"
    }
