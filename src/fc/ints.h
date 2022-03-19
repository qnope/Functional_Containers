#pragma once

#include <limits>
#include "algos.h"

namespace fc {
    struct int_iterator {
        using reference = int;
        using value_type = int;
        using difference_type = std::ptrdiff_t;
        using pointer = std::add_pointer_t<int>;
        using iterator_category = std::input_iterator_tag;

        int i = 0;
        constexpr int_iterator &operator++() {
            ++i;
            return *this;
        }

        constexpr int operator*() const noexcept { return i; }

        constexpr bool operator==(const int_iterator &) const noexcept = default;
    };

    struct ints_t : generic_algos<ints_t> {
        using value_type = int;
        int start = 0;
        int finish = std::numeric_limits<int>::max();
        constexpr auto begin() const noexcept { return int_iterator{start}; }
        constexpr auto end() const noexcept { return int_iterator{finish}; }
    };

    constexpr auto ints() { return ints_t{}; }
    constexpr auto ints(int start) { return ints_t{.start = start}; }
    constexpr auto ints(int start, int end) { return ints_t{.start = start, .finish = end}; }
} // namespace fc
