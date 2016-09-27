#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

/// pontella is a command  line parser.
namespace pontella {

    /// Optionnal is an abstract class to be inherited by options and flags.
    class Optionnal {
        public:
            Optionnal(std::string name) : Optionnal(name, std::move(name)) {}
            Optionnal(std::string name, std::string alias) :
                _name(name),
                _alias(alias)
            {
                if (_name.empty()) {
                    throw std::logic_error("The optionnal name cannot be empty");
                }
                if (_name[0] == '-') {
                    throw std::logic_error("The optionnal name '" + _name + "' cannot start with the character '-'");
                }
                for (auto&& character : _name) {
                    if (isspace(character)) {
                        throw std::logic_error("The optionnal name '" + _name + "' cannot contain white-space characters");
                    }
                    if (character == '=') {
                        throw std::logic_error("The optionnal name '" + _name + "' cannot contain the character '='");
                    }
                }
                if (_alias.empty()) {
                    throw std::logic_error("The optionnal alias cannot be empty");
                }
                if (_alias[0] == '-') {
                    throw std::logic_error("The optionnal alias '" + _alias + "' cannot start with the character '-'");
                }
                for (auto&& character : _alias) {
                    if (isspace(character)) {
                        throw std::logic_error("The optionnal alias '" + _alias + "' cannot contain white-space characters");
                    }
                    if (character == '=') {
                        throw std::logic_error("The optionnal alias '" + _alias + "' cannot contain the character '='");
                    }
                }
            }
            Optionnal(const Optionnal&) = default;
            Optionnal(Optionnal&&) = default;
            Optionnal& operator=(const Optionnal&) = default;
            Optionnal& operator=(Optionnal&&) = default;
            virtual ~Optionnal() {}

            /// name returns the option name.
            const std::string& name() const {
                return _name;
            }

            /// alias returns the option alias (shorter than the name).
            const std::string& alias() const {
                return _alias;
            }

        protected:
            std::string _name;
            std::string _alias;
    };

    /// Option represents a named command line option expecting an argument.
    class Option : public Optionnal {
        public:
            Option(std::string name) : Optionnal(std::move(name)) {}
            Option(std::string name, std::string alias) : Optionnal(std::move(name), std::move(alias)) {}
            Option(const Option&) = default;
            Option(Option&&) = default;
            Option& operator=(const Option&) = default;
            Option& operator=(Option&&) = default;
            virtual ~Option() {}
    };


    /// Flag represents a named command line option without argument.
    class Flag : public Optionnal {
        public:
            Flag(std::string name) : Optionnal(std::move(name)) {}
            Flag(std::string name, std::string alias) : Optionnal(std::move(name), std::move(alias)) {}
            Flag(const Flag&) = default;
            Flag(Flag&&) = default;
            Flag& operator=(const Flag&) = default;
            Flag& operator=(Flag&&) = default;
            virtual ~Flag() {}
    };

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
    Command parse(int argc, char* argv[], int64_t numberOfArguments, std::initializer_list<Option> options, std::initializer_list<Flag> flags) {
        auto optionnalByName = std::unordered_map<std::string, std::reference_wrapper<const Optionnal>>();
        auto nameByAlias = std::unordered_map<std::string, std::string>();
        auto optionsNames = std::unordered_set<std::string>();
        for (auto&& option : options) {
            if (optionnalByName.find(option.name()) != optionnalByName.end()) {
                throw std::logic_error("Duplicated optionnal name '" + option.name() + "'");
            }
            optionnalByName.insert(std::make_pair(option.name(), std::ref(option)));
            if (nameByAlias.find(option.alias()) != nameByAlias.end()) {
                throw std::logic_error("Duplicated optionnal alias '" + option.alias() + "'");
            }
            nameByAlias.insert(std::make_pair(option.alias(), option.name()));
            optionsNames.insert(option.name());
        }
        for (auto&& flag : flags) {
            if (optionnalByName.find(flag.name()) != optionnalByName.end()) {
                throw std::logic_error("Duplicated optionnal name '" + flag.name() + "'");
            }
            optionnalByName.insert(std::make_pair(flag.name(), std::ref(flag)));
            if (nameByAlias.find(flag.alias()) != nameByAlias.end()) {
                throw std::logic_error("Duplicated optionnal alias '" + flag.alias() + "'");
            }
            nameByAlias.insert(std::make_pair(flag.alias(), flag.name()));
        }

        auto command = Command{{}, {}, {}};

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

                auto nameAndOptionnal = optionnalByName.end();
                auto parameter = std::string();
                auto hasEqual = false;
                {
                    auto nameOrAlias = std::string();
                    for (auto characterIterator = nameOrAliasAndParameter.begin(); characterIterator != nameOrAliasAndParameter.end(); ++characterIterator) {
                        if (*characterIterator == '=') {
                            hasEqual = true;
                            nameOrAlias = std::string(nameOrAliasAndParameter.begin(), characterIterator);
                            parameter = std::string(std::next(characterIterator), nameOrAliasAndParameter.end());
                        }
                    }
                    if (!hasEqual) {
                        nameOrAlias = nameOrAliasAndParameter;
                        if (index < argc - 1) {
                            parameter = std::string(argv[index + 1]);
                        }
                    }
                    const auto nameAndOptionnalCandidate = optionnalByName.find(nameOrAlias);
                    if (nameAndOptionnalCandidate == optionnalByName.end()) {
                        const auto aliasAndNameCandidate = nameByAlias.find(nameOrAlias);
                        if (aliasAndNameCandidate == nameByAlias.end()) {
                            throw std::runtime_error("Unknown option name or alias '" + nameOrAlias + "'");
                        }
                        nameAndOptionnal = optionnalByName.find(aliasAndNameCandidate->second);
                    } else {
                        nameAndOptionnal = nameAndOptionnalCandidate;
                    }
                }

                if (optionsNames.find(nameAndOptionnal->second.get().name()) == optionsNames.end()) {
                    if (hasEqual) {
                        throw std::runtime_error("The flag '" + nameAndOptionnal->second.get().name() + "' does not take a parameter");
                    }
                    command.flags.insert(nameAndOptionnal->second.get().name());
                } else {
                    if (!hasEqual) {
                        if (index == argc - 1) {
                            throw std::runtime_error("The option '" + nameAndOptionnal->second.get().name() + "' requires a parameter");
                        }
                        ++index;
                    }
                    command.options.insert(std::make_pair(nameAndOptionnal->second.get().name(), parameter));

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
    bool test(int argc, char* argv[], Flag flag) {
        auto found = false;
        auto patterns = std::unordered_set<std::string>({"--" + flag.name(), "-" + flag.name(), "--" + flag.alias(), "-" + flag.alias()});
        for (auto index = 1; index < argc; ++index) {
            if (patterns.find(std::string(argv[index])) != patterns.end()) {
                found = true;
                break;
            }
        }
        return found;
    }
}
