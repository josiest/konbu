#pragma once
#include <string>
#include <string_view>
#include <expected>

#include <yaml-cpp/yaml.h>
#include "pi/containers/lookup_table.hpp"
#include "konbu/read.hpp"

inline namespace gold {

namespace justification {
/** Horizontal justification setting */
enum class horizontal {
    left,   /** Widget should be left-justified */
    right,  /** Widget should be right-justified */
    center, /** Widget should be centered horizontally */
    fill    /** Widget should horizontally fill its layout */
};
pi::lookup_table<horizontal, std::string_view, 4>
constexpr horizontal_names {
    { horizontal::left,     "left" },
    { horizontal::right,    "right" },
    { horizontal::center,   "center" },
    { horizontal::fill,     "fill" }
};

/** Vertical justification setting */
enum class vertical {
    top,    /** Widget should be anchored to the top */
    bottom, /** Widget should be anchored to the bottom */
    center, /** Widget should be centered vertically */
    fill    /** Widget should vertically fill its layout */
};
pi::lookup_table<vertical, std::string_view, 4>
constexpr vertical_names {
    { vertical::top,    "top" },
    { vertical::bottom, "bottom" },
    { vertical::center, "center" },
    { vertical::fill,   "fill" }
};
}

inline std::string to_string(gold::justification::horizontal value)
{
    namespace just = gold::justification;
    return std::string{ just::horizontal_names.find(value)->second };
}
inline std::string to_string(gold::justification::vertical value)
{
    namespace just = gold::justification;
    return std::string{ just::vertical_names.find(value)->second };
}
}

inline namespace pi {

namespace konbu {
template<std::ranges::output_range<YAML::Exception> Errors>
bool read(YAML::Node const & config, gold::justification::horizontal & value, Errors & errors)
{
    namespace just = gold::justification;
    return read_lookup(config, value, just::horizontal_names, errors);
}

template<std::ranges::output_range<YAML::Exception> Errors>
bool read(YAML::Node const & config, gold::justification::vertical & value, Errors & errors)
{
    namespace just = gold::justification;
    return read_lookup(config, value, just::vertical_names, errors);
}
}
}