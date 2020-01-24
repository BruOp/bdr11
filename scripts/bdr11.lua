BDR_DIR = (path.getabsolute("..") .. "/")
EXAMPLES_DIR = (BDR_DIR .. "examples/")

EXTERNAL_DIR = (BDR_DIR .. "external/")

-- $(SolutionDir).build\$(Platform)\$(Configuration)\
local BUILD_DIR = path.join(BDR_DIR, ".build")

--
-- Solution
--
workspace "bdr11"
  language "C++"
  configurations {"Debug", "Release"}
  platforms {"x64"}
  startproject "bdr11_lib"


  premake.vstudio.toolset = "v142"

  defines { "_WIN64" }
  targetdir (path.join(BUILD_DIR, "x64" .. _ACTION, "bin"))
  objdir (path.join(BUILD_DIR, "x64" .. _ACTION, "obj"))
  libdirs {
    path.join(_libDir, "lib/win64_" .. _ACTION),
  }

  defines {
    "WIN32",
    "_WIN32",
    "_HAS_EXCEPTIONS=0",
    "_HAS_ITERATOR_DEBUGGING=0",
    "_ITERATOR_DEBUG_LEVEL=0",
    "_SCL_SECURE=0",
    "_SECURE_SCL=0",
    "_SCL_SECURE_NO_WARNINGS",
    "_CRT_SECURE_NO_WARNINGS",
    "_CRT_SECURE_NO_DEPRECATE",
  }
  buildoptions {
    "/ignore:4267",
    "/ignore:28612",
  }
  linkoptions {
    "/ignore:4267",
    "/ignore:28612",
    "/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
  }


---
--- Projects
---

BDR_SRC_DIR = path.join(BDR_DIR, "src")

project("bdr_lib")
uuid(os.uuid("bdr_lib"))
kind "StaticLib"

files {
  path.join(BDR_SRC_DIR, "**.cpp"),
}

includedirs {
  BDR_SRC_DIR,
}

pchheader "pch.h"
pchsource "pch.cpp"

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

strip()
