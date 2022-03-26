#pragma once

/**
 * @file algos.h
 */
#pragma once

#include <numeric>
#include <iterator>
#include <algorithm>
#include <functional>

#include "map.h"
#include "filter.h"

#ifdef __cpp_lib_constexpr_algorithms
#define FC_CONSTEXPR_ALGO constexpr
#else
#define FC_CONSTEXPR_ALGO
#endif

namespace fc {

    using std::begin;
    using std::empty;
    using std::end;
    using std::size;

    template <typename T>
    struct error_t;

    template <typename T>
    struct type_t {
        using type = T;
    };

#define FC_CAT_IMPL(x, ...) x##__VA_ARGS__
#define FC_CAT(...) FC_CAT_IMPL(__VA_ARGS__)

    ////// For normal array
#define MAKE_UNDERLYING_TYPE_TRAITS(name, value_type)                                                                  \
    template <typename T, bool b = borrowed<std::remove_cvref_t<T>>>                                                   \
    struct name;                                                                                                       \
                                                                                                                       \
    template <typename T>                                                                                              \
    using FC_CAT(name, _t) = typename name<T>::type;                                                                   \
                                                                                                                       \
    template <typename T, bool B>                                                                                      \
    struct name<T &, B> : type_t<std::add_lvalue_reference_t<typename T::value_type>> {};                              \
                                                                                                                       \
    template <typename T, bool B>                                                                                      \
    struct name<const T &, B> : type_t<std::add_lvalue_reference_t<std::add_const_t<typename T::value_type>>> {};      \
                                                                                                                       \
    template <typename T>                                                                                              \
    struct name<T &&, true> : type_t<std::add_lvalue_reference_t<typename std::remove_cvref_t<T>::value_type>> {};     \
                                                                                                                       \
    template <typename T>                                                                                              \
    struct name<T &&, false> : type_t<typename std::remove_cvref_t<T>::value_type> {};

    MAKE_UNDERLYING_TYPE_TRAITS(underlying_type_with_cv_ref, value_type)
    MAKE_UNDERLYING_TYPE_TRAITS(underlying_mapped_type_with_cv_ref, mapped_type)

#undef MAKE_UNDERLYING_TYPE_TRAITS

    template <typename T, bool b = borrowed<std::remove_cvref_t<T>>>
    struct underlying_range {
        using type = T;
    };

    template <typename T>
    struct underlying_range<T, true> {
        using type = typename T::underlying_range;
    };

    template <typename T>
    using underlying_range_t = typename underlying_range<T>::type;

    template <typename... T>
    struct iter_impl;

#define FC_MAKE_FUNCTIONS(foo, ...)                                                                                    \
  public:                                                                                                              \
    __VA_OPT__(template <)                                                                                             \
    __VA_ARGS__ __VA_OPT__(>) FC_CONSTEXPR_ALGO decltype(auto) foo(auto &&...xs) &noexcept(                            \
        noexcept(FC_CAT(foo, _static)(this->self(), fwd(xs)...))) {                                                    \
        return FC_CAT(foo, _static)(this->self(), fwd(xs)...);                                                         \
    }                                                                                                                  \
    __VA_OPT__(template <)                                                                                             \
    __VA_ARGS__ __VA_OPT__(>) FC_CONSTEXPR_ALGO decltype(auto) foo(auto &&...xs)                                       \
        const &noexcept(noexcept(FC_CAT(foo, _static)(this->self(), fwd(xs)...))) {                                    \
        return FC_CAT(foo, _static)(this->self(), fwd(xs)...);                                                         \
    }                                                                                                                  \
    __VA_OPT__(template <)                                                                                             \
    __VA_ARGS__ __VA_OPT__(>) FC_CONSTEXPR_ALGO decltype(auto) foo(auto &&...xs) &&noexcept(                           \
        noexcept(FC_CAT(foo, _static)(std::move(this->self()), fwd(xs)...))) {                                         \
        return FC_CAT(foo, _static)(std::move(this->self()), fwd(xs)...);                                              \
    }                                                                                                                  \
    __VA_OPT__(template <)                                                                                             \
    __VA_ARGS__ __VA_OPT__(>) FC_CONSTEXPR_ALGO decltype(auto) foo(auto &&...xs)                                       \
        const &&noexcept(noexcept(FC_CAT(foo, _static)(std::move(this->self()), fwd(xs)...))) {                        \
        return FC_CAT(foo, _static)(std::move(this->self()), fwd(xs)...);                                              \
    }

