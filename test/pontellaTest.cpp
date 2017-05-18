#include "catch.hpp"

#include "../source/pontella.hpp"

TEST_CASE("Parse a valid command line", "[parse]") {
    for (const auto& firstOptionParts : std::vector<std::vector<const char*>>({
        {"--verbose=1"},
        {"-verbose=1"},
        {"--v=1"},
        {"-v=1"},
        {"--verbose", "1"},
        {"-verbose", "1"},
        {"--v", "1"},
        {"-v", "1"},
    })) {
        for (const auto& secondOption : std::vector<const char*>({
            "--help",
            "-help",
            "--h",
            "-h",
        })) {
            for (auto permutationIndex = static_cast<std::size_t>(0); permutationIndex < 6; ++permutationIndex) {
                auto arguments = std::vector<const char*>({"./program"});
                switch (permutationIndex) {
                    case 0: {
                        arguments.push_back("input.log");
                        for (const auto& firstOptionPart : firstOptionParts) {
                            arguments.push_back(firstOptionPart);
                        }
                        arguments.push_back(secondOption);
                        break;
                    }
                    case 1: {
                        arguments.push_back("input.log");
                        arguments.push_back(secondOption);
                        for (const auto& firstOptionPart : firstOptionParts) {
                            arguments.push_back(firstOptionPart);
                        }
                        break;
                    }
                    case 2: {
                        for (const auto& firstOptionPart : firstOptionParts) {
                            arguments.push_back(firstOptionPart);
                        }
                        arguments.push_back("input.log");
                        arguments.push_back(secondOption);
                        break;
                    }
                    case 3: {
                        for (const auto& firstOptionPart : firstOptionParts) {
                            arguments.push_back(firstOptionPart);
                        }
                        arguments.push_back(secondOption);
                        arguments.push_back("input.log");
                        break;
                    }
                    case 4: {
                        arguments.push_back(secondOption);
                        arguments.push_back("input.log");
                        for (const auto& firstOptionPart : firstOptionParts) {
                            arguments.push_back(firstOptionPart);
                        }
                        break;
                    }
                    case 5: {
                        arguments.push_back(secondOption);
                        for (const auto& firstOptionPart : firstOptionParts) {
                            arguments.push_back(firstOptionPart);
                        }
                        arguments.push_back("input.log");
                        break;
                    }
                    default: {
                        break;
                    }
                }
                auto command = pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 1, {
                    {"verbose", "v"},
                }, {
                    {"help", "h"},
                });
                REQUIRE(command.arguments.size() == 1);
                REQUIRE(command.arguments.front() == "input.log");
                REQUIRE(command.options.size() == 1);
                REQUIRE(command.options.find("verbose") != command.options.end());
                REQUIRE(command.options["verbose"] == "1");
                REQUIRE(command.flags.find("help") != command.flags.end());
            }
        }
    }
}

TEST_CASE("Fail on too many arguments", "[parse]") {
    auto arguments = std::vector<const char*>({"./program", "input.log"});
    REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {}, {}));
}

TEST_CASE("Fail on not enough arguments", "[parse]") {
    auto arguments = std::vector<const char*>({"./program", "input.log"});
    REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 2, {}, {}));
}

TEST_CASE("Fail on options with the same name", "[parse]") {
    auto arguments = std::vector<const char*>({"./program"});
    REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {}, {
        {"help", "h1"},
        {"help", "h2"},
    }));
}

TEST_CASE("Fail on options with the same alias", "[parse]") {
    auto arguments = std::vector<const char*>({"./program"});
    REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {
        {"hidden", "h"},
    }, {
        {"help", "h"},
    }));
}

TEST_CASE("Fail on flag with a parameter", "[parse]") {
    for (const auto& option : std::vector<const char*>({"--help=true", "-help=true", "--h=true", "-h=true"})) {
        auto arguments = std::vector<const char*>({"./program", option});
        REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {}, {{"help", "h"}}));
    }
}

TEST_CASE("Fail on option without a parameter", "[parse]") {
    for (const auto& option : std::vector<const char*>({"--verbose", "-verbose", "--v", "-v"})) {
        auto arguments = std::vector<const char*>({"./program", option});
        REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {{"verbose", "v"}}, {}));
    }
}

TEST_CASE("Fail on unknown option", "[parse]") {
    for (const auto& option : std::vector<const char*>({"--verbose", "-verbose", "--v", "-v"})) {
        auto arguments = std::vector<const char*>({"./program", option});
        REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {}, {{"help", "h"}}));
    }
}

TEST_CASE("Fail on unexpected characters", "[parse]") {
    for (const auto& option : std::vector<const char*>({"-", "--"})) {
        auto arguments = std::vector<const char*>({"./program", option});
        REQUIRE_THROWS(pontella::parse(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), 0, {}, {{"help", "h"}}));
    }
}

TEST_CASE("Test a command line for a flag", "[test]") {
    for (const auto& option : std::vector<const char*>({"--help", "-help", "--h", "-h"})) {
        auto arguments = std::vector<const char*>({"./program", option});
        REQUIRE(pontella::test(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), {"help", "h"}));
    }
    for (const auto& option : std::vector<const char*>({"--help", "-help", "--h", "-h"})) {
        auto arguments = std::vector<const char*>({"./program", option});
        REQUIRE(!pontella::test(static_cast<int>(arguments.size()), const_cast<char**>(arguments.data()), {"verbose", "v"}));
    }
}
