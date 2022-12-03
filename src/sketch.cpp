
// i/o
#include <iostream>
#include <yaml-cpp/yaml.h>

// type constraints
#include <ranges>
#include <concepts>

// data types and structures
#include <unordered_set>
#include <string>

namespace ranges = std::ranges;

int main()
{
    std::unordered_set<std::string> const setting_names{
        "left", "right", "center", "fill"
    };
    auto const config = YAML::LoadFile("../assets/widget.yaml");

    if (not config) {
        std::cout << "Unable to load yaml config\n";
        return EXIT_FAILURE;
    }
    auto const horizontal_config = config["horizontal"];
    if (not horizontal_config) {
        std::cout << "Couldn't find \"horizontal\" settings in config\n";
        return EXIT_FAILURE;
    }
    if (not horizontal_config.IsScalar()) {
        std::cout << "Expecting \"horizontal\" settings to be a string\n";
        return EXIT_FAILURE;
    }
    std::string const horizontal_value = horizontal_config.as<std::string>();
    if (not setting_names.contains(horizontal_value)) {
        std::cout << "Expecting \"horizontal\" value to be one of: [";
        std::string sep = "";
        for (const auto& name : setting_names) {
            std::cout << sep << name;
            sep = ", ";
        }
        std::cout << "]\n";
        return EXIT_FAILURE;
    }
    std::cout << "Using \"" << horizontal_value << "\" for horizontal value\n";
    return EXIT_SUCCESS;
}
