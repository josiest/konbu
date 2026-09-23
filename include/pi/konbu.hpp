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

// Concepts
template<typename T>
concept yaml_convertible = requires(YAML::Node node, T val)
{
    requires std::default_initializable<T>;
    { YAML::convert<T>::decode(node, val) } -> std::same_as<bool>;
    { YAML::convert<T>::encode(val) } -> std::same_as<YAML::Node>;
};

template<typename T>
concept ranged_tuple = requires (T value)
{
    requires std::ranges::range<T>;
    requires std::tuple_size_v<T> == std::ranges::size(value);
    requires std::same_as<std::tuple_element_t<0, T>, std::ranges::range_value_t<T>>;
};

// Enum Helpers
template<typename EnumType> requires std::is_enum_v<EnumType>
constexpr std::string_view enum_name(EnumType value);

template<typename EnumType> requires std::is_scoped_enum_v<EnumType>
constexpr std::string scoped_enum_name(EnumType value);

// Annotation Helpers
consteval std::string_view serialized_name_of(std::meta::info info);

// Read YAML interface
// ===================
template<yaml_convertible T>
bool read_yaml(const YAML::Node & node, T & value);

template<typename EnumT>
requires (not yaml_convertible<EnumT>) and std::is_enum_v<EnumT>
bool read_yaml(const YAML::Node & node, EnumT & value);

template<ranged_tuple TupleT>
requires (not yaml_convertible<TupleT>)
bool read_yaml(const YAML::Node & node, TupleT & value);

template<std::default_initializable AggregateT>
requires (not yaml_convertible<AggregateT>) and (not ranged_tuple<AggregateT>) and std::is_aggregate_v<AggregateT>
bool read_yaml(const YAML::Node & node, AggregateT & value);

template<typename T>
requires yaml_convertible<T> or std::is_enum_v<T> or std::is_aggregate_v<T> or ranged_tuple<T>
bool read_yaml(const YAML::Node & node, std::vector<T> & values);

template<std::default_initializable T>
std::optional<T> try_read_yaml(const YAML::Node & node);

template<typename T>
std::optional<T> try_read_yaml_constructible(const YAML::Node & node);
}

// Helpers Implementation
// ======================

template<typename EnumType> requires std::is_enum_v<EnumType>
constexpr std::string_view pi::enum_name(EnumType value)
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
constexpr std::string pi::scoped_enum_name(EnumType value)
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

consteval std::string_view pi::serialized_name_of(std::meta::info info)
{
    return std::meta::identifier_of(info);
}

// Read YAML Implementation
// ========================

template<pi::yaml_convertible T>
bool pi::read_yaml(const YAML::Node & node, T & value)
{
    return YAML::convert<T>::decode(node, value);
}

template<typename EnumT>
requires (not pi::yaml_convertible<EnumT>) and std::is_enum_v<EnumT>
bool pi::read_yaml(const YAML::Node & node, EnumT & value)
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

template<pi::ranged_tuple TupleT>
requires (not pi::yaml_convertible<TupleT>)
bool pi::read_yaml(const YAML::Node & node, TupleT & value)
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
requires (not pi::yaml_convertible<AggregateT>) and (not pi::ranged_tuple<AggregateT>)
     and std::is_aggregate_v<AggregateT>

bool pi::read_yaml(const YAML::Node & node, AggregateT & value)
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

template<typename T>
requires pi::yaml_convertible<T> or std::is_enum_v<T> or std::is_aggregate_v<T> or pi::ranged_tuple<T>
bool pi::read_yaml(const YAML::Node & node, std::vector<T> & values)
{
    if (not node.IsDefined() or node.IsNull() or not node.IsSequence()) { return false; }
    bool success = true;
    for (const YAML::Node element_node : node)
    {
        if (T value; read_yaml(element_node, value))
        {
            values.emplace_back(std::move(value));
        }
        else
        {
            success = false;
        }
    }
    return success;
}

template<std::default_initializable T>
std::optional<T> pi::try_read_yaml(const YAML::Node & node)
{
    if (T value; read_yaml(node, value))
    {
        return std::optional<T>(std::in_place, value);
    }
    return std::nullopt;
}

template<typename T>
std::optional<T> pi::try_read_yaml_constructible(const YAML::Node & node)
{
    if (not node.IsDefined() or node.IsNull() or not (node.IsScalar() or node.IsSequence()))
    {
        return std::nullopt;
    }

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto member : std::define_static_array(std::meta::members_of(^^T, ctx)))
    {
        if constexpr (std::meta::is_constructor(member) and not std::meta::is_default_constructor(member)
                      and not std::meta::is_copy_constructor(member) and not std::meta::is_move_constructor(member))
        {
            constexpr std::size_t parameter_size = std::meta::parameters_of(member).size();
            if (parameter_size == 1 and node.IsScalar())
            {
                constexpr auto parameter_type = std::meta::type_of(std::meta::parameters_of(member)[0]);
                if constexpr (std::meta::is_default_constructible_type(parameter_type))
                {
                    typename [:parameter_type:] value;
                    read_yaml(node, value);
                    return std::optional<T>(std::in_place, value);
                }
            }
            else if (node.IsSequence() and parameter_size == node.size())
            {
                std::cout << std::meta::display_string_of(member) << "\n";
            }
        }
    }
    return std::nullopt;
}