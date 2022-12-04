#include "konbu/konbu.h"

// i/o
#include <iostream>
#include <yaml-cpp/yaml.h>

// type constraints and algorithms
#include <ranges>
#include <algorithm>

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

template<typename number>
requires std::is_arithmetic_v<number> and
        (not std::same_as<number, bool>)
struct padding{
    number left = 0;
    number right = 0;
    number top = 0;
    number bottom = 0;
};

std::string to_string(just::horizontal const &horz) {
    using namemap = std::unordered_map<just::horizontal, std::string>;
    static namemap const names{
        {just::horizontal::left,   "left"},
        {just::horizontal::right,  "right"},
        {just::horizontal::center, "center"},
        {just::horizontal::fill,   "fill"}
    };
    return names.find(horz)->second;
}

std::string to_string(just::vertical const &vert) {
    using namemap = std::unordered_map<just::vertical, std::string>;
    static namemap const names{
        {just::vertical::top,    "top"},
        {just::vertical::bottom, "bottom"},
        {just::vertical::center, "center"},
        {just::vertical::fill,   "fill"}
    };
    return names.find(vert)->second;
}
}

namespace just = gold::just;
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
                << gold::to_string(horz) << "\"";
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
                << gold::to_string(vert) << "\"";
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

template<typename number,
         std::ranges::output_range<YAML::Exception> error_output>
requires std::is_arithmetic_v<number> and
         (not std::same_as<number, bool>)
void read(YAML::Node const & config,
          gold::padding<number> & padding,
          error_output & errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;

    std::vector<YAML::Exception> un_contextualized;
    auto contextualize = [](YAML::Exception const & error) {
        std::stringstream message;
        message << "couldn't read padding value\n  " << error.msg
                << "\n  using default value of " << gold::padding<number>{}.left;
        return YAML::Exception{ error.mark, message.str() };
    };
    if (config.IsScalar()) {
        number padding_size = padding.left;
        read(config, padding_size, un_contextualized);
        if (un_contextualized.empty()) {
            padding.left = padding_size;
            padding.right = padding_size;
            padding.top = padding_size;
            padding.bottom = padding_size;
        }
    }
    else if (config.IsSequence() and config.size() == 2) {
        number horizontal_padding = padding.left;
        read(config[0], horizontal_padding, un_contextualized);

        if (un_contextualized.empty()) {
            padding.left = horizontal_padding;
            padding.right = horizontal_padding;
        }
        auto const num_errors = un_contextualized.size();
        number vertical_padding = padding.top;
        read(config[1], vertical_padding, un_contextualized);

        if (num_errors == un_contextualized.size()) {
            padding.top = vertical_padding;
            padding.bottom = vertical_padding;
        }
    }
    else if (config.IsSequence() and config.size() == 4) {
        number left_padding;
        read(config[0], left_padding, un_contextualized);
        if (un_contextualized.empty()) {
            padding.left = left_padding;
        }
        auto const num_left_errors = un_contextualized.size();
        number right_padding;
        read(config[1], right_padding, un_contextualized);
        if (num_left_errors == un_contextualized.size()) {
            padding.right = right_padding;
        }
        auto const num_right_errors = un_contextualized.size();
        number top_padding;
        read(config[2], top_padding, un_contextualized);
        if (num_right_errors == un_contextualized.size()) {
            padding.top = top_padding;
        }
        auto const num_top_errors = un_contextualized.size();
        number bottom_padding;
        read(config[3], bottom_padding, un_contextualized);
        if (num_top_errors == un_contextualized.size()) {
            padding.bottom = bottom_padding;
        }
    }
    else if (config.IsSequence()) {
        YAML::Exception const error{
            config.Mark(),
            "expecting either 1, 2 or 4 padding parameters"
        };
        ranges::copy(views::single(error),
                     back_inserter_preference(un_contextualized));
    }
    else {
        YAML::Exception const error{
            config.Mark(),
            "expecting a number or a sequence"
        };
        ranges::copy(views::single(error),
                     back_inserter_preference(un_contextualized));
    }
    if (not un_contextualized.empty()) {
        ranges::transform(un_contextualized,
                          back_inserter_preference(errors),
                          contextualize);
    }
}
}

void print_error(std::string const & error)
{
    std::cout << error << "\n\n";
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
    auto const padding_config = config["padding"];
    if (not padding_config) {
        std::cout << "Couldn't find \"padding\" settings in config\n";
        return EXIT_FAILURE;
    }

    std::vector<YAML::Exception> errors;

    gold::layout layout;
    konbu::read(layout_config, layout, errors);

    gold::padding<float> padding;
    konbu::read(padding_config, padding, errors);

    ranges::for_each(errors | views::transform(&YAML::Exception::what),
                     print_error);

    std::cout << "Using \"" << gold::to_string(layout.horz)
                            << "\" for horizontal value\n"
              << "  and \"" << gold::to_string(layout.vert)
                            << "\" for vertical value\n";

    std::cout << "Padding: [" << padding.left << ", " << padding.right << ", "
                              << padding.top << ", " << padding.bottom << "]\n";
    return EXIT_SUCCESS;
}
