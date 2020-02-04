function exampleProjectDefaults()


end

function exampleProject(...)
  for _, name in ipairs({...}) do
    project("example-" .. name)
    uuid(os.uuid("example-" .. name))
    kind "ConsoleApp"

    files {
      path.join(EXAMPLES_DIR, name, "**.cpp"),
      path.join(EXAMPLES_DIR, name, "**.h")
    }

    removefiles {
      path.join(EXAMPLES_DIR, name, "**.bin.h")
    }

    debugdir(RUNTIME_DIR)

    includedirs {
      EXTERNAL_DIR,
      path.join(BDR_DIR, "src"),
    }

    flags {
      "FatalWarnings"
    }

    defines {
      "_SECURE_SCL=0"
    }

    links {
      "bdr_lib"
    }

    configuration {"vs*", "x64"}
    linkoptions {
      "/ignore:4199" -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
    }

    configuration{}
  end
end
