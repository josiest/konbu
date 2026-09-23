#pragma once

namespace gold
{
namespace justification
{
/** Horizontal justification setting */
enum class horizontal
{
    left,   /** Widget should be left-justified */
    right,  /** Widget should be right-justified */
    center, /** Widget should be centered horizontally */
    fill    /** Widget should horizontally fill its layout */
};

/** Vertical justification setting */
enum class vertical
{
    top,    /** Widget should be anchored to the top */
    bottom, /** Widget should be anchored to the bottom */
    center, /** Widget should be centered vertically */
    fill    /** Widget should vertically fill its layout */
};
}

struct layout
{
    justification::horizontal horizontal = justification::horizontal::left;
    justification::vertical vertical = justification::vertical::top;
};

/** Define padding for a widget's layout. */
template<typename Field> requires std::is_arithmetic_v<Field>
struct padding{
    constexpr padding(Field left, Field right, Field top, Field bottom)
        : left{left}, right{right}, top{top}, bottom{bottom}
    {
    }
    constexpr padding(Field horizontal, Field vertical)
        : padding(horizontal, horizontal, vertical, vertical)
    {
    }
    constexpr padding(Field value)
        : padding(value, value, value, value)
    {
    }
    constexpr padding() : padding(0) {}

    Field left = 0;
    Field right = 0;
    Field top = 0;
    Field bottom = 0;
};
}
