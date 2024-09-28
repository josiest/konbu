#pragma once
#include <type_traits>
#include <concepts>
#include <iterator>

#include <yaml-cpp/yaml.h>
#include "pi/konbu/read.hpp"
#include "pi/konbu/meta.hpp"

inline namespace gold {

template<typename Number>
concept numeric = std::is_arithmetic_v<std::remove_cvref_t<Number>>
              and (not std::same_as<std::remove_cvref_t<Number>, bool>);

/** Define padding for a widget's layout. */
template<numeric Number>
struct padding{
    constexpr padding(Number left, Number right, Number top, Number bottom)
        : left{left}, right{right}, top{top}, bottom{bottom}
    {
    }
    constexpr padding(Number horizontal, Number vertical)
        : padding(horizontal, horizontal, vertical, vertical)
    {
    }
    constexpr padding(Number value)
        : padding(value, value, value, value)
    {
    }
    constexpr padding() : padding(0) {}

    Number left = 0;
    Number right = 0;
    Number top = 0;
    Number bottom = 0;
};

template<typename Container>
concept padding_like = requires(Container padding) {
    { padding.left } -> numeric;
    requires std::same_as<decltype(padding.right), decltype(padding.left)>;
    requires std::same_as<decltype(padding.top), decltype(padding.left)>;
    requires std::same_as<decltype(padding.bottom), decltype(padding.left)>;
};

}

inline namespace pi {
namespace konbu {

template<gold::padding_like Padding>
auto reflect()
{
    using namespace entt::literals;
    using Field = std::remove_cvref_t<decltype(std::declval<Padding>().left)>;
    return entt::meta<Padding>()
        .type("gold::padding"_hs)
        .template ctor<Field, Field, Field, Field>()
        .template ctor<Field, Field>()
        .template ctor<Field>()
        .template data<&Padding::left>("left"_hs)
        .template data<&Padding::right>("right"_hs)
        .template data<&Padding::top>("top"_hs)
        .template data<&Padding::bottom>("bottom"_hs);
}

template<padding_like Padding, std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read(YAML::Node const & config, Padding & padding, ErrorOutput & errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;

    // errors will be written here first, before re-contextualizing and
    // copied into the main error list
    std::vector<YAML::Exception> uncontextualized;
    // let the reader know the error happened while parsing padding settings
    auto contextualize = [](YAML::Exception const & error) {
        std::stringstream message;
        message << "couldn't read padding value\n  " << error.msg
                // the default-values for padding members should all be the same,
                // so we can just show the default left value
                << "\n  using default value of " << Padding{}.left;
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
    bool success = true;

    // case "padding: <N>"
    if (config.IsScalar()) {
        success = read(config, padding.left, uncontextualized);
        padding.right = padding.left;
        padding.top = padding.left;
        padding.bottom = padding.left;
    }
        // case "padding: [<H>, <V>]"
    else if (config.IsSequence() and config.size() == 2) {
        success = read(config[0], padding.left, uncontextualized);
        padding.right = padding.left;

        success = success and read(config[1], padding.top, uncontextualized);
        padding.bottom = padding.top;
    }
        // case "padding: [<L>, <R>, <T>, <B>]"
    else if (config.IsSequence() and config.size() == 4) {
        success = read(config[0], padding.left, uncontextualized);
        success = success and read(config[1], padding.right, uncontextualized);
        success = success and read(config[2], padding.top, uncontextualized);
        success = success and read(config[3], padding.bottom, uncontextualized);
    }
    // config is a sequence, but has the incorrect Number of elements
    else if (config.IsSequence()) {
        YAML::Exception const error{ config.Mark(),
                                     "expecting either 1, 2 or 4 padding parameters" };
        ranges::copy(views::single(error), std::back_inserter(uncontextualized));
        success = false;
    }
        // config was not a number or a sequence
    else {
        YAML::Exception const error{ config.Mark(), "expecting a number or a sequence" };
        ranges::copy(views::single(error), std::back_inserter(uncontextualized));
        success = false;
    }
    ranges::copy(uncontextualized | views::transform(contextualize),
                 back_inserter_preference(errors));
    return success;
}
}
}