
project "LabWeb"
    kind "StaticLib"
    language "C++"    
    platforms { "x32", "x64" }
    
    includedirs { "src", "src/websocketpp/src" } 
    files { "src/LabWeb/*.h", "src/LabWeb/*.hpp", "src/LabWeb/*.cpp",
            "src/websocketpp/src/**.h", "src/websocketpp/src/**.hpp", "src/websocketpp/src/**.cpp"
     }        
    excludes { }

    configuration "Debug"
        targetdir "build/Debug"
        defines {  "DEBUG", "__MACOSX_CORE__", "OSX" }
        flags { "Symbols" }

    configuration "Release"
        targetdir "build/Release"
        defines { "NDEBUG", "__MACOSX_CORE__", "OSX" }
        flags { "Optimize" } 

    configuration "macosx"
        buildoptions { "-std=c++11", "-stdlib=libc++" }
