#pragma once

#include "utils.h"
#include "algos.h"

namespace fc {

    template <iterable T>
    struct iter_impl<T> :
        generic_algos<iter_impl<T>>,
        sort_algos<iter_impl<T>>,
        set_algos<iter_impl<T>>,
        heap_algos<iter_impl<T>>,
        modifyers_algos<iter_impl<T>>,
        partition_algos<iter_impl<T>> {
        using underlying_range = iter_impl<std::remove_cvref_t<T>>;

        using iterator = decltype(call_begin(std::declval<std::add_lvalue_reference_t<T>>()));

        using value_type = typename std::remove_cvref_t<T>::value_type;

        iter_impl() noexcept requires(!std::is_lvalue_reference_v<T>) = default;
        iter_impl(T &&x) noexcept : m_range(fwd(x)) {}

        constexpr auto end() noexcept { return call_end(m_range); }
        constexpr auto begin() noexcept { return call_begin(m_range); }

        constexpr auto end() const noexcept { return call_end(m_range); }
        constexpr auto begin() const noexcept { return call_begin(m_range); }

        constexpr auto empty() const noexcept { return call_empty(m_range); }

        template <typename... Ts>
        constexpr void erase(Ts &&...ts) noexcept(noexcept(this->m_range.erase(fwd(ts)...))) {
            m_range.erase(fwd(ts)...);
        }

        template <typename... Ts>
        constexpr auto insert(Ts &&...ts) noexcept(noexcept(this->m_range.insert(fwd(ts)...))) {
            return m_range.insert(fwd(ts)...);
        }

        constexpr auto size() const requires(std::ranges::sized_range<std::remove_cvref_t<T>>) {
            return call_size(m_range);
        }

      private : remove_rvalue_reference_t<T> m_range;
    };

    template <std::forward_iterator Begin, typename End>
    struct iter_impl<Begin, End> :
        generic_algos<iter_impl<Begin, End>>,
        sort_algos<iter_impl<Begin, End>>,
        set_algos<iter_impl<Begin, End>>,
        heap_algos<iter_impl<Begin, End>>,
        modifyers_algos<iter_impl<Begin, End>>,
        partition_algos<iter_impl<Begin, End>> {
        using underlying_range = iter_impl<Begin, End>;

        constexpr iter_impl(Begin begin, End end) noexcept : m_begin{std::move(begin)}, m_end{std::move(end)} {}

        constexpr auto begin() const noexcept { return m_begin; }
        constexpr auto end() const noexcept { return m_end; }

      private:
        Begin m_begin;
        End m_end;
    };

    template <typename Map>
    struct map_iter_impl : map_algos<map_iter_impl<Map>> {
        using mapped_type = typename std::remove_cvref_t<Map>::mapped_type;

        map_iter_impl() noexcept requires(!std::is_lvalue_reference_v<Map>) = default;
        map_iter_impl(Map &&x) noexcept : m_map(fwd(x)) {}

        constexpr auto end() noexcept { return call_end(m_map); }

        constexpr auto begin() noexcept { return call_begin(m_map); }

        constexpr auto end() const noexcept { return call_end(m_map); }
        constexpr auto begin() const noexcept { return call_begin(m_map); }

        constexpr auto empty() const noexcept { return call_empty(m_map); }

        constexpr Map &underlying_map() noexcept { return m_map; }
        constexpr const Map &underlying_map() const noexcept { return m_map; }

      private:
        remove_rvalue_reference_t<Map> m_map;
    };

    template <typename T>
    constexpr auto iter(T &&range) noexcept requires(lvalue<T> &&iterable<T>) {
        return iter_impl<T>(range);
    }

    template <typename T>
    constexpr auto into_iter(T &&range) noexcept requires(rvalue<T> &&iterable<T>) {
        return iter_impl<T>(fwd(range));
    }

    template <typename Begin, typename End>
    constexpr auto as_view(Begin b, End e) noexcept {
        return iter_impl<Begin, End>{std::move(b), std::move(e)};
    }

    template <lvalue T>
    constexpr auto map_iter(T &&range) noexcept {
        return map_iter_impl<T>{range};
    }

    template <rvalue T>
    constexpr auto into_map_iter(T &&range) noexcept {
        return map_iter_impl<T>{fwd(range)};
    }

} // namespace fc

namespace std::ranges {
    template <typename T>
    inline constexpr bool enable_borrowed_range<fc::iter_impl<T>> =
        std::is_lvalue_reference_v<T> || enable_borrowed_range<std::remove_cvref_t<T>>;
}
