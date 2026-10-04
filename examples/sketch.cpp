#include <cstdlib>
#include <iostream>

#include "pi/konbu.hpp"
#include "gold.hpp"

struct SDL_Point
{
    int x, y;
};

template<>
struct std::tuple_size<SDL_Point>
{
    static constexpr std::size_t value = 2;
};

template<std::size_t I> requires (I < 2)
struct std::tuple_element<I, SDL_Point>
{
    using type = int;
};

struct window_settings
{
    [[=pi::serialized_name("options")]]
    std::vector<std::string> flag_names;
};

static constexpr auto begin(const SDL_Point & p) { return &p.x; }
static constexpr auto begin(SDL_Point & p) { return &p.x; }
static constexpr auto end(const SDL_Point & p) { return &p.y+1; }
static constexpr auto end(SDL_Point & p) { return &p.y+1; }

template<typename EnumType> requires std::is_scoped_enum_v<EnumType>
static std::string read_enum(std::string_view src)
{
    return pi::try_read_yaml<EnumType>(YAML::Load(src.data()))
        .transform([](EnumType value) { return pi::scoped_enum_name(value); })
        .value_or(std::string("<unknown>"));
}

static void read_justification_as_horizontal_and_vertical(std::string_view src)
{
    using Horizontal = gold::justification::horizontal;
    using Vertical = gold::justification::vertical;
    std::cout << read_enum<Horizontal>(src) << "\n";
    std::cout << read_enum<Vertical>(src) << "\n";
}

static void read_justifications_example()
{
    read_justification_as_horizontal_and_vertical("fill");
    read_justification_as_horizontal_and_vertical("bottom");
    read_justification_as_horizontal_and_vertical("left");
    read_justification_as_horizontal_and_vertical("center");
}

static void print(const gold::layout & layout)
{
    std::cout << "[" << pi::enum_name(layout.horizontal) << ", " << pi::enum_name(layout.vertical) << "]";
}

template<typename T> requires std::is_arithmetic_v<T>
static void print(const gold::padding<T> & padding)
{
    std::cout << "[" << padding.left << ", " << padding.right << ", " << padding.top << ", " << padding.bottom << "]";
}

static void print(const SDL_Point & p)
{
    std::cout << "(" << p.x << ", " << p.y << ")";
}

template<typename T>
concept printable = requires(T val)
{
    print(val);
};

static void println(const printable auto & val)
{
    print(val); std::cout << "\n";
}

static void read_layout_example()
{
    constexpr std::string_view layout_src =
        "horizontal: right\n"
        "vertical: center\n";
    const YAML::Node layout_node = YAML::Load(layout_src.data());

    gold::layout layout;
    pi::read_yaml(layout_node, layout);
    println(layout);

    constexpr std::string_view point_src = "[3, 4]";
    const YAML::Node point_node = YAML::Load(point_src.data());
    SDL_Point p;
    pi::read_yaml(point_node, p);
    println(p);
    static_assert(pi::ranged_tuple<SDL_Point>);

    std::cout << "Is padding aggregate? "
              << (std::is_aggregate_v<gold::padding<float>>? "Yes\n" : "No\n");
}

static void read_padding_example()
{
    constexpr std::string_view src = "[15, 30, 10 , 20]";
    const YAML::Node node = YAML::Load(src.data());
    if (const auto padding = pi::try_read_yaml_constructible<gold::padding<float>>(node)) { println(*padding); }
}

static void read_vector_example()
{
    constexpr std::string_view src = "[video, audio, gamepad]";
    const YAML::Node node = YAML::Load(src.data());

    static_assert(pi::yaml_convertible<std::string>);
    if (std::vector<std::string> flag_names; pi::read_yaml<std::string>(node, flag_names))
    {
        for (const auto & name : flag_names)
        std::cout << name << "\n";
    }
}

static void read_serialized_name_example()
{
    constexpr std::string_view src = "options: [opengl, resizable]";
    const YAML::Node node = YAML::Load(src.data());
    if (window_settings settings; pi::read_yaml(node, settings))
    {
        for (const auto & name : settings.flag_names)
        {
            std::cout << name << "\n";
        }
    }

}

int main()
{
    // read_justifications_example();
    // read_layout_example();
    // read_padding_example();
    // read_vector_example();
    read_serialized_name_example();
    return EXIT_SUCCESS;
}
