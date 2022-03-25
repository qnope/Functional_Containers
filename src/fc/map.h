#pragma once

#include "utils.h"
#include "optional.h"

namespace fc {
    template <typename F, typename It>
    struct map_iterator {
        constexpr map_iterator(fc::optional<F &> f, It it) noexcept : m_f{f}, m_it{std::move(it)} {}

        constexpr map_iterator &operator++() noexcept {
            ++m_it;
            return *this;
        }

        constexpr decltype(auto) operator*() const {
            if constexpr (apply_invocable<F, decltype(*std::declval<It>())>) {
                return fc::apply(*m_f, *m_it);
            } else {
                return std::invoke(*m_f, *m_it);
            }
        }

        constexpr bool operator==(const map_iterator &b) const noexcept { return m_it == b.m_it; }

      private:
        fc::optional<F &> m_f;
        It m_it;

      public:
        using reference = decltype(*std::declval<map_iterator &>());
        using value_type = reference;
        using difference_type = std::ptrdiff_t;
        using pointer = std::add_pointer_t<std::remove_reference_t<reference>>;
        using iterator_category = std::input_iterator_tag;
    };

    template <typename F, typename Range>
    struct map_range {
        using iterator = map_iterator<F, decltype(call_begin(std::declval<Range>()))>;
        using value_type = typename iterator::value_type;

        constexpr map_range(F f, Range &&range) noexcept : m_f{std::move(f)}, m_range{fwd(range)} {}

        constexpr iterator begin() const noexcept { return {m_f, call_begin(m_range)}; }
        constexpr iterator end() const noexcept { return {fc::nullopt, call_end(m_range)}; }

      private:
        [[no_unique_address]] mutable F m_f;
        remove_rvalue_reference_t<Range> m_range;
    };
} // namespace fc
