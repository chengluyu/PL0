workspace "PL0"
    configurations { "Debug", "Release" }

project "pl0"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/%{cfg.buildcfg}"

    files { "./src/*.h", "./src/*.cpp" }
    cppdialect "C++17"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
