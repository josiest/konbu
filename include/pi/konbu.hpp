#pragma once
#include <meta>
#include <concepts>

#include <string>
#include <sstream>
#include <string_view>

#include <yaml-cpp/yaml.h>

namespace pi
{
template<typename EnumType> requires std::is_enum_v<EnumType>
constexpr std::string_view enum_name(EnumType value)
{
    template for (constexpr auto element : std::define_static_array(std::meta::enumerators_of(^^EnumType)))
    {
        if ([:element:] == value)
        {
            return std::meta::identifier_of(element);
        }
    }
    return "<unknown>";
}

template<typename EnumType> requires std::is_scoped_enum_v<EnumType>
constexpr std::string scoped_enum_name(EnumType value)
{
    template for (constexpr auto element : std::define_static_array(std::meta::enumerators_of(^^EnumType)))
    {
        if ([:element:] == value)
        {
            std::stringstream name_stream;
            name_stream << std::meta::identifier_of(^^EnumType) << "::" << std::meta::identifier_of(element);
            return name_stream.str();
        }
    }
    return "<unknown>";
}

template<typename DecodeFn, typename T>
concept yaml_decoder_for = requires(YAML::Node node, T val)
{
    { DecodeFn(node, val) } -> std::same_as<bool>;
};

template<typename Encode, typename T>
concept yaml_encoder_for = requires(T val)
{
    { EncodeFn(val) } -> std::same_as<YAML::Node>;
};

template<typename T>
concept yaml_convertible = requires
{
    requires std::default_initializable<T>;
    { YAML::convert<T>::decode } -> yaml_decoder_for<T>;
    { YAML::convert<T>::encode } -> yaml_encoder_for<T>;
};

template<yaml_convertible T>
bool read_yaml(const YAML::Node & node, T & value)
{
    return YAML::convert<T>::decode(node, value);
}

template<yaml_convertible T>
std::optional<T> try_read_yaml(const YAML::Node & node)
{
    T value;
    if (not YAML::convert<T>::decode(node, value)) { return std::nullopt; }
    return std::optional<T>(std::in_place, std::move(value));
}

template<typename EnumT>
requires (not yaml_convertible<EnumT>) and std::is_enum_v<EnumT>
std::optional<EnumT> try_read_yaml(const YAML::Node & node)
{
    if (not node.IsDefined() or node.IsNull() or not node.IsScalar()) { return std::nullopt; }
    template for (constexpr auto element : std::define_static_array(std::meta::enumerators_of(^^EnumT)))
    {
        if (std::meta::identifier_of(element) == node.Scalar())
        {
            return std::optional<EnumT>(std::in_place, [:element:]);
        }
    }
    return std::nullopt;
}

template<std::default_initializable T>
requires (not yaml_convertible<T>)
bool read_value(const YAML::Node & node, T & value)
{
    if (std::optional<T> read_result = try_read_yaml<T>(node))
    {
        value = *std::move(read_result);
        return true;
    }
    return false;
}
}
