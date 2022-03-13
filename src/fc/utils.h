#pragma once

#include <type_traits>
#include <compare>

#include "concepts.h"

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

    inline constexpr auto make_caller(auto f) {
        return [f = std::move(f)](auto &&x) {
            if constexpr (apply_invocable<decltype(f), decltype(x)>) {
                return fc::apply(f, fwd(x));
            } else {
                return std::invoke(f, fwd(x));
            }
        };
    }

    template <typename T, bool is_ref, bool is_const>
    using infer_type_from_cv_ref_t = std::conditional_t<
        is_ref,
        std::conditional_t<is_const, std::add_lvalue_reference_t<std::add_const_t<T>>, std::add_lvalue_reference_t<T>>,
        std::conditional_t<is_const, std::add_const_t<T>, T>>;

    template <typename T>
    using infer_value_type_from_cvref_container_t =
        infer_type_from_cv_ref_t<typename std::remove_cvref_t<T>::value_type, std::is_lvalue_reference_v<T>,
                                 std::is_const_v<std::remove_reference_t<T>>>;

    using std::begin;
    using std::empty;
    using std::end;
    using std::size;

    template <typename C>
    constexpr auto call_end(C &&x) noexcept {
        return end(fwd(x));
    }

    template <typename C>
    constexpr auto call_begin(C &&x) noexcept {
        return begin(fwd(x));
    }

    template <typename C>
    constexpr auto call_size(C &&x) noexcept {
        return size(fwd(x));
    }

    template <typename C>
    constexpr auto call_empty(C &&x) noexcept {
        return empty(fwd(x));
    }

} // namespace fc
