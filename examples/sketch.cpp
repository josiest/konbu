#include <cstdlib>
#include <iostream>

#include <yaml-cpp/yaml.h>

#include "pi/konbu.hpp"
#include "gold.hpp"

template<typename EnumType> requires std::is_scoped_enum_v<EnumType>
static std::string read_enum(std::string_view src)
{
    return pi::try_read_yaml<EnumType>(YAML::Node(src))
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

int main()
{
    read_justification_as_horizontal_and_vertical("fill");
    read_justification_as_horizontal_and_vertical("bottom");
    read_justification_as_horizontal_and_vertical("left");
    return EXIT_SUCCESS;
}
