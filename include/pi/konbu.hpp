#pragma once
#include <meta>
#include <concepts>

#include <regex>
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

consteval std::string_view serialized_name_of(std::meta::info info)
{
    return std::meta::identifier_of(info);
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
concept yaml_convertible = requires(YAML::Node node, T val)
{
    requires std::default_initializable<T>;
    { YAML::convert<T>::decode(node, val) } -> std::same_as<bool>;
    { YAML::convert<T>::encode(val) } -> std::same_as<YAML::Node>;
};

template<yaml_convertible T>
bool read_yaml(const YAML::Node & node, T & value)
{
    return YAML::convert<T>::decode(node, value);
}

template<typename EnumT>
requires (not yaml_convertible<EnumT>) and std::is_enum_v<EnumT>
bool read_yaml(const YAML::Node & node, EnumT & value)
{
    if (not node.IsDefined() or node.IsNull() or not node.IsScalar()) { return false; }
    template for (constexpr auto element : std::define_static_array(std::meta::enumerators_of(^^EnumT)))
    {
        if (std::meta::identifier_of(element) == node.Scalar())
        {
            value = [:element:];
            return true;
        }
    }
    return false;
}

template<typename T>
concept tuple_like = requires
{
    { std::tuple_size_v<T> } -> std::same_as<const std::size_t &>;
    typename std::tuple_element<0uz, T>::type;
};

template<tuple_like TupleT>
requires (not yaml_convertible<TupleT>) and std::ranges::range<TupleT>
bool read_yaml(const YAML::Node & node, TupleT & value)
{
    if (not node.IsDefined() or node.IsNull() or not node.IsSequence()
                             or node.size() != std::tuple_size_v<TupleT>) { return false; }

    bool success = true;
    auto * it = std::ranges::begin(value);
    template for (constexpr std::size_t I : std::views::iota(0uz, std::tuple_size_v<TupleT>))
    {
        success = success and read_yaml(node[I], *it);
        it = std::ranges::next(it, 1, std::ranges::end(value));
    }
    return success;
}

template<std::default_initializable AggregateT>
requires (not yaml_convertible<AggregateT>) and (not tuple_like<AggregateT>) and std::is_aggregate_v<AggregateT>
bool read_yaml(const YAML::Node & node, AggregateT & value)
{
    if (not node.IsDefined() or node.IsNull() or not node.IsMap()) { return false; }
    constexpr auto ctx = std::meta::access_context::current();
    bool success = true;
    template for (constexpr auto member
        : std::define_static_array(std::meta::nonstatic_data_members_of(^^AggregateT, ctx)))
    {
        if (const YAML::Node member_node = node[serialized_name_of(member).data()])
        {
            success = success and read_yaml(member_node, value.[:member:]);
        }
    }
    return success;
}

template<std::default_initializable T>
std::optional<T> try_read_yaml(const YAML::Node & node)
{
    if (T value; read_yaml(node, value))
    {
        return std::optional<T>(std::in_place, value);
    }
    return std::nullopt;
}
}
