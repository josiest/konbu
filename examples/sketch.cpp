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
/** Define how a widget will be justified in the layout */
struct layout {
    just::horizontal horz = just::horizontal::left;
    just::vertical vert = just::vertical::top;
};

/** Define padding for a widget's layout. */
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
        { just::horizontal::left,   "left" },
        { just::horizontal::right,  "right" },
        { just::horizontal::center, "center" },
        { just::horizontal::fill,   "fill" }
    };
    return names.find(horz)->second;
}

std::string to_string(just::vertical const &vert) {
    using namemap = std::unordered_map<just::vertical, std::string>;
    static namemap const names{
        { just::vertical::top,    "top" },
        { just::vertical::bottom, "bottom" },
        { just::vertical::center, "center" },
        { just::vertical::fill,   "fill" }
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
    // read the errors first into an isolated list, so that we can
    // re-contextualize them before copying them into the main error list
    std::vector<YAML::Exception> read_errors;
    read_lookup(config, horz, as_horizontal_justification, read_errors);

    // let the reader know that the error happened when parsing
    // horizontal justification
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
    // read the errors first into an isolated list, so that we can
    // re-contextualize them before copying them into the main error list
    std::vector<YAML::Exception> read_errors;
    read_lookup(config, vert, as_vertical_justification, read_errors);

    // let the reader know that the error happened when parsing
    // vertical justification
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
    // Won't be able to parse any data if the layout config isn't a map,
    // so we'll need to short-circuit if it isn't
    if (not config.IsMap()) {
        YAML::Exception const error{
            config.Mark(), "expecting \"layout\" settings to be a map\n"
        };
        ranges::copy(views::single(error), back_inserter_preference(errors));
        return;
    }
    // read the errors first into an isolated list, so that we can
    // re-contextualize them before copying them into the main error list
    std::vector<YAML::Exception> not_contextualized;
    if (auto const horizontal_config = config["horizontal"]) {
        read(horizontal_config, layout.horz, not_contextualized);
    }
    if (auto const vertical_config = config["vertical"]) {
        read(vertical_config, layout.vert, not_contextualized);
    }
    // let the reader know that the error happened when parsing layout settings
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
requires std::is_arithmetic_v<number> and (not std::same_as<number, bool>)

void read(YAML::Node const & config,
          gold::padding<number> & padding,
          error_output & errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;

    // errors will be written here first, before re-contextualizing and
    // copied into the main error list
    std::vector<YAML::Exception> un_contextualized;
    // let the reader know the error happened while parsing padding settings
    auto contextualize = [](YAML::Exception const & error) {
        std::stringstream message;
        message << "couldn't read padding value\n  " << error.msg
                // the default-values for padding members should all be the same
                // so we can just show the default left value
                << "\n  using default value of " << gold::padding<number>{}.left;
        return YAML::Exception{ error.mark, message.str() };
    };
    // inspired by Unreal's UMG widget padding component, there are three ways
    // to specify:
    // - "padding: <N>" -> all padding members use the value N
    // - "padding: [<H>, <V>]" ->
    //      horizontal members use the value H
    //      vertical members use the value V
    // - "padding: [<L>, <R>, <T>, <B>]" ->
    //      left and right members use the values L and R, respectively
    //      top and bottom members use the values T and B, respectively

    // case "padding: <N>"
    if (config.IsScalar()) {
        read(config, padding.left, un_contextualized);
        padding.right = padding.left;
        padding.top = padding.left;
        padding.bottom = padding.left;
    }
    // case "padding: [<H>, <V>]"
    else if (config.IsSequence() and config.size() == 2) {
        read(config[0], padding.left, un_contextualized);
        padding.right = padding.left;
        read(config[1], padding.top, un_contextualized);
        padding.bottom = padding.top;
    }
    // case "padding: [<L>, <R>, <T>, <B>]"
    else if (config.IsSequence() and config.size() == 4) {
        read(config[0], padding.left, un_contextualized);
        read(config[1], padding.right, un_contextualized);
        read(config[2], padding.top, un_contextualized);
        read(config[3], padding.bottom, un_contextualized);
    }
    // config is a sequence, but has the incorrect number of elements
    else if (config.IsSequence()) {
        YAML::Exception const error{
            config.Mark(),
            "expecting either 1, 2 or 4 padding parameters"
        };
        ranges::copy(views::single(error),
                     back_inserter_preference(un_contextualized));
    }
    // config was not a number or a sequence
    else {
        YAML::Exception const error{
            config.Mark(),
            "expecting a number or a sequence"
        };
        ranges::copy(views::single(error),
                     back_inserter_preference(un_contextualized));
    }
    if (not un_contextualized.empty()) {
        ranges::copy(un_contextualized | views::transform(contextualize),
                     back_inserter_preference(errors));
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
    // Won't be able to parse any data if the config couldn't load, or if isn't
    // a map, so we'll need to short-circuit in those cases
    if (not config) {
        std::cout << "Unable to load yaml config\n";
        return EXIT_FAILURE;
    }
    if (not config.IsMap()) {
        std::cout << "Expecting config to be a map\n";
        return EXIT_FAILURE;
    }
    std::vector<YAML::Exception> errors;
    // layout and padding have reasonable defaults, so if they're not specified
    // in the config, that's fine
    gold::layout layout;
    if (auto const layout_config = config["layout"]) {
        konbu::read(layout_config, layout, errors);
    }
    gold::padding<float> padding;
    if (auto const padding_config = config["padding"]) {
        konbu::read(padding_config, padding, errors);
    }
    // if we ran into any errors parsing the config file,
    // write them to the console here
    ranges::for_each(errors | views::transform(&YAML::Exception::what),
                     print_error);

    // finally, display the values that ended up being used
    std::cout << "Using \"" << gold::to_string(layout.horz)
                            << "\" for horizontal value\n"
              << "  and \"" << gold::to_string(layout.vert)
                            << "\" for vertical value\n";

    std::cout << "Padding: [" << padding.left << ", " << padding.right << ", "
                              << padding.top << ", " << padding.bottom << "]\n";
    return EXIT_SUCCESS;
}
