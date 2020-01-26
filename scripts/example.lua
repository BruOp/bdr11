function exampleProjectDefaults()
  debugdir(RUNTIME_DIR)

  includedirs {
      path.join(BDR_DIR, "src"),
  }

  flags {
      "FatalWarnings"
  }

  defines {
      "_HAS_ITERATOR_DEBUGGING=0",
      "_SECURE_SCL=0"
  }

  links {
      "bdr11_lib"
  }

  configuration {"vs*", "x64"}
  linkoptions {
      "/ignore:4199" -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
  }

--   configuration {"vs20*", "x32 or x64"}
--   links {
--       "gdi32",
--       "psapi"
--   }
  configuration{}

end

function exampleProject(...)
  for _, name in ipairs({...}) do
    project("example-" .. name)
    uuid(os.uuid("example-" .. name))
    kind "WindowedApp"

    files {
        path.join(EXAMPLES_DIR, name, "**.cpp"),
        path.join(EXAMPLES_DIR, name, "**.h")
    }

    removefiles {
        path.join(EXAMPLES_DIR, name, "**.bin.h")
    }

    exampleProjectDefaults()
  end
end
