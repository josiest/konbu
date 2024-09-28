#pragma once
#include <yaml-cpp/yaml.h>
#include <entt/meta/meta.hpp>
#include <entt/meta/factory.hpp>
#include "pi/konbu/read.hpp"

#include <cstdint>
#include <ranges>

inline namespace pi {
namespace konbu {

template<typename T>
auto reflect() { return entt::meta<T>(); }

template<typename T>
concept reflectable = requires {
    { reflect<T>() } -> std::same_as<entt::meta_factory<T>>;
};

namespace meta_helpers {
template<typename Value, std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read_meta(YAML::Node const & config,
               entt::meta_any & reflected_value,
               ErrorOutput & errors)
{
    Value value;
    if (read(config, value, errors)) {
        reflected_value = value;
        return true;
    }
    return false;
}

template<std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read_unsigned_integer(const YAML::Node & number_config,
                           entt::meta_any & number_value,
                           ErrorOutput & errors)
{
    const auto number_type = number_value.type();
    if (number_type.size_of() == sizeof(std::uint8_t)) {
        return read_meta<std::uint8_t>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(std::uint16_t)) {
        return read_meta<std::uint16_t>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(std::uint32_t)) {
        return read_meta<std::uint32_t>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(std::uint64_t)) {
        return read_meta<std::uint64_t>(number_config, number_value, errors);
    }

    namespace ranges = std::ranges;
    namespace views = std::views;

    YAML::Exception const error{ number_config.Mark(),
                                 "couldn't read unsigned integer of unknown size" };
    ranges::copy(views::single(error), back_inserter_preference(errors));
    return false;
}

template<std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read_signed_integer(YAML::Node const & number_config,
                         entt::meta_any & number_value,
                         ErrorOutput & errors)
{
    auto const number_type = number_value.type();
    if (number_type.size_of() == sizeof(std::int8_t)) {
        return read_meta<std::int8_t>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(std::int16_t)) {
        return read_meta<std::int16_t>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(std::int32_t)) {
        return read_meta<std::int32_t>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(std::int64_t)) {
        return read_meta<std::int64_t>(number_config, number_value, errors);
    }
    namespace ranges = std::ranges;
    namespace views = std::views;

    YAML::Exception const error{ number_config.Mark(),
                                 "couldn't read signed integer of unknown size" };
    ranges::copy(views::single(error), back_inserter_preference(errors));
    return false;
}

template<std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read_float(YAML::Node const & number_config,
                entt::meta_any & number_value,
                ErrorOutput & errors)
{
    auto const number_type = number_value.type();
    if (number_type.size_of() == sizeof(float)) {
        return read_meta<float>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(double)) {
        return read_meta<double>(number_config, number_value, errors);
    }
    if (number_type.size_of() == sizeof(long double)) {
        return read_meta<long double>(number_config, number_value, errors);
    }
    namespace ranges = std::ranges;
    namespace views = std::views;

    YAML::Exception const error{ number_config.Mark(),
                                 "couldn't read float type of unknown size" };
    ranges::copy(views::single(error), back_inserter_preference(errors));
    return false;
}

template<std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read_object(YAML::Node const & object_config,
                 entt::meta_any & object_value,
                 ErrorOutput & errors)
{
    using namespace entt::literals;
    auto const object_type = object_value.type();
    if (auto read_fn = object_type.func("konbu-read"_hs)) {
        return read_with_function(object_config, read_fn, object_value, errors);
    }
    if (object_type == entt::resolve<std::string>()) {
        return read_meta<std::string>(object_config, object_value, errors);
    }
    return false;
}
}

template<std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read(YAML::Node const & config, entt::meta_any & value, ErrorOutput & errors)
{
    using namespace meta_helpers;
    auto const type = value.type();
    if (type.is_integral()) {
        if (type.is_signed()) {
            return read_signed_integer(config, value, errors);
        }
        return read_unsigned_integer(config, value, errors);
    }
    if (type.is_arithmetic()) {
        return read_float(config, value, errors);
    }
    // if (type.is_enum()) {
    //     return read_enum(config, value, errors);
    // }
    // if (type.is_array()) {
    //     return read_array(config, value, errors);
    // }
    // if (type.is_sequence_container()) {
    //     return read_sequence(config, value, errors);
    // }
    // if (type.is_associative_container()) {
    //     return read_map(config, value, errors);
    // }
    if (type.is_class()) {
        return read_object(config, value, errors);
    }
    return false;
}
}
}