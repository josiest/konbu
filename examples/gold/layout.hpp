#pragma once
#include <expected>
#include <algorithm>
#include <ranges>

#include "pi/konbu/read.hpp"
#include "justification.hpp"

inline namespace gold {
struct layout {
    justification::horizontal horizontal = justification::horizontal::left;
    justification::vertical vertical = justification::vertical::top;
};
}

inline namespace pi {
namespace konbu {

template<std::ranges::output_range<YAML::Exception> ErrorOutput>
bool read(YAML::Node const & config, gold::layout & layout, ErrorOutput & errors)
{
    namespace ranges = std::ranges;
    namespace views = std::views;
    // Won't be able to parse any data if the layout config isn't a map,
    // so we'll need to short-circuit if it isn't
    if (not config.IsMap()) {
        YAML::Exception const error{ config.Mark(),
                                     "expecting \"layout\" settings to be a map\n" };
        ranges::copy(views::single(error), back_inserter_preference(errors));
        return false;
    }
    // read the errors first into an isolated list, so that we can
    // re-contextualize them before copying them into the main error list
    std::vector<YAML::Exception> uncontextualized;
    bool success = true;
    if (auto const horizontal_config = config["horizontal"]) {
        success = read(horizontal_config, layout.horizontal, uncontextualized);
    }
    if (auto const vertical_config = config["vertical"]) {
        success = success and read(vertical_config, layout.vertical, uncontextualized);
    }
    // let the reader know that the error happened when parsing layout settings
    auto contextualize = [](YAML::Exception const & no_context) {
        std::stringstream message;
        message << "couldn't read layout settings\n  " << no_context.msg;
        return YAML::Exception{ no_context.mark, message.str() };
    };
    ranges::copy(uncontextualized | views::transform(contextualize),
                 back_inserter_preference(errors));
    return success;
}
}
}