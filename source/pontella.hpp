#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

/// pontella is a command  line parser.
namespace pontella {

    /// Command contains parsed arguments, options and flags.
    struct Command {

        /// arguments contains the positionnal arguments given to the program.
        std::vector<std::string> arguments;

        /// options contains the named options and the associated parameter given to the program.
        std::unordered_map<std::string, std::string> options;

        /// flags contains the named flags given to the program.
        std::unordered_set<std::string> flags;
    };

    /// parse turns argc and argv into parsed arguments and options.
    /// If numberOfArguments is negative, the number of arguments is unlimited.
    Command parse(
        int argc,
        char* argv[],
        int64_t numberOfArguments,
        std::initializer_list<std::initializer_list<std::string>> options,
        std::initializer_list<std::initializer_list<std::string>> flags
    ) {
        auto isOptionByName = std::unordered_map<std::string, bool>();
        auto nameByAlias = std::unordered_map<std::string, std::string>();
        for (const auto& option : options) {
            if (option.size() == 0) {
                throw std::logic_error("An option cannot be empty");
            }
            for (const auto& nameOrAlias : option) {
                if (nameOrAlias.empty()) {
                    throw std::logic_error("An option name or alias cannot be empty");
                }
                if (nameOrAlias[0] == '-') {
                    throw std::logic_error("The option name or alias '" + nameOrAlias + "' cannot start with the charcater '-'");
                }
                for (auto character : nameOrAlias) {
                    if (isspace(character)) {
                        throw std::logic_error("The option name or alias '" + nameOrAlias + "' cannot contain white-space characters");
                    }
                    if (character == '=') {
                        throw std::logic_error("The option name or alias '" + nameOrAlias + "' cannot contain the character '='");
                    }
                }
            }
            {
                const auto inserter = isOptionByName.insert(std::make_pair(*option.begin(), true));
                if (!inserter.second) {
                    throw std::logic_error("Duplicated option name '" + *option.begin() + "'");
                }
            }
            for (auto aliasIterator = std::next(option.begin()); aliasIterator != option.end(); ++aliasIterator) {
                if (isOptionByName.find(*aliasIterator) != isOptionByName.end()) {
                    throw std::logic_error("Duplicated option name or alias '" + *aliasIterator + "'");
                }
                const auto inserter = nameByAlias.insert(std::make_pair(*aliasIterator, *option.begin()));
                if (!inserter.second) {
                    throw std::logic_error("Duplciated option alias '" + *aliasIterator + "'");
                }
            }
        }
        for (const auto& flag : flags) {
            if (flag.size() == 0) {
                throw std::logic_error("An flag cannot be empty");
            }
            for (const auto& nameOrAlias : flag) {
                if (nameOrAlias.empty()) {
                    throw std::logic_error("An flag name or alias cannot be empty");
                }
                if (nameOrAlias[0] == '-') {
                    throw std::logic_error("The flag name or alias '" + nameOrAlias + "' cannot start with the charcater '-'");
                }
                for (auto character : nameOrAlias) {
                    if (isspace(character)) {
                        throw std::logic_error("The flag name or alias '" + nameOrAlias + "' cannot contain white-space characters");
                    }
                    if (character == '=') {
                        throw std::logic_error("The flag name or alias '" + nameOrAlias + "' cannot contain the character '='");
                    }
                }
            }
            {
                const auto inserter = isOptionByName.insert(std::make_pair(*flag.begin(), false));
                if (!inserter.second) {
                    throw std::logic_error("Duplicated flag name '" + *flag.begin() + "'");
                }
            }
            for (auto aliasIterator = std::next(flag.begin()); aliasIterator != flag.end(); ++aliasIterator) {
                if (isOptionByName.find(*aliasIterator) != isOptionByName.end()) {
                    throw std::logic_error("Duplicated flag name or alias '" + *aliasIterator + "'");
                }
                const auto inserter = nameByAlias.insert(std::make_pair(*aliasIterator, *flag.begin()));
                if (!inserter.second) {
                    throw std::logic_error("Duplciated flag alias '" + *aliasIterator + "'");
                }
            }
        }

        auto command = Command{};

        for (auto index = 1; index < argc; ++index) {
            const auto element = std::string(argv[index]);

            if (element[0] == '-') {
                auto nameOrAliasAndParameter = std::string();
                if (element.size() == 1) {
                    throw std::runtime_error("Unexpected character '-' without an associated name or alias");
                } else {
                    if (element[1] == '-') {
                        if (element.size() == 2) {
                            throw std::runtime_error("Unexpected characters '--' without an associated name or alias");
                        } else {
                            nameOrAliasAndParameter = element.substr(2);
                        }
                    } else {
                        nameOrAliasAndParameter = element.substr(1);
                    }
                }

                auto nameAndIsOption = isOptionByName.end();
                auto parameter = std::string();
                auto hasEqual = false;
                {
                    auto nameOrAlias = std::string();
                    for (auto characterIterator = nameOrAliasAndParameter.begin(); characterIterator != nameOrAliasAndParameter.end(); ++characterIterator) {
                        if (*characterIterator == '=') {
                            hasEqual = true;
                            nameOrAlias = std::string(nameOrAliasAndParameter.begin(), characterIterator);
                            parameter = std::string(std::next(characterIterator), nameOrAliasAndParameter.end());
                            break;
                        }
                    }
                    if (!hasEqual) {
                        nameOrAlias = nameOrAliasAndParameter;
                        if (index < argc - 1) {
                            parameter = std::string(argv[index + 1]);
                        }
                    }
                    const auto nameAndIsOptionCandidate = isOptionByName.find(nameOrAlias);
                    if (nameAndIsOptionCandidate == isOptionByName.end()) {
                        const auto aliasAndNameCandidate = nameByAlias.find(nameOrAlias);
                        if (aliasAndNameCandidate == nameByAlias.end()) {
                            throw std::runtime_error("Unknown option name or alias '" + nameOrAlias + "'");
                        }
                        nameAndIsOption = isOptionByName.find(aliasAndNameCandidate->second);
                    } else {
                        nameAndIsOption = nameAndIsOptionCandidate;
                    }
                }

                if (nameAndIsOption->second) {
                    if (!hasEqual) {
                        if (index == argc - 1) {
                            throw std::runtime_error("The option '" + nameAndIsOption->first + "' requires a parameter");
                        }
                        ++index;
                    }
                    command.options.insert(std::make_pair(nameAndIsOption->first, parameter));
                } else {
                    if (hasEqual) {
                        throw std::runtime_error("The flag '" + nameAndIsOption->first + "' does not take a parameter");
                    }
                    command.flags.insert(nameAndIsOption->first);
                }
            } else {
                if (numberOfArguments >= 0 && command.arguments.size() >= numberOfArguments) {
                    throw std::runtime_error("Too many arguments (" + std::to_string(numberOfArguments) + " expected)");
                }
                command.arguments.push_back(element);
            }
        }
        if (numberOfArguments >= 0 && command.arguments.size() < numberOfArguments) {
            throw std::runtime_error("Not enough arguments (" + std::to_string(numberOfArguments) + " expected)");
        }

        return command;
    }

