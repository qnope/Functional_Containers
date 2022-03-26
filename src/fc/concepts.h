#pragma once

#include <array>
#include <tuple>
#include <ranges>
#include <utility>
#include <concepts>
#include <type_traits>

#ifndef fwd
#define fwd(x) static_cast<decltype(x) &&>(x)
#endif

namespace fc {
    using std::begin;
    using std::end;

    template <typename T>
    concept iterable = requires(T &&a) {
        begin(fwd(a));
        end(fwd(a));
    };

    template <typename T>
    concept copy_assignable = std::copy_constructible<T> && std::is_copy_assignable_v<T>;

    template <typename T>
    concept move_assignable = std::move_constructible<T> && std::is_move_assignable_v<T>;

    template <typename T, typename U>
    concept assignable = std::constructible_from<T, U> && std::assignable_from<T &, U>;

    template <typename T>
    concept trivially_destructible = std::is_trivially_destructible_v<T>;

    template <typename T>
    concept trivially_copy_assignable = std::is_trivially_copy_constructible_v<T> &&
        std::is_trivially_copy_assignable_v<T> && trivially_destructible<T>;

    template <typename T>
    concept trivially_move_assignable = std::is_trivially_move_constructible_v<T> &&
        std::is_trivially_move_assignable_v<T> && trivially_destructible<T>;

    template <typename T>
    concept lvalue = std::is_lvalue_reference_v<T &&>;

    template <typename T>
    concept rvalue = std::is_rvalue_reference_v<T &&>;

    template <typename T>
    constexpr auto enabled_borrowed_range = std::ranges::enable_borrowed_range<std::remove_cvref_t<T>>;

    template <typename T>
    concept borrowed = lvalue<T> || enabled_borrowed_range<T>;

    template <typename T>
    struct is_applyable_object : std::false_type {};

    template <typename... Ts>
    struct is_applyable_object<std::pair<Ts...>> : std::true_type {};

    template <typename... Ts>
    struct is_applyable_object<std::tuple<Ts...>> : std::true_type {};

    template <typename... Ts>
    struct is_applyable_object<std::array<Ts...>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_applyable_object_v = is_applyable_object<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept applyable_object = is_applyable_object_v<T>;

} // namespace fc