    template <typename Rng>
    struct generic_algos : crtp<Rng, generic_algos<Rng>> {
        FC_CONSTEXPR_ALGO auto all_of(auto &&...fs) const noexcept requires(sizeof...(fs) > 0) {
            return std::all_of(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

        FC_CONSTEXPR_ALGO auto any_of(auto &&...fs) const noexcept requires(sizeof...(fs) > 0) {
            return std::any_of(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

        FC_CONSTEXPR_ALGO auto none_of(auto &&...fs) const noexcept requires(sizeof...(fs) > 0) {
            return std::none_of(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

        FC_CONSTEXPR_ALGO auto count(const auto &v) const noexcept {
            return std::count(begin(this->self()), end(this->self()), v);
        }

        FC_CONSTEXPR_ALGO auto count_if(auto &&...fs) const noexcept requires(sizeof...(fs) > 0) {
            return std::count_if(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

      private : FC_CONSTEXPR_ALGO static auto find_it_static(auto &&that, const auto &v) {
            static_assert(borrowed<decltype(that)>);
            return std::find(begin(that), end(that), v);
        }

        FC_CONSTEXPR_ALGO static auto find_static(auto &&that, const auto &v) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto it = std::find(begin(that), end(that), v);
            if (it == end(that)) {
                return R{fc::nullopt};
            }
            return R{*it};
        }

        FC_CONSTEXPR_ALGO static auto find_if_it_static(auto &&that, auto &&...fs) requires(sizeof...(fs) > 0) {
            static_assert(borrowed<decltype(that)>);
            return std::find_if(begin(that), end(that), make_caller(compose(fwd(fs)...)));
        }

        FC_CONSTEXPR_ALGO static auto find_if_static(auto &&that, auto &&...fs) requires(sizeof...(fs) > 0) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto it = std::find_if(begin(that), end(that), make_caller(compose(fwd(fs)...)));
            if (it == end(that)) {
                return R{fc::nullopt};
            }
            return R{*it};
        }

        FC_MAKE_FUNCTIONS(find)
        FC_MAKE_FUNCTIONS(find_if)

        FC_MAKE_FUNCTIONS(find_it)
        FC_MAKE_FUNCTIONS(find_if_it)

        FC_CONSTEXPR_ALGO auto find_nullable(const auto &v) const {
            using R = std::remove_cvref_t<decltype(*begin(this->self()))>;
            static_assert(std::is_constructible_v<R>);
            auto it = std::find(begin(this->self()), end(this->self()), v);
            if (it == end(this->self())) {
                return R{};
            }
            return R{*it};
        }

        FC_CONSTEXPR_ALGO auto find_if_nullable(auto &&...fs) const requires(sizeof...(fs) > 0) {
            using R = std::remove_cvref_t<decltype(*begin(this->self()))>;
            static_assert(std::is_constructible_v<R>);
            auto it = std::find_if(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
            if (it == end(this->self())) {
                return R{};
            }
            return R{*it};
        }

        FC_CONSTEXPR_ALGO optional<std::size_t> index_of(const auto &v) const {
            if (auto it = find_it(v); it != end(this->self())) {
                return std::size_t(std::distance(begin(this->self()), it));
            }
            return {};
        }

        FC_CONSTEXPR_ALGO optional<std::size_t> index_if(auto &&...fs) const requires(sizeof...(fs) > 0) {
            if (auto it = find_if_it(fwd(fs)...); it != end(this->self())) {
                return std::size_t(std::distance(begin(this->self()), it));
            }
            return {};
        }

        FC_CONSTEXPR_ALGO auto accumulate(auto init, auto &&...op) const {
            return std::accumulate(begin(this->self()), end(this->self()), std::move(init), fwd(op)...);
        }

        constexpr auto sum() const { return accumulate(std::decay_t<decltype(*begin(this->self()))>{}); }

        FC_CONSTEXPR_ALGO auto computeMean() const {
            if (empty(this->self())) {
                return fc::optional<decltype(sum() / size(this->self()))>{};
            }
            return fc::make_optional(sum() / size(this->self()));
        }

        FC_CONSTEXPR_ALGO auto contains(const auto &v) const { return find_it(v) != end(this->self()); }

        FC_CONSTEXPR_ALGO auto contains_if(auto &&...fs) const { return find_if_it(fwd(fs)...) != end(this->self()); }

        FC_CONSTEXPR_ALGO auto equal(const iterable auto &c, auto &&...op) const {
            return std::equal(begin(this->self()), end(this->self()), begin(c), end(c), fwd(op)...);
        }

        template <typename T>
        FC_CONSTEXPR_ALGO auto equal(std::initializer_list<T> c, auto &&...op) const {
            return std::equal(begin(this->self()), end(this->self()), begin(c), end(c), fwd(op)...);
        }

        FC_CONSTEXPR_ALGO auto for_each(auto &&...fs) {
            return std::for_each(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

        FC_CONSTEXPR_ALGO auto for_each(auto &&...fs) const {
            return std::for_each(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

      private:
        FC_CONSTEXPR_ALGO static auto max_element_it_static(auto &&that, auto &&...fs) {
            static_assert(borrowed<decltype(that)>);
            return std::max_element(begin(that), end(that), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO static auto min_element_it_static(auto &&that, auto &&...fs) {
            static_assert(borrowed<decltype(that)>);
            return std::min_element(begin(that), end(that), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO static auto minmax_element_it_static(auto &&that, auto &&...fs) {
            static_assert(borrowed<decltype(that)>);
            return std::minmax_element(begin(that), end(that), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO static auto min_element_static(auto &&that, auto &&...fs) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto e = end(that);
            if (auto it = min_element_it_static(fwd(that), fwd(fs)...); it != e) {
                return R{*it};
            } else {
                return R{fc::nullopt};
            }
        }

        FC_CONSTEXPR_ALGO static auto max_element_static(auto &&that, auto &&...fs) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto e = end(that);
            if (auto it = max_element_it_static(fwd(that), fwd(fs)...); it != e) {
                return R{*it};
            } else {
                return R{fc::nullopt};
            }
        }

        FC_CONSTEXPR_ALGO static auto minmax_element_static(auto &&that, auto &&...fs) {
            using r = underlying_type_with_cv_ref_t<decltype(that) &&>;
            using R = optional<std::pair<r, r>>;
            auto e = end(that);
            if (auto it = minmax_element_it_static(fwd(that), fwd(fs)...); it.first != e) {
                return R{std::in_place, *it.first, *it.second};
            } else {
                return R{fc::nullopt};
            }
        }

        FC_MAKE_FUNCTIONS(min_element_it)
        FC_MAKE_FUNCTIONS(max_element_it)
        FC_MAKE_FUNCTIONS(minmax_element_it)
        FC_MAKE_FUNCTIONS(min_element)
        FC_MAKE_FUNCTIONS(max_element)
        FC_MAKE_FUNCTIONS(minmax_element)

      private:
        FC_CONSTEXPR_ALGO static auto map_static(auto &&that, auto &&...fs) {
            auto f = compose(std::move(fs)...);
            using F = decltype(f);
            struct result : generic_algos<result>, map_range<F, decltype(that)> {
                using map_range<F, decltype(that)>::map_range;
            };
            return result{std::move(f), fwd(that)};
        }

        FC_CONSTEXPR_ALGO static auto filter_static(auto &&that, auto &&...fs) {
            auto f = compose(std::move(fs)...);
            using F = decltype(f);
            struct result : generic_algos<result>, filter_range<F, decltype(that)> {
                using filter_range<F, decltype(that)>::filter_range;
            };
            return result{std::move(f), fwd(that)};
        }

        FC_MAKE_FUNCTIONS(map)
        FC_MAKE_FUNCTIONS(filter)
    };

    template <typename Rng>
    struct sort_algos : crtp<Rng, sort_algos<Rng>> {
        FC_CONSTEXPR_ALGO auto is_sorted(auto &&...fs) const {
            return std::is_sorted(begin(this->self()), end(this->self()), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO Rng &sort(auto &&...fs) & {
            std::sort(begin(this->self()), end(this->self()), fwd(fs)...);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng sort(auto &&...fs) && {
            std::sort(begin(this->self()), end(this->self()), fwd(fs)...);
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &stable_sort(auto &&...fs) & {
            std::stable_sort(begin(this->self()), end(this->self()), fwd(fs)...);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng stable_sort(auto &&...fs) && {
            std::stable_sort(begin(this->self()), end(this->self()), fwd(fs)...);
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &unique(auto &&...fs) & {
            auto it = std::unique(begin(this->self()), end(this->self()), fwd(fs)...);
            this->self().erase(it, end(this->self()));
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng unique(auto &&...fs) && {
            auto it = std::unique(begin(this->self()), end(this->self()), fwd(fs)...);
            this->self().erase(it, end(this->self()));
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &shuffle(auto &&g) & {
            std::shuffle(begin(this->self()), end(this->self()), fwd(g));
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng shuffle(auto &&g) && {
            std::shuffle(begin(this->self()), end(this->self()), fwd(g));
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO bool binary_search(const auto &value, auto &&...fs) const {
            return std::binary_search(begin(this->self()), end(this->self()), value, fwd(fs)...);
        }

      private:
        FC_CONSTEXPR_ALGO static auto nth_element_it_static(auto &&that, std::size_t nth, auto &&...fs) {
            static_assert(borrowed<decltype(that)>);
            return std::nth_element(begin(that), std::next(begin(that), nth), end(that), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO static auto nth_element_static(auto &&that, std::size_t nth, auto &&...fs) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto e = end(that);
            if (auto it = nth_element_it_static(fwd(that), nth, fwd(fs)...); it != e) {
                return R{*it};
            }
            return R{fc::nullopt};
        }

        FC_CONSTEXPR_ALGO static auto lower_bound_it_static(auto &&that, const auto &v, auto &&...fs) {
            static_assert(borrowed<decltype(that)>);
            return std::lower_bound(begin(that), end(that), v, fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO static auto upper_bound_it_static(auto &&that, const auto &v, auto &&...fs) {
            static_assert(borrowed<decltype(that)>);
            return std::upper_bound(begin(that), end(that), v, fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO static auto lower_bound_static(auto &&that, const auto &v, auto &&...fs) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto e = end(that);
            if (auto it = lower_bound_it_static(fwd(that), v, fwd(fs)...); it != e) {
                return R{*it};
            }
            return R{fc::nullopt};
        }

        FC_CONSTEXPR_ALGO static auto upper_bound_static(auto &&that, const auto &v, auto &&...fs) {
            using R = optional<underlying_type_with_cv_ref_t<decltype(that) &&>>;
            auto e = end(that);
            if (auto it = upper_bound_it_static(fwd(that), v, fwd(fs)...); it != e) {
                return R{*it};
            }
            return R{fc::nullopt};
        }

        FC_CONSTEXPR_ALGO static auto equal_range_static(auto &&that, const auto &v, auto &&...fs) {
            using T = decltype(that);
            auto [f, e] = std::equal_range(begin(that), end(that), v, fwd(fs)...);

            if constexpr (borrowed<T>) {
                using R = iter_impl<decltype(begin(that)), decltype(begin(that))>;
                return R{f, e};
            } else {
                auto group = std::vector(f, e);
                using R = iter_impl<decltype(group)>;
                return R{std::move(group)};
            }
        }

        FC_MAKE_FUNCTIONS(nth_element_it)
        FC_MAKE_FUNCTIONS(nth_element)
        FC_MAKE_FUNCTIONS(lower_bound_it)
        FC_MAKE_FUNCTIONS(upper_bound_it)
        FC_MAKE_FUNCTIONS(lower_bound)
        FC_MAKE_FUNCTIONS(upper_bound)
        FC_MAKE_FUNCTIONS(equal_range)
    };

    template <typename Rng>
    struct set_algos : crtp<Rng, set_algos<Rng>> {
        FC_CONSTEXPR_ALGO auto merge(const iterable auto &c, auto &&...fs) const {
            underlying_range_t<Rng> res;
            std::merge(begin(this->self()), end(this->self()), begin(c), end(c), std::inserter(res, begin(res)),
                       fwd(fs)...);
            return res;
        }

        FC_CONSTEXPR_ALGO bool includes(const iterable auto &c, auto &&...fs) const {
            return std::includes(begin(this->self()), end(this->self()), begin(c), end(c), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO auto set_difference(const iterable auto &c, auto &&...fs) const {
            underlying_range_t<Rng> res;
            std::set_difference(begin(this->self()), end(this->self()), begin(c), end(c),
                                std::inserter(res, begin(res)), fwd(fs)...);
            return res;
        }

        FC_CONSTEXPR_ALGO auto set_intersection(const iterable auto &c, auto &&...fs) const {
            underlying_range_t<Rng> res;
            std::set_intersection(begin(this->self()), end(this->self()), begin(c), end(c),
                                  std::inserter(res, begin(res)), fwd(fs)...);
            return res;
        }

        FC_CONSTEXPR_ALGO auto set_symmetric_difference(const iterable auto &c, auto &&...fs) const {
            underlying_range_t<Rng> res;
            std::set_symmetric_difference(begin(this->self()), end(this->self()), begin(c), end(c),
                                          std::inserter(res, begin(res)), fwd(fs)...);
            return res;
        }

        FC_CONSTEXPR_ALGO auto set_union(const iterable auto &c, auto &&...fs) const {
            underlying_range_t<Rng> res;
            std::set_union(begin(this->self()), end(this->self()), begin(c), end(c), std::inserter(res, begin(res)),
                           fwd(fs)...);
            return res;
        }
    };

    template <typename Rng>
    struct heap_algos : crtp<Rng, heap_algos<Rng>> {
        FC_CONSTEXPR_ALGO auto is_heap(auto &&...fs) const {
            return std::is_heap(begin(this->self()), end(this->self()), fwd(fs)...);
        }

        FC_CONSTEXPR_ALGO Rng &make_heap(auto &&...fs) & {
            std::make_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng make_heap(auto &&...fs) && {
            std::make_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &push_heap(auto &&new_element, auto &&...fs) & {
            *std::inserter(this->self(), end(this->self())) = fwd(new_element);
            std::push_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng push_heap(auto &&new_element, auto &&...fs) && {
            *std::inserter(this->self(), end(this->self())) = fwd(new_element);
            std::push_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO auto pop_heap(auto &&...fs) & {
            std::pop_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            optional<typename Rng::value_type> res;
            if (!empty(this->self())) {
                auto last_it = std::prev(end(this->self()));
                res = *last_it;
                this->self().erase(last_it);
            }
            return res;
        }

        FC_CONSTEXPR_ALGO Rng &sort_heap(auto &&...fs) & {
            std::sort_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng sort_heap(auto &&...fs) && {
            std::sort_heap(begin(this->self()), end(this->self()), fwd(fs)...);
            return std::move(this->self());
        }
    };

    template <typename Rng>
    struct modifyers_algos : crtp<Rng, modifyers_algos<Rng>> {
        FC_CONSTEXPR_ALGO Rng &fill(const auto &v) & {
            std::fill(begin(this->self()), end(this->self()), v);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng fill(const auto &v) && {
            std::fill(begin(this->self()), end(this->self()), v);
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &iota(auto &&v) & {
            std::iota(begin(this->self()), end(this->self()), fwd(v));
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng iota(auto &&v) && {
            std::iota(begin(this->self()), end(this->self()), fwd(v));
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &generate(auto &&f) & {
            std::generate(begin(this->self()), end(this->self()), fwd(f));
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng generate(auto &&f) && {
            std::generate(begin(this->self()), end(this->self()), fwd(f));
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &reverse() & {
            std::reverse(begin(this->self()), end(this->self()));
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng reverse() && {
            std::reverse(begin(this->self()), end(this->self()));
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &replace(const auto &old_value, const auto &new_value) & {
            std::replace(begin(this->self()), end(this->self()), old_value, new_value);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng replace(const auto &old_value, const auto &new_value) && {
            std::replace(begin(this->self()), end(this->self()), old_value, new_value);
            return std::move(this->self());
        }

        FC_CONSTEXPR_ALGO Rng &replace_if(auto &&f, const auto &new_value) & {
            std::replace_if(begin(this->self()), end(this->self()), fwd(f), new_value);
            return this->self();
        }

        FC_CONSTEXPR_ALGO Rng replace_if(auto &&f, const auto &new_value) && {
            std::replace_if(begin(this->self()), end(this->self()), fwd(f), new_value);
            return std::move(this->self());
        }
    };

    template <typename Rng>
    struct partition_algos : crtp<Rng, partition_algos<Rng>> {
        FC_CONSTEXPR_ALGO auto is_partitioned(auto &&...fs) const {
            return std::is_partitioned(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

      private:
        static FC_CONSTEXPR_ALGO auto partition_static(auto &&that, auto &&...fs) {
            using T = decltype(that);
            static_assert(!std::is_const_v<std::remove_reference_t<T>>, "Unable to partition a const container");
            auto it = std::partition(begin(that), end(that), make_caller(compose(fwd(fs)...)));

            if constexpr (borrowed<T>) {
                using R = iter_impl<decltype(begin(that)), decltype(begin(that))>;
                return std::array{R{begin(that), it}, R{it, end(that)}};
            } else {
                auto first_group = std::vector(begin(that), it);
                auto second_group = std::vector(it, end(that));

                using R = iter_impl<decltype(first_group)>;
                return std::array{R{std::move(first_group)}, R{std::move(second_group)}};
            }
        }

        static FC_CONSTEXPR_ALGO auto stable_partition_static(auto &&that, auto &&...fs) {
            using T = decltype(that);
            static_assert(!std::is_const_v<std::remove_reference_t<T>>, "Unable to partition a const container");
            auto it = std::stable_partition(begin(that), end(that), make_caller(compose(fwd(fs)...)));

            if constexpr (borrowed<T>) {
                using R = iter_impl<decltype(begin(that)), decltype(begin(that))>;
                return std::array{R{begin(that), it}, R{it, end(that)}};
            } else {
                auto first_group = std::vector(begin(that), it);
                auto second_group = std::vector(it, end(that));

                using R = iter_impl<decltype(first_group)>;
                return std::array{R{std::move(first_group)}, R{std::move(second_group)}};
            }
        }

        static FC_CONSTEXPR_ALGO auto partition_point_static(auto &&that, auto &&...fs) {
            using T = decltype(that);
            auto it = std::partition_point(begin(that), end(that), make_caller(compose(fwd(fs)...)));

            if constexpr (borrowed<T>) {
                using R = iter_impl<decltype(begin(that)), decltype(begin(that))>;
                return std::array{R{begin(that), it}, R{it, end(that)}};
            } else {
                auto first_group = std::vector(begin(that), it);
                auto second_group = std::vector(it, end(that));

                using R = iter_impl<decltype(first_group)>;
                return std::array{R{std::move(first_group)}, R{std::move(second_group)}};
            }
        }

        FC_MAKE_FUNCTIONS(partition)
        FC_MAKE_FUNCTIONS(partition_point)
        FC_MAKE_FUNCTIONS(stable_partition)
    };

    template <typename Rng>
    struct map_algos : crtp<Rng, map_algos<Rng>> {
        FC_CONSTEXPR_ALGO static auto find_it_static(auto &&that, const auto &k) {
            static_assert(borrowed<decltype(that)>);
            return that.underlying_map().find(k);
        }

        FC_CONSTEXPR_ALGO static auto find_static(auto &&that, const auto &k) {
            using R = optional<underlying_mapped_type_with_cv_ref_t<decltype(that) &&>>;
            auto it = find_it_static(that, k);
            if (it == end(that)) {
                return R{fc::nullopt};
            }
            return R{it->second};
        }

        FC_CONSTEXPR_ALGO static auto take_static(auto &&that, const auto &k) {
            using R = optional<typename std::remove_cvref_t<decltype(that)>::mapped_type>;
            auto it = find_it_static(that, k);
            if (it == end(that)) {
                return R{fc::nullopt};
            }
            R result{it->second};
            that.underlying_map().erase(it);
            return result;
        }

        FC_MAKE_FUNCTIONS(find_it)
        FC_MAKE_FUNCTIONS(find)
        FC_MAKE_FUNCTIONS(take)

        FC_CONSTEXPR_ALGO auto for_each(auto &&...fs) {
            return std::for_each(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }

        FC_CONSTEXPR_ALGO auto for_each(auto &&...fs) const {
            return std::for_each(begin(this->self()), end(this->self()), make_caller(compose(fwd(fs)...)));
        }
    };

} // namespace fc
