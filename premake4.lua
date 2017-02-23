solution 'pontella'
    configurations {'Release', 'Debug'}
    location 'build'

    newaction {
        trigger = "install",
        description = "Install the library",
        execute = function ()
            os.copyfile('source/pontella.hpp /usr/local/include/pontella.hpp')

            print(string.char(27) .. '[32mPontella library installed.' .. string.char(27) .. '[0m')
            os.exit()
        end
    }

    newaction {
        trigger = 'uninstall',
        description = 'Remove all the files installed during build processes',
        execute = function ()
            os.execute('rm -f /usr/local/include/pontella.hpp')
            print(string.char(27) .. '[32mPontella library uninstalled.' .. string.char(27) .. '[0m')
            os.exit()
        end
    }

    project 'pontellaTest'
        -- General settings
        kind 'ConsoleApp'
        language 'C++'
        location 'build'
        files {'source/**.hpp', 'test/**.hpp', 'test/**.cpp'}

        -- Declare the configurations
        configuration 'Release'
            targetdir 'build/Release'
            defines {'NDEBUG'}
            flags {'OptimizeSpeed'}
        configuration 'Debug'
            targetdir 'build/Debug'
            defines {'DEBUG'}
            flags {'Symbols'}

        -- Linux specific settings
        configuration 'linux'
            buildoptions {'-std=c++11'}
            linkoptions {'-std=c++11'}
            postbuildcommands {'cp ../source/pontella.hpp /usr/local/include/pontella.hpp')}

        -- Mac OS X specific settings
        configuration 'macosx'
            buildoptions {'-std=c++11', '-stdlib=libc++'}
            linkoptions {'-std=c++11', '-stdlib=libc++'}
            postbuildcommands {'cp ../source/pontella.hpp /usr/local/include/pontella.hpp')}
