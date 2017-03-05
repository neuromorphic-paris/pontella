![pontella](pontellaBanner.png "The Pontella banner")

# Pontella

Pontella is a lightweight command line parser.

# Installation

## Dependencies

Pontella relies on [Premake 4.x](https://github.com/premake/premake-4.x) (x ≥ 3), which is a utility to ease the install process. Follow these steps:
  - __Debian / Ubuntu__: Open a terminal and execute the command `sudo apt-get install premake4`.
  - __Fedora__: Open a terminal and execute the command `sudo dnf install premake`. Then, run<br />
  `echo '/usr/local/lib' | sudo tee /etc/ld.so.conf.d/neuromorphic-paris.conf > /dev/null`.
  - __OS X__: Open a terminal and execute the command `brew install premake`. If the command is not found, you need to install Homebrew first with the command<br />
  `ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`.

## Install

To install the source, go to the *pontella* directory and run:
  - __Linux__: `premake4 install`
  - __OS X__: `premake4 install`
The library files are installed in */usr/local/include*.

## Uninstall

To uninstall the library, run `premake4 uninstall` from the *pontella* directory.

## Test

To test the library, run the following commands:
  - Go to the *pontella* directory and run `premake4 gmake && cd build && make`.
  - Run the executable *Release/pontellaTest*.

## Documentation

Assuming the command line `./fileOpener /path/to/file --help --verbose 1`, Pontella can be called as follows:
```cpp
#include <pontella.hpp>

int main(int argc, char* argv[]) {
    const auto command = pontella::parse(argc, argv, 1, {
        {"verbose", "v"},
    },
        {"help", "h"},
    });

    return 0;
```

The function `pontella::parse` has the signature:
```cpp
namespace pontella {

    /// parse turns argc and argv into parsed arguments and options.
    /// If numberOfArguments is negative, the number of arguments is unlimited.
    Command parse(
        int argc,
        char* argv[],
        int64_t numberOfArguments,
        std::initializer_list<std::initializer_list<std::string>> options,
        std::initializer_list<std::initializer_list<std::string>> flags
    );
}
```

- `argc` is the program's `argc` given to the main function.
- `argv` is the program's `argv` given to the main function.
- `numberOfArguments` is the expected number of positional arguments. If the incorrect number of arguments is passed to the program, `pontella::parse` will throw an exception. To allow any number of arguments, set `numberOfArguments` to `-1`.
- `options` lists the available options (named arguments with a parameter) and their aliases (each option can have any number of aliases).
- `flags` lists the available flags (named arguments without parameter) and their aliases (each flag can have any number of aliases).

The returned `pontella::Command` is defined by:
```cpp
namespace pontella {

    /// Command contains parsed arguments, options and flags.
    struct Command {

        /// arguments contains the positional arguments given to the program.
        std::vector<std::string> arguments;

        /// options contains the named options and the associated parameter given to the program.
        std::unordered_map<std::string, std::string> options;

        /// flags contains the named flags given to the program.
        std::unordered_set<std::string> flags;
    };
}
```

As an example, to check wether help is required, one can write:
```cpp
#include <pontella.hpp>

int main(int argc, char* argv[]) {
    const auto command = pontella::parse(argc, argv, 1, {
        {"verbose", "v"},
    },
        {"help", "h"},
    });

    if (command.flags.find("help") != command.flags.end()) {
        // display help here
    }

    return 0;
}
```

`pontella::parse` may throw two kind of exceptions: `std::logic_error` are thrown when the options and flags chosen by the programer are not valid, whereas `std::runtime_error` are thrown when the arguments given by the user do not match the options and flags.

Pontella authorizes the following syntaxes for options:
  - `./fileOpener /path/to/file --help --verbose 1`
  - `./fileOpener /path/to/file --help --verbose=1`
  - `./fileOpener /path/to/file --help -verbose 1`
  - `./fileOpener /path/to/file --help -verbose=1`
  - `./fileOpener /path/to/file --help --v 1`
  - `./fileOpener /path/to/file --help --v=1`
  - `./fileOpener /path/to/file --help -v 1`
  - `./fileOpener /path/to/file --help -v=1`

Pontella authorizes the following syntaxes for flags:
  - `./fileOpener /path/to/file --help --verbose 1`
  - `./fileOpener /path/to/file -help --verbose 1`
  - `./fileOpener /path/to/file --h --verbose 1`
  - `./fileOpener /path/to/file -h --verbose 1`

All the positions for options and flags are possible:
  - `./fileOpener /path/to/file --help --verbose 1`
  - `./fileOpener /path/to/file --verbose 1 --help`
  - `./fileOpener --help /path/to/file --verbose 1`
  - `./fileOpener --help --verbose 1 /path/to/file`
  - `./fileOpener --verbose 1 /path/to/file --help`
  - `./fileOpener --verbose 1 --help /path/to/file`

Here is an extensive example with error handling:
```cpp
#include <pontella.hpp>

int main(int argc, char* argv[]) {
    auto showHelp = false;
    try {
        const auto command = pontella::parse(argc, argv, 1, {
            {"verbose", "v"},
        },
            {"help", "h"},
        });
        if (command.flags.find("help") != command.flags.end()) {
            showHelp = true;
        } else {
            auto verbose = static_cast<uint64_t>(0);
            {
                const auto verboseCandidate = command.options.find("verbose");
                if (verboseCandidate != command.options.end()) {
                    try {
                        const auto unvalidatedVerbose = std::stoll(verboseCandidate->second);
                        if (unvalidatedVerbose < 0) {
                            throw std::runtime_error("[verbose level] must be a positive integer (got '" + verboseCandidate->second + "')");
                        }
                        verbose = static_cast<uint64_t>(unvalidatedVerbose);
                    } catch (const std::invalid_argument&) {
                        throw std::runtime_error("[verbose level] must be a positive integer (got '" + verboseCandidate->second + "')");
                    }
                }
            }

            // use command.arguments and command.verbose to do cool things
        }
    } catch (const std::runtime_error& exception) {
        showHelp = true;
        if (!pontella::test(argc, argv, {"help", "h"})) { // only show the error if help was not required
            std::cout << "\e[31m" << exception.what() << "\e[0m" << std::endl;
        }
    }

    if (showHelp) {
        std::cout <<
            "Syntax: ./fileOpener [options] /path/to/file\n"
            "Available options:\n"
            "    -v [verbose level], --verbose [verbose level]   define the level of verbose (defaults to 0)\n"
            "    -h, --help                                      show this help message\n"
        << std::endl;
    }

    return 0;
```

This example makes use of the function `pontella::test`, with the signature:
```cpp
namespace pontella {

    /// test determines wether the given flag was used.
    /// It is meant to be used to hide the error message when a specific flag (such as help) is given.
    /// This method does not work with options.
    bool test(int argc, char* argv[], std::initializer_list<std::string> flag);
}
```

# License

See the [LICENSE](LICENSE.txt) file for license rights and limitations (GNU GPLv3).
