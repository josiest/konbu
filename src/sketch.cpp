#include "konbu/konbu.h"

// i/o
#include <iostream>
#include <yaml-cpp/yaml.h>

// type constraints
#include <ranges>

// data types and structures
#include <unordered_map>
#include <vector>
#include <string>

namespace ranges = std::ranges;
namespace views = std::views;

namespace gold {
namespace just {
/** Horizontal justification setting */
enum class horizontal {
    left,   /** Widget should be left-justified */
    right,  /** Widget should be right-justified */
    center, /** Widget should be centered horizontally */
    fill    /** Widget should horizontally fill its layout */
};
/** Vertical justification setting */
enum class vertical {
    top,    /** Widget should be anchored to the top */
    bottom, /** Widget should be anchored to the bottom */
    center, /** Widget should be centered vertically */
    fill    /** Widget should vertically fill its layout */
};
}
struct layout {
    just::horizontal horz = just::horizontal::left;
    just::vertical vert = just::vertical::top;
};
}
namespace just = gold::just;
std::string to_string(just::horizontal const & horz)
{
    using namemap = std::unordered_map<just::horizontal, std::string>;
    static namemap const names{
        { just::horizontal::left,   "left" },
        { just::horizontal::right,  "right" },
        { just::horizontal::center, "center" },
        { just::horizontal::fill,   "fill" }
    };
    return names.find(horz)->second;
}
std::string to_string(just::vertical const & vert)
{
    using namemap = std::unordered_map<just::vertical, std::string>;
    static namemap const names{
        { just::vertical::top,      "top" },
        { just::vertical::bottom,   "bottom" },
        { just::vertical::center,   "center" },
        { just::vertical::fill,     "fill" }
    };
    return names.find(vert)->second;
}


namespace konbu {
template<ranges::output_range<YAML::Exception> error_output>
void read(YAML::Node const & config,
          just::horizontal & horz,
          error_output & errors)
{
    static std::unordered_map<std::string, just::horizontal> const
    as_horizontal_justification {
        { "left",   just::horizontal::left },
        { "right",  just::horizontal::right },
        { "center", just::horizontal::center },
        { "fill",   just::horizontal::fill }
    };
    std::vector<YAML::Exception> read_errors;
    read_lookup(config, horz, as_horizontal_justification, read_errors);
    auto contextualize = [&horz](YAML::Exception const & no_context) {
        std::stringstream message;
        message << "couldn't read horizontal justification: "
                << no_context.msg << "\n  using default value \""
                << to_string(horz) << "\"";
        return YAML::Exception{ no_context.mark, message.str() };
    };
    ranges::copy(read_errors | views::transform(contextualize),
                 back_inserter_preference(errors));
}

template <ranges::output_range<YAML::Exception> error_output>
void read(YAML::Node const & config,
          just::vertical & vert,
          error_output & errors)
{
    static std::unordered_map<std::string, just::vertical> const
    as_vertical_justification {
       { "top",     just::vertical::top },
       { "bottom",  just::vertical::bottom },
       { "center",  just::vertical::center },
       { "fill",    just::vertical::fill }
    };
    std::vector<YAML::Exception> read_errors;
    read_lookup(config, vert, as_vertical_justification, read_errors);
    auto contextualize = [&vert](YAML::Exception const & no_context) {
        std::stringstream message;
        message << "couldn't read vertical justification: "
                << no_context.msg << "\n  using default value \""
                << to_string(vert) << "\"";
        return YAML::Exception{ no_context.mark, message.str() };
    };
    ranges::copy(read_errors | views::transform(contextualize),
                 back_inserter_preference(errors));
}

template<ranges::output_range<YAML::Exception> error_output>
void read(YAML::Node const & config,
          gold::layout & layout,
          error_output & errors)
{
    if (not config.IsMap()) {
        YAML::Exception const error{
            config.Mark(), "expecting \"layout\" settings to be a map\n"
        };
        ranges::copy(views::single(error), back_inserter_preference(errors));
        return;
    }
    std::vector<YAML::Exception> not_contextualized;
    if (auto const horizontal_config = config["horizontal"]) {
        read(horizontal_config, layout.horz, not_contextualized);
    }
    if (auto const vertical_config = config["vertical"]) {
        read(vertical_config, layout.vert, not_contextualized);
    }
    auto contextualize = [](YAML::Exception const & no_context) {
        std::stringstream message;
        message << "couldn't read layout settings\n  " << no_context.msg;
        return YAML::Exception{ no_context.mark, message.str() };
    };
    ranges::copy(not_contextualized | views::transform(contextualize),
                 back_inserter_preference(errors));
}

void print_error(std::string const & message) {
    std::cout << message << "\n\n";
}
}

int main()
{
    auto const config = YAML::LoadFile("../assets/widget.yaml");
    if (not config) {
        std::cout << "Unable to load yaml config\n";
        return EXIT_FAILURE;
    }
    auto const layout_config = config["layout"];
    if (not layout_config) {
        std::cout << "Couldn't find \"layout\" settings in config\n";
        return EXIT_FAILURE;
    }

    gold::layout layout;
    std::vector<YAML::Exception> errors;
    konbu::read(layout_config, layout, errors);
    ranges::for_each(errors | views::transform(&YAML::Exception::what),
                     konbu::print_error);

    std::cout << "Using \"" << to_string(layout.horz)
                            << "\" for horizontal value\n"
              << "  and \"" << to_string(layout.vert)
                            << "\" for vertical value\n";
    return EXIT_SUCCESS;
}
