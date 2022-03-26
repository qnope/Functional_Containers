#pragma once

#include <tuple>
#include "ints.h"
#include "algos.h"

namespace fc {

    template <typename ValueType, typename F, typename... Args>
    struct zip_iterator {
        using reference = ValueType;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using pointer = std::add_pointer_t<value_type>;
        using iterator_category = std::input_iterator_tag;

        constexpr zip_iterator(optional<F &> f, Args... xs) noexcept : m_f{f}, m_its{std::move(xs)...} {}

        constexpr reference operator*() const noexcept { return apply(*m_f, m_its); }

        constexpr zip_iterator &operator++() noexcept {
            apply([](auto &...xs) { (++xs, ...); }, m_its);
            return *this;
        }

        constexpr bool operator==(const zip_iterator &b) const noexcept {
            return apply(
                [&](const auto &...xs) {
                    return apply(
                        [&](const auto &...ys) { //
                            return (... || (xs == ys));
                        },
                        b.m_its);
                },
                m_its);
        }

      private:
        mutable optional<F &> m_f;
        mutable std::tuple<Args...> m_its;
    };

    template <typename F, typename ValueType, typename... Xs>
    struct zip_range : generic_algos<zip_range<F, ValueType, Xs...>> {
        using value_type = ValueType;

        constexpr zip_range(F f, Xs &&...xs) noexcept : m_f{std::move(f)}, m_range{fwd(xs)...} {}

        constexpr auto begin() const noexcept {
            return apply(
                [this](auto &&...xs) {
                    return zip_iterator<value_type, F, decltype(call_begin(xs))...>{m_f, call_begin(xs)...};
                },
                m_range);
        }

        constexpr auto end() const noexcept {
            return apply(
                [](auto &&...xs) { //
                    return zip_iterator<value_type, F, decltype(call_end(xs))...>{fc::nullopt, call_end(xs)...};
                },
                m_range);
        }

      private:
        [[no_unique_address]] mutable F m_f;
        mutable std::tuple<remove_rvalue_reference_t<Xs>...> m_range;
    };

    template <typename F, iterable... Containers>
    constexpr auto zip_with(F f, Containers &&...xs) {
        using value_type = std::invoke_result_t<F, infer_value_type_from_cvref_container_t<Containers>...>;
        auto g = [f = std::move(f)](auto &&...its) mutable {
            return std::invoke(f, static_cast<infer_value_type_from_cvref_container_t<Containers> &&>(*its)...);
        };
        return zip_range<decltype(g), value_type, Containers...>{std::move(g), fwd(xs)...};
    }

    template <iterable... Containers>
    constexpr auto zip(Containers &&...xs) {
        if constexpr (sizeof...(Containers) == 2) {
            using value_type = std::pair<infer_value_type_from_cvref_container_t<Containers>...>;
            auto f = [](auto &&...xs) { return value_type(fwd(xs)...); };
            return zip_with(f, fwd(xs)...);
        } else {
            using value_type = std::tuple<infer_value_type_from_cvref_container_t<Containers>...>;
            auto f = [](auto &&...xs) { return value_type(fwd(xs)...); };
            return zip_with(f, fwd(xs)...);
        }
    }

    template <iterable... Containers>
    constexpr auto enumerate(Containers &&...xs) {
        return zip(ints(), fwd(xs)...);
    }
} // namespace fc
