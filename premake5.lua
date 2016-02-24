solution "Nikyuria"
   language "C++"
   location "project"
   targetdir "C:/Nikyuria/dll"

   configuration "vs*" -- speed shit
   --gr- = no typeinfo
   buildoptions({"/Qpar", "/Qfast_transcendentals", "/GL", "/Ox", "/Gm", "/MP", "/GR-", "/MD", "/Gy", "/Gw"})
   linkoptions { "/OPT:REF", "/OPT:ICF", "/LTCG"}

   flags { "Optimize", "NoMinimalRebuild", "NoFramePointer", "Unicode", "EnableSSE2", "FloatFast", "NoBufferSecurityCheck"}

   vpaths {
      ["Header Files/*"] = "src/**.h",
      ["Source Files/*"] = "src/**.cpp",
   }

   kind "SharedLib"

   configurations { "Debug", "Release" }

   files { "src/**.h", "src/**.cpp" }
   includedirs { "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include" }
   libdirs {"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86", "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64"}
   
   -- A project defines one build target
   project "Nikyuria_generic"
      targetname "generic"

      configuration "Release"
         defines { "NDEBUG", "_GENERIC" }
         
      configuration "Debug"
         defines { "DEBUG", "_GENERIC" }
         flags { "Symbols", "Unicode", "EnableSSE2" }

   -- A project defines one build target
   project "Nikyuria_css"

      targetname "counter-strikesource"

      configuration "Release"
         defines { "NDEBUG", "_CSS" }

      configuration "Debug"
         defines { "DEBUG", "_CSS" }
         flags { "Symbols", "Unicode", "EnableSSE2" }

   -- A project defines one build target
   project "Nikyuria_csgo"
      targetname "counter-strike_globaloffensive"

      configuration "Release"
         defines { "NDEBUG", "_CSGO" }

      configuration "Debug"
         defines { "DEBUG", "_CSGO" }
         flags { "Symbols", "Unicode", "EnableSSE2" }

   -- A project defines one build target
   project "Nikyuria_tf2"
      targetname "teamfortress2"

      configuration "Release"
         defines { "NDEBUG", "_TF2" }

      configuration "Debug"
         defines { "DEBUG", "_TF2" }
         flags { "Symbols", "Unicode", "EnableSSE2" }

   -- A project defines one build target
   project "Nikyuria_gmod"
      targetname "garry'smod"

      configuration "Release"
         defines { "NDEBUG", "_GMOD" }

      configuration "Debug"
         defines { "DEBUG", "_GMOD" }
         flags { "Symbols", "Unicode", "EnableSSE2"}
   