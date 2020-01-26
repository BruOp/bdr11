BDR_DIR = (path.getabsolute("..") .. "/")
RUNTIME_DIR = (path.getabsolute("..") .. "/runtime/")
EXAMPLES_DIR = (BDR_DIR .. "examples/")

EXTERNAL_DIR = (BDR_DIR .. "external/include/")

-- $(SolutionDir).build\$(Platform)\$(Configuration)\
local BUILD_DIR = path.join(BDR_DIR, ".build")

include("./example.lua")

--
-- Solution
--
workspace "bdr11"
  language "C++"
  configurations {"Debug", "Release"}
  platforms {"x64"}
  startproject "bdr11_lib"
  cppdialect "C++17"
  premake.vstudio.toolset = "v142"

  filter { "configurations:Debug" }
    symbols "On"
  filter { "configurations:Release" }
    optimize "On"
    -- Reset the filter for other settings
  filter { }

  defines { "_WIN64" }
  targetdir (path.join(BUILD_DIR, "x64" .. _ACTION, "bin"))
  objdir (path.join(BUILD_DIR, "x64" .. _ACTION, "obj"))
  libdirs {
    path.join(_libDir, "lib/win64_" .. _ACTION),
  }

  floatingpoint "fast"

  defines {
    "WIN32",
    "_WIN32",
    "_HAS_ITERATOR_DEBUGGING=0",
    "_ITERATOR_DEBUG_LEVEL=0",
    "_SCL_SECURE=0",
    "_SECURE_SCL=0",
    "_SCL_SECURE_NO_WARNINGS",
    "_CRT_SECURE_NO_WARNINGS",
    "_CRT_SECURE_NO_DEPRECATE",
  }
  linkoptions {
    "/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
  }

  disablewarnings {
    "4267",
    "28612",
  }

---
--- Projects
---

BDR_SRC_DIR = path.join(BDR_DIR, "src")

project("bdr_lib")
  uuid(os.uuid("bdr_lib"))
  kind "StaticLib"

  nuget {
    "directxtk_desktop_2015:2019.12.17.1"
  }

  files {
    path.join(BDR_SRC_DIR, "Shaders/**.hlsl"),
    path.join(BDR_SRC_DIR, "**.cpp"),
    path.join(BDR_SRC_DIR, "**.h"),
  }

  filter { "files:**.hlsl" }
    flags {"ExcludeFromBuild"}
  filter {}

  includedirs {
    EXTERNAL_DIR,
  }

  pchheader "pch.h"
  pchsource (path.join(BDR_SRC_DIR, "pch.cpp"))

  links {
    "d3d11",
    "dxgi",
    "dxguid",
    "uuid",
    "kernel32",
    "user32",
    "gdi32",
    "winspool",
    "comdlg32",
    "advapi32",
    "shell32",
    "ole32",
    "oleaut32",
    "odbc32",
    "odbccp32",
    "D3DCompiler"
  }

  configuration {}

group "examples"
exampleProject(
    "01-basic"
)