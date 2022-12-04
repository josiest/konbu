
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
template<typename T>
concept string_convertible = requires(T const &t) {
    { to_string(t) } -> std::convertible_to<std::string>;
};
}

template<konbu::string_convertible T>
std::ostream & operator<<(std::ostream & os, T const & t)
{
    return os << to_string(t);
}

namespace konbu {
template<typename container>
concept can_push_back =
requires(container & c, container::value_type const & v)
{
    c.push_back(v);
};
template<typename container>
concept can_push_front =
requires(container & c, container::value_type const & v)
{
    c.push_front(v);
};

template<ranges::range container>
auto back_inserter_preference(container & c)
{
    return std::inserter(c, ranges::begin(c));
}
template<ranges::range container>
requires can_push_back<container>
auto back_inserter_preference(container & c)
{
    return std::back_inserter(c);
}
template<ranges::range container>
requires can_push_front<container> and (not can_push_back<container>)
auto back_inserter_preference(container & c)
{
    return std::front_inserter(c);
}

template<typename container>
using lookup_key_t = typename container::key_type;

template <typename container>
using lookup_mapped_t = typename container::mapped_type;

template<typename container>
concept lookup_table =
requires(container const & c, lookup_key_t<container> const & key)
{
    { c.find(key) } -> std::indirectly_readable;
    { c.find(key)->first } -> std::convertible_to<lookup_key_t<container>>;
    { c.find(key)->second } -> std::convertible_to<lookup_mapped_t<container>>;
};

template<ranges::output_range<YAML::Exception> error_output,
         lookup_table name_lookup>
requires std::convertible_to<std::string, lookup_key_t<name_lookup>>

void read_lookup(YAML::Node const & config,
                 lookup_mapped_t<name_lookup> & value,
                 name_lookup const & lookup,
                 error_output & errors)
{
    if (not config.IsScalar()) {
        YAML::Exception const error{ config.Mark(), "expecting a string" };
        ranges::copy(views::single(error),
                     back_inserter_preference(errors));
        return;
    }
    auto const search = lookup.find(config.as<std::string>());
    if (search != lookup.end()) {
        value = search->second;
        return;
    }
    std::stringstream message;
    message << "\n    expecting value to be one of the following: [";
    std::string sep;
    for (const auto & name : lookup | views::keys) {
        message << sep << name;
        sep = ", ";
    }
    message << "]";
    YAML::Exception const error{ config.Mark(), message.str() };
    ranges::copy(views::single(error),
                 back_inserter_preference(errors));
}

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
                << horz << "\"";
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
                << vert << "\"";
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

    std::cout << "Using \"" << layout.horz << "\" for horizontal value\n"
              << "  and \"" << layout.vert << "\" for vertical value\n";
    return EXIT_SUCCESS;
}
