#include "konbu/read.hpp"
#include "gold/justification.hpp"
#include "gold/layout.hpp"
#include "gold/padding.hpp"

// i/o
#include <filesystem>
#include <iostream>
#include <yaml-cpp/yaml.h>

// type constraints and algorithms
#include <ranges>
#include <algorithm>

// data types and structures
#include <vector>
#include <string>
#include <string_view>

namespace ranges = std::ranges;
namespace views = std::views;
namespace fs = std::filesystem;

void print_error(std::string const & error)
{
    std::cout << error << "\n\n";
}

void contextualize_with_filename(fs::path const & filename)
{
    std::cout << "Encountered errors reading widget config at \"" << filename << "\"\n";
}

void flush_errors(std::vector<YAML::Exception> & errors, fs::path const & filename)
{
    if (not errors.empty()) {
        contextualize_with_filename(filename);
    }
    ranges::for_each(errors | views::transform(&YAML::Exception::what), &print_error);
    errors.clear();
}

int main()
{
    auto const base_dir = fs::path("../..");
    auto const asset_dir = base_dir/"assets";
    auto const widget_config_filepath = asset_dir/"widget.yml";

    auto const config = YAML::LoadFile(widget_config_filepath.string());
    // Won't be able to parse any data if the config couldn't load, or if isn't
    // a map, so we'll need to short-circuit in those cases
    if (not config) {
        std::cout << "Unable to load widget config at \""
                  << widget_config_filepath << "\"\n";
        return EXIT_FAILURE;
    }
    if (not config.IsMap()) {
        contextualize_with_filename(widget_config_filepath);
        std::cout << "  Expecting config to be a map\n";
        return EXIT_FAILURE;
    }
    std::vector<YAML::Exception> errors;
    // layout and padding have reasonable defaults, so if they're not specified
    // in the config, that's fine
    gold::layout layout;
    if (auto const layout_config = config["layout"]) {
        konbu::read(layout_config, layout, errors);
        flush_errors(errors, widget_config_filepath);
    }
    gold::padding<float> padding;
    if (auto const padding_config = config["padding"]) {
        konbu::read(padding_config, padding, errors);
        flush_errors(errors, widget_config_filepath);
    }
    // finally, display the values that ended up being used
    std::cout << "Using \"" << gold::to_string(layout.horizontal) << "\" for horizontal value\n"
              << "  and \"" << gold::to_string(layout.vertical) << "\" for vertical value\n";

    std::cout << "Padding: [" << padding.left << ", " << padding.right << ", "
                              << padding.top << ", " << padding.bottom << "]\n";
    return EXIT_SUCCESS;
}
