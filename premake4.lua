solution 'pontella'
    configurations {'release', 'debug'}
    location 'build'
    project 'pontella'
        kind 'ConsoleApp'
        language 'C++'
        location 'build'
        files {'source/*.hpp', 'test/*.cpp'}
        configuration 'release'
            targetdir 'build/release'
            defines {'NDEBUG'}
            flags {'OptimizeSpeed'}
        configuration 'debug'
            targetdir 'build/debug'
            defines {'DEBUG'}
            flags {'Symbols'}
        configuration 'linux'
            buildoptions {'-std=c++11'}
            linkoptions {'-std=c++11'}
        configuration 'macosx'
            buildoptions {'-std=c++11'}
            linkoptions {'-std=c++11'}
        configuration 'windows'
            files {'.clang-format'}