    /// test determines wether the given flag was used.
    /// It is meant to be used to hide the error message when a specific flag (such as help) is given.
    /// This method does not work with options.
    bool test(int argc, char* argv[], std::initializer_list<std::string> flag) {
        auto patterns = std::unordered_set<std::string>();
        if (flag.size() == 0) {
            throw std::logic_error("An flag cannot be empty");
        }
        for (const auto& nameOrAlias : flag) {
            if (nameOrAlias.empty()) {
                throw std::logic_error("An flag name or alias cannot be empty");
            }
            if (nameOrAlias[0] == '-') {
                throw std::logic_error("The flag name or alias '" + nameOrAlias + "' cannot start with the charcater '-'");
            }
            for (const auto& character : nameOrAlias) {
                if (isspace(character)) {
                    throw std::logic_error("The flag name or alias '" + nameOrAlias + "' cannot contain white-space characters");
                }
                if (character == '=') {
                    throw std::logic_error("The flag name or alias '" + nameOrAlias + "' cannot contain the character '='");
                }
            }
            for (const auto& prefix : std::initializer_list<std::string>({"-", "--"})) {
                const auto inserter = patterns.insert(prefix + nameOrAlias);
                if (!inserter.second) {
                    throw std::logic_error("Duplicated flag name or alias '" + nameOrAlias + "'");
                }
            }
        }
        auto found = false;
        for (auto index = 1; index < argc; ++index) {
            if (patterns.find(std::string(argv[index])) != patterns.end()) {
                found = true;
                break;
            }
        }
        return found;
    }
}
