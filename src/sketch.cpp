
// i/o
#include <iostream>
#include <yaml-cpp/yaml.h>

// type constraints
#include <iterator>
#include <ranges>

// data types and structures
#include <unordered_set>
#include <unordered_map>
#include <string>

namespace ranges = std::ranges;
namespace views = std::views;

namespace konbu {
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

template<typename container>
concept can_push_back =
requires(container & c, container::value_type const & v)
{
    c.push_back(v);
};

template<typename container>
using lookup_value_t = typename container::value_type;

template<typename container>
using lookup_key_t = typename container::key_type;

template <typename container>
using lookup_mapped_t = typename container::mapped_type;

template<typename container>
concept lookup_table =
requires(container const & c, container::key_type const & key)
{
    { c.find(key) } -> std::indirectly_readable;
    { c.find(key)->first } -> std::convertible_to<lookup_key_t<container>>;
    { c.find(key)->second } -> std::convertible_to<lookup_mapped_t<container>>;
};

template<ranges::output_range<YAML::Exception> error_output,
         lookup_table name_lookup>
requires can_push_back<error_output> and
         std::convertible_to<std::string, lookup_key_t<name_lookup>> and
         std::convertible_to<lookup_mapped_t<name_lookup>, just::horizontal>

void expect_lookup(YAML::Node const & config,
                   just::horizontal & horz,
                   name_lookup const & lookup,
                   error_output & errors)
{
    if (not config.IsScalar()) {
        YAML::Exception const error{ config.Mark(), "Expecting a string" };
        ranges::copy(views::single(error),
                     std::back_inserter(errors));
        return;
    }
    auto const search = lookup.find(config.as<std::string>());
    if (search != lookup.end()) {
        horz = search->second;
        return;
    }
    std::stringstream message;
    message << "expecting value to be one of the following: [";
    std::string sep = "";
    for (const auto & name : lookup | views::keys) {
        message << sep << name;
        sep = ", ";
    }
    message << "]";
    YAML::Exception const error{ config.Mark(), message.str() };
    ranges::copy(views::single(error),
                 std::back_inserter(errors));
}

template<ranges::output_range<YAML::Exception> error_output>
requires can_push_back<error_output>
void expect(YAML::Node const & config,
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
    return expect_lookup(config, horz, as_horizontal_justification, errors);
}

void print_error(std::string const & message) {
    std::cout << message << "\n";
}
}

namespace just = konbu::just;
std::ostream & operator<<(std::ostream & os, just::horizontal const & horz)
{
    using namemap = std::unordered_map<just::horizontal, std::string>;
    static namemap const names{
        { just::horizontal::left,   "left" },
        { just::horizontal::right,  "right" },
        { just::horizontal::center, "center" },
        { just::horizontal::fill,   "fill" }
    };
    os << names.find(horz)->second;
    return os;
}

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
    namespace just = konbu::just;

    std::vector<YAML::Exception> errors;
    just::horizontal horz;
    konbu::expect(horizontal_config, horz, errors);
    ranges::for_each(errors | views::transform(&YAML::Exception::what),
                     konbu::print_error);

    std::cout << "Using \"" << horz << "\" for horizontal value\n";
    return EXIT_SUCCESS;
}
