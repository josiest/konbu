#pragma once
#include <yaml-cpp/yaml.h>
#include <iterator>
#include <concepts>
#include <ranges>
#include <cstdio>

#include <utility>
#include <regex>

#include <optional>
#include <expected>

#include <string>
#include <sstream>
#include <vector>

#include "pi/containers/lookup_table.hpp"

inline namespace pi {

namespace expect_helpers {
void print_error(std::string const & error)
{
    std::printf("%s\n", error.c_str());
}
}

template<typename To, typename From>
concept convertible_from = std::convertible_to<From, To>;

/**
 * \brief parse an arbitrary type from a name-lookup
 *
 * \tparam name_lookup      maps strings to value types
 * \tparam ErrorOutput     allocator-aware container of yaml-exceptions
 *
 * \param config    YAML string input
 * \param value     write parsed value to
 * \param lookup    maps names to their desired values
 * \param errors    write any parsing errors to
 */
template<typename Value,
         convertible_from<std::string> StringLike,
         std::size_t N, std::weakly_incrementable ErrorOutput>

requires std::indirectly_writable<ErrorOutput, YAML::Exception>

std::expected<Value, ErrorOutput>
expect_lookup(YAML::Node const & config,
              pi::lookup_table<Value, StringLike, N> const & name_lookup,
              ErrorOutput into_errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;

    if (not config.IsScalar()) {
        std::stringstream message;
        message << "expecting a string but got \"" << YAML::Dump(config) << "\"";
        into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                                   into_errors).out;
        return std::unexpected{ into_errors };
    }
    auto const search = name_lookup.find(config.as<std::string>());
    if (search != name_lookup.end()) {
        return search->first;
    }
    std::stringstream message;
    message << "expecting value to be one of the following: [";
    std::string sep;
    for (const auto & [_, name] : name_lookup) {
        message << sep << name;
        sep = ", ";
    }
    message << "]";
    into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                               into_errors).out;
    return std::unexpected{ into_errors };
}

/**
 * \brief Read a string value from config
 *
 * \tparam string_like      can be converted to a string
 * \tparam ErrorOutput     an allocator-aware container of yaml-exceptions
 *
 * \param config    YAML string input
 * \param errors    write any parsing errors to
 */
template<convertible_from<std::string> StringLike,
         std::weakly_incrementable ErrorOutput>
requires std::indirectly_writable<ErrorOutput, YAML::Exception>

std::expected<StringLike, ErrorOutput>
expect(YAML::Node const & config, ErrorOutput into_errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;

    if (config.IsScalar()) {
        return config.Scalar();
    }
    std::stringstream message;
    message << "expecting a string but got \"" << YAML::Dump(config) << "\"";
    into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                               into_errors).out;
    return std::unexpected{ into_errors };
}

/**
 * \brief Read an integer point Number from config
 *
 * \tparam Number           integer type
 * \tparam ErrorOutput     allocator-aware range of yaml-exceptions
 *
 * \param config    YAML integer input
 * \param errors    write any parsing errors to
 *
 * \note Reading a negative Number from `config` for an unsigned `Number` type
 *       will result in an error written to `errors`.
 */
template<std::integral Number, std::weakly_incrementable ErrorOutput>
requires std::indirectly_writable<ErrorOutput, YAML::Exception>

std::expected<Number, ErrorOutput>
expect(YAML::Node const & config, ErrorOutput into_errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;
    
    static std::regex const negative_pattern{ "^-" };
    static std::regex const integer_pattern{ "-?[0-9]+[ \t]*" };

    if (not config.IsScalar()) {
        std::stringstream message;
        message << "expecting a non-negative integer but got non-scalar \""
                << YAML::Dump(config) << "\"";
        into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                                   into_errors).out;
    }
    else if (std::is_unsigned_v<Number> and std::regex_search(config.Scalar(), negative_pattern)) {
        std::stringstream message;
        message << "expecting a non-negative integer but got \""
                << YAML::Dump(config) << "\"";
        into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                                   into_errors).out;
    }
    else if (not std::regex_match(config.Scalar(), integer_pattern)) {
        std::stringstream message;
        message << "expecting an integer but got \"" << YAML::Dump(config) << "\"";
        into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                                   into_errors).out;
    }
    else {
        return config.as<Number>();
    }
    return into_errors;
}

/**
 * \brief Read a floating point Number from config
 *
 * \tparam Number           floating-point type
 * \tparam ErrorOutput     allocator aware container of yaml-exceptions
 *
 * \param config    YAML floating point input
 * \param errors    write any parsing errors to
 */
template<std::floating_point Number, std::weakly_incrementable ErrorOutput>
requires std::indirectly_writable<ErrorOutput, YAML::Exception>

std::expected<Number, ErrorOutput>
expect(YAML::Node const & config, ErrorOutput into_errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;

    static std::regex const integer_pattern{ "-?[0-9]+\\.?" };
    static std::regex const decimal_pattern{ "-?\\.[0-9]+" };
    static std::regex const real_pattern{ "-?[0-9]+\\.[0-9]+" };

    if (not config.IsScalar()) {
        std::stringstream message;
        message << "expecting a number but got \"" << YAML::Dump(config) << "\"";

        into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                                   into_errors).out;
        return std::unexpected{ into_errors };
    }
    std::string const& scalar_value = config.Scalar();
    if (not std::regex_match(scalar_value, integer_pattern) and
        not std::regex_match(scalar_value, decimal_pattern) and
        not std::regex_match(scalar_value, real_pattern)) {

        std::stringstream message;
        message << "expecting a number but got \"" << YAML::Dump(config) << "\"";
        into_errors = ranges::copy(views::single(YAML::Exception{ config.Mark(), message.str() }),
                                   into_errors).out;
    }
    else {
        return config.as<Number>();
    }
    return std::unexpected{ into_errors };
}

template<typename T>
struct konbu_expecter{
};

template<typename Value, typename ErrorOutput>
concept konbu_expectable = requires(YAML::Node node, ErrorOutput into_errors) {
    { konbu_expecter<Value>::expect(node, into_errors) }
        -> std::same_as<std::expected<Value, ErrorOutput>>;
};

template<typename Value, typename ErrorOutput>
requires konbu_expectable<Value, ErrorOutput>

std::expected<Value, ErrorOutput>
expect(YAML::Node const & node, ErrorOutput into_errors)
{
    return konbu_expecter<Value>::expect(node, into_errors);
}

template<typename Value>
concept expectable = requires(YAML::Node node, std::vector<YAML::Exception> errors) {
    { expect<Value>(node, std::back_inserter(errors)) }
        -> std::same_as<std::expected<Value, std::vector<YAML::Exception>::iterator>>;
};

template<expectable Value> requires std::default_initializable<Value>
Value expect(YAML::Node const & node)
{
    namespace ranges = std::ranges;
    namespace views = std::views;
    using namespace expect_helpers;

    std::vector<YAML::Exception> errors;
    auto result = expect<Value>(node, std::back_inserter(errors));
    ranges::for_each(errors | views::transform(&YAML::Exception::what), &print_error);
    if (not result) { return Value{}; }
    return *result;
}
}