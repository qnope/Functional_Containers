#pragma once

#include <type_traits>
#include <compare>

#include "concepts.h"

#ifndef fwd
#define fwd(x) static_cast<decltype(x) &&>(x)
#endif

namespace fc {
    template <typename T>
    struct decay_ref {
        using type = std::decay_t<T>;
    };

    template <typename T>
    struct decay_ref<std::reference_wrapper<T>> {
        using type = T &;
    };

    template <typename T>
    using decay_ref_t = typename decay_ref<T>::type;

    template <typename T>
    struct is_reference_wrapper : std::false_type {};

    template <typename T>
    struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

    template <typename T>
    constexpr bool is_reference_wrapper_v = is_reference_wrapper<std::decay_t<T>>::value;

    template <typename T, typename Tag>
    struct crtp {
      protected:
        constexpr T &self() noexcept { return static_cast<T &>(*this); }
        constexpr const T &self() const noexcept { return static_cast<const T &>(*this); }
    };

    template <typename T>
    struct remove_rvalue_reference {
        using type = T;
    };

    template <typename T>
    struct remove_rvalue_reference<T &&> {
        using type = T;
    };

    template <typename T>
    using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

    constexpr auto compose() noexcept {
        return [](auto &&x) -> decltype(auto) { return fwd(x); };
    }

    template <typename F, typename... Fs>
    constexpr auto compose(F f, Fs... fs) noexcept {
        if constexpr (sizeof...(Fs) == 0) {
            return f;
        } else {
            return [=](auto &&...xs) -> decltype(auto) { //
                return std::invoke(compose(fs...), std::invoke(f, fwd(xs)...));
            };
        }
    }

    namespace detail {
        template <typename F, applyable_object Tuple, std::size_t... I>
        constexpr auto apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>)
            -> decltype(std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...)) {
            return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
        }
    } // namespace detail

    // a sfinae friendly apply
    template <typename F, applyable_object Tuple>
    constexpr auto apply(F &&f, Tuple &&t)
        -> decltype(detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                                       std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{})) {
        return detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                                  std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }

    template <typename F, typename Tuple>
    concept apply_invocable = applyable_object<Tuple> && requires(F &&f, Tuple &&tuple) {
        fc::apply(fwd(f), fwd(tuple));
    };

} // namespace fc
