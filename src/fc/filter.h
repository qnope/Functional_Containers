#pragma once

#include "utils.h"
#include "optional.h"

namespace fc {
    template <typename F, typename It>
    struct filter_iterator {
        using iterator_category = std::input_iterator_tag;
        using pointer = typename std::iterator_traits<It>::pointer;
        using reference = typename std::iterator_traits<It>::reference;
        using value_type = typename std::iterator_traits<It>::value_type;
        using difference_type = typename std::iterator_traits<It>::difference_type;
        constexpr filter_iterator(fc::optional<F &> f, It it, It end) noexcept :
            m_f{f}, m_it{std::find_if(std::move(it), end, *m_f)}, m_end{std::move(end)} {}

        constexpr filter_iterator(It end) noexcept : m_it{end}, m_end{std::move(end)} {}

        constexpr filter_iterator &operator++() noexcept {
            m_it = std::find_if(std::next(m_it), m_end, *m_f);
            return *this;
        }

        constexpr decltype(auto) operator*() const { return *m_it; }
        constexpr bool operator==(const filter_iterator &b) const noexcept { return m_it == b.m_it; }

      private:
        fc::optional<F &> m_f;
        It m_it;
        It m_end;
    };

    template <typename F, typename Range>
    struct filter_range {
        using iterator = filter_iterator<F, decltype(call_begin(std::declval<Range>()))>;
        using value_type = typename iterator::value_type;

        constexpr filter_range(F f, Range &&range) noexcept : m_f{std::move(f)}, m_range{fwd(range)} {}

        constexpr iterator begin() const noexcept { return {m_f, call_begin(m_range), call_end(m_range)}; }
        constexpr iterator end() const noexcept { return {call_end(m_range)}; }

      private:
        [[no_unique_address]] mutable F m_f;
        remove_rvalue_reference_t<Range> m_range;
    };
} // namespace fc
