#pragma once

#include <type_traits>
#include <compare>

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

} // namespace fc
