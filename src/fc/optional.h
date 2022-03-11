#pragma once

#include "utils.h"

#include <cassert>
#include <memory>
#include <utility>

namespace fc {
    constexpr std::in_place_t inplace{};

    struct nullopt_t {
        explicit constexpr nullopt_t(int) {}
    };
    constexpr nullopt_t nullopt{0};

    struct bad_optional_access : std::exception {
        const char *what() const noexcept override { return "bad optional access"; }
    };

    template <typename T>
    class optional;

    template <typename T>
    struct is_optional : std::false_type {};

    template <typename T>
    struct is_optional<optional<T>> : std::true_type {};

    template <typename T>
    constexpr bool is_optional_v = is_optional<std::decay_t<T>>::value;

    namespace details {
        template <typename T, typename U>
        concept convertible_or_constructible_from_optional =
            (std::is_constructible_v<T, optional<U> &> || std::is_constructible_v<T, const optional<U> &> ||
             std::is_constructible_v<T, optional<U> &&> || std::is_constructible_v<T, const optional<U> &&> ||
             std::is_convertible_v<optional<U> &, T> || std::is_convertible_v<const optional<U> &, T> ||
             std::is_convertible_v<optional<U> &&, T> || std::is_convertible_v<const optional<U> &&, T>);

        template <typename T, typename U>
        concept convertible_or_constructible_or_assignable_from_optional =
            (convertible_or_constructible_from_optional<T, U> ||
             (std::is_assignable_v<T &, optional<U> &> || std::is_assignable_v<T &, const optional<U> &> ||
              std::is_assignable_v<T &, optional<U> &&> || std::is_assignable_v<T &, const optional<U> &&>));
    } // namespace details

    template <typename T>
    struct value_functions : crtp<T, value_functions<T>> {
        template <typename U = T>
        constexpr auto
        value_or(U x) const &noexcept(std::is_nothrow_constructible_v<T, U> &&std::is_nothrow_copy_constructible_v<T>) {
            if (this->self())
                return *this->self();
            return x;
        }

        template <typename U = T>
        constexpr auto
        value_or(U x) &&noexcept(std::is_nothrow_constructible_v<T, U> &&std::is_nothrow_move_constructible_v<T>) {
            if (this->self())
                return *std::move(this->self());
            return x;
        }

        constexpr decltype(auto) value() & {
            if (this->self())
                return *this->self();
            throw bad_optional_access{};
        }

        constexpr decltype(auto) value() const & {
            if (this->self())
                return *this->self();
            throw bad_optional_access{};
        }

        constexpr decltype(auto) value() && {
            if (this->self())
                return *std::move(this->self());
            throw bad_optional_access{};
        }

        constexpr decltype(auto) value() const && {
            if (this->self())
                return *std::move(this->self());
            throw bad_optional_access{};
        }
    };

    // clang does not implement P0848...
    template <typename T, bool trivial_destructible = std::is_trivially_destructible_v<T>>
    struct optional_storage {
      protected:
        using StoredType = std::remove_const_t<T>;
        struct EmptyByte {};
        union {
            EmptyByte m_empty;
            StoredType m_payload;
        };
        bool m_has_value{false};

        constexpr optional_storage() noexcept {}

        template <typename... Args>
        constexpr optional_storage(Args &&...args) : m_payload(fwd(args)...), m_has_value{true} {}
    };

    template <typename T> // clang does not implement P0848...
    struct optional_storage<T, false> {
      protected:
        using StoredType = std::remove_const_t<T>;
        struct EmptyByte {};
        union {
            EmptyByte m_empty;
            StoredType m_payload;
        };

        bool m_has_value{false};

        constexpr optional_storage() noexcept {}

        template <typename... Args>
        constexpr optional_storage(Args &&...args) : m_payload(fwd(args)...), m_has_value{true} {}

      public:
        constexpr ~optional_storage() {
            if (m_has_value)
                m_payload.~StoredType();
        }
    };

    template <typename T, typename Opt>
    struct optional_extension : public crtp<Opt, optional_extension<T, Opt>> {
        constexpr T take() {
            assert(this->self());
            T x = *std::move(this->self());
            this->self().reset();
            return x;
        }

        template <typename F>
        constexpr optional<remove_rvalue_reference_t<std::invoke_result_t<F, T &>>> map(F &&f) & {
            if (this->self()) {
                return std::invoke(fwd(f), *this->self());
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr optional<remove_rvalue_reference_t<std::invoke_result_t<F, const T &>>> map(F &&f) const & {
            if (this->self()) {
                return std::invoke(fwd(f), *this->self());
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr optional<remove_rvalue_reference_t<std::invoke_result_t<F, T &&>>> map(F &&f) && {
            if (this->self()) {
                return std::invoke(fwd(f), *std::move(this->self()));
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr optional<remove_rvalue_reference_t<std::invoke_result_t<F, const T &&>>> map(F &&f) const && {
            if (this->self()) {
                return std::invoke(fwd(f), *std::move(this->self()));
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr std::invoke_result_t<F, T &> and_then(F &&f) & {
            if (this->self()) {
                return std::invoke(fwd(f), *this->self());
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr std::invoke_result_t<F, T &> and_then(F &&f) const & {
            if (this->self()) {
                return std::invoke(fwd(f), *this->self());
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr std::invoke_result_t<F, T &> and_then(F &&f) && {
            if (this->self()) {
                return std::invoke(fwd(f), *std::move(this->self()));
            }
            return fc::nullopt;
        }

        template <typename F>
        constexpr std::invoke_result_t<F, T &> and_then(F &&f) const && {
            if (this->self()) {
                return std::invoke(fwd(f), *std::move(this->self()));
            }
            return fc::nullopt;
        }
    };

    template <typename T>
    class optional :
        public optional_storage<T>,          //
        public value_functions<optional<T>>, //
        public optional_extension<T, optional<T>> {

        static_assert(!std::is_rvalue_reference_v<T>);
        static_assert(!std::is_same_v<nullopt_t, T>);
        static_assert(!std::is_same_v<std::in_place_t, T>);
        using StoredType = std::remove_const_t<T>;
        struct EmptyByte {};

      public:
        using value_type = T;

        // constructor 1
        constexpr optional() noexcept {}
        constexpr optional(nullopt_t) noexcept {}

        // constructor 2
        constexpr optional(const optional &)                  //
            noexcept(std::is_nothrow_copy_constructible_v<T>) //
            requires std::copy_constructible<T> && std::is_trivially_copy_constructible_v<T>
        = default;

        constexpr optional(const optional &rhs)               //
            noexcept(std::is_nothrow_copy_constructible_v<T>) //
            requires std::copy_constructible<T> {
            if (rhs.m_has_value)
                construct(rhs.m_payload);
        }

        // constructor 3
        constexpr optional(optional &&)                       //
            noexcept(std::is_nothrow_move_constructible_v<T>) //
            requires std::move_constructible<T> && std::is_trivially_move_constructible_v<T>
        = default;

        constexpr optional(optional &&rhs)                    //
            noexcept(std::is_nothrow_move_constructible_v<T>) //
            requires std::move_constructible<T> {
            if (rhs.m_has_value)
                construct(std::move(rhs.m_payload));
        }

        // constructor 4
        template <typename U>
        explicit(!std::is_convertible_v<const U &, T>) constexpr optional(const optional<U> &rhs) //
            noexcept(std::is_nothrow_constructible_v<T, const U &>)                               //
            requires(std::constructible_from<T, const U &> &&
                     !details::convertible_or_constructible_from_optional<T, U>) {
            if (rhs)
                construct(*rhs);
        }

        // constructor 5
        template <typename U>
        explicit(!std::is_convertible_v<U &&, T>) constexpr optional(optional<U> &&rhs) //
            noexcept(std::is_nothrow_constructible_v<T, U &&>)                          //
            requires(std::constructible_from<T, U &&> && !details::convertible_or_constructible_from_optional<T, U>) {
            if (rhs)
                construct(std::move(*rhs));
        }

        // constructor 6
        template <typename... Args>
        explicit constexpr optional(std::in_place_t,
                                    Args &&...args)                  //
            noexcept(std::is_nothrow_constructible_v<T, Args &&...>) //
            requires(std::constructible_from<T, Args &&...>) :
            optional_storage<T>(fwd(args)...) {}

        // constructor 7
        template <typename U, typename... Args>
        explicit constexpr optional(std::in_place_t, std::initializer_list<U> list, Args &&...args) //
            noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args &&...>)      //
            requires(std::constructible_from<T, std::initializer_list<U>, Args &&...>) :
            optional_storage<T>(list, fwd(args)...) {}

        // constructor 8
        template <typename U = T>
        explicit(!std::is_convertible_v<U &&, T>) constexpr optional(U &&value) //
            noexcept(std::is_nothrow_constructible_v<T, U &&>)                  //
            requires(std::constructible_from<T, U &&> && (!(std::is_same_v<std::decay_t<U>, std::in_place_t> ||
                                                            std::is_same_v<std::decay_t<U>, optional>))) :
            optional_storage<T>(fwd(value)) {}

        // assign 1
        constexpr optional &operator=(fc::nullopt_t) noexcept {
            reset();
            return *this;
        }

        // assign 2
        constexpr optional &operator=(const optional &other)                                         //
            noexcept(std::is_nothrow_copy_assignable_v<T> &&std::is_nothrow_copy_constructible_v<T>) //
            requires(copy_assignable<T> &&trivially_copy_assignable<T>) = default;

        constexpr optional &operator=(const optional &other) //
            noexcept(std::is_nothrow_copy_assignable_v<T>)   //
            requires(copy_assignable<T>) {
            if (this->m_has_value && other) {
                this->m_payload = *other;
            } else if (other) {
                construct(*other);
            } else {
                reset();
            }
            return *this;
        }

        // assign 3
        constexpr optional &operator=(optional &&other)                                              //
            noexcept(std::is_nothrow_move_assignable_v<T> &&std::is_nothrow_move_constructible_v<T>) //
            requires(move_assignable<T> &&trivially_move_assignable<T>) = default;

        constexpr optional &operator=(optional &&other)    //
            noexcept(std::is_nothrow_move_assignable_v<T>) //
            requires(move_assignable<T>) {
            if (this->m_has_value && other) {
                this->m_payload = std::move(*other);
            } else if (other) {
                construct(std::move(*other));
            } else {
                reset();
            }
            return *this;
        }

        // assign 4
        template <typename U = T>
        constexpr optional &operator=(U &&x)                                                             //
            noexcept(std::is_nothrow_assignable_v<T &, U &&> &&std::is_nothrow_constructible_v<T, U &&>) //
            requires(assignable<T, U> && !std::is_same_v<std::remove_cvref_t<U>, optional<T>> &&
                     (!std::is_scalar_v<T> || !std::same_as<std::decay_t<U>, T>)) {
            if (this->m_has_value) {
                this->m_payload = fwd(x);
            } else {
                construct(fwd(x));
            }
            return *this;
        }

        // assign 5
        template <typename U>
        constexpr optional &operator=(const optional<U> &other)                                                    //
            noexcept(std::is_nothrow_assignable_v<T &, const U &> &&std::is_nothrow_constructible_v<T, const U &>) //
            requires(assignable<T, const U &> &&
                     !details::convertible_or_constructible_or_assignable_from_optional<T, U>) {
            if (this->m_has_value && other) {
                this->m_payload = *other;
            } else if (other) {
                construct(*other);
            } else {
                reset();
            }
            return *this;
        }

        // assign 6
        template <typename U>
        constexpr optional &operator=(optional<U> &&other)                                               //
            noexcept(std::is_nothrow_assignable_v<T &, U &&> &&std::is_nothrow_constructible_v<T, U &&>) //
            requires(assignable<T, U &&> && !details::convertible_or_constructible_or_assignable_from_optional<T, U>) {
            if (this->m_has_value && other) {
                this->m_payload = std::move(*other);
            } else if (other) {
                construct(std::move(*other));
            } else {
                reset();
            }
            return *this;
        }

        constexpr T *operator->() noexcept {
            assert(this->m_has_value);
            return std::addressof(this->m_payload);
        }
        constexpr std::add_const_t<T> *operator->() const noexcept {
            assert(this->m_has_value);
            return std::addressof(this->m_payload);
        };

        constexpr T &operator*() &noexcept {
            assert(this->m_has_value);
            return this->m_payload;
        }
        constexpr const std::add_const_t<T> &operator*() const &noexcept {
            assert(this->m_has_value);
            return this->m_payload;
        }

        constexpr T operator*() &&noexcept(std::is_nothrow_move_constructible_v<T>) {
            assert(this->m_has_value);
            return std::move(this->m_payload);
        }

        constexpr T operator*() const &&noexcept(std::is_nothrow_move_constructible_v<T>) {
            assert(this->m_has_value);
            return std::move(this->m_payload);
        }

        constexpr bool has_value() const noexcept { return this->m_has_value; }
        constexpr explicit operator bool() const noexcept { return this->m_has_value; }

        constexpr void swap(optional &other)                                                   //
            noexcept(std::is_nothrow_move_constructible_v<T> &&std::is_nothrow_swappable_v<T>) //
            requires(std::move_constructible<T>) {
            using std::swap;
            if (this->m_has_value && other.m_has_value) {
                swap(this->m_payload, other.m_payload);
            } else if (this->m_has_value) {
                other.construct(std::move(this->m_payload));
                reset();
            } else if (other.m_has_value) {
                construct(std::move(other.m_payload));
                other.reset();
            }
        }

        constexpr void reset() noexcept {
            if (this->m_has_value) {
                this->m_payload.~StoredType();
                this->m_has_value = false;
            }
        }

        template <typename... Args>
        constexpr T &emplace(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args &&...>) {
            reset();
            construct(fwd(args)...);
            return this->m_payload;
        }

        template <class U, typename... Args>
        constexpr T &
        emplace(std::initializer_list<U> list,
                Args &&...args) noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args &&...>) {
            reset();
            construct(list, fwd(args)...);
            return this->m_payload;
        }

      private:
        template <typename... Args>
        constexpr void construct(Args &&...args) noexcept(std::is_nothrow_constructible_v<StoredType, Args &&...>) {
            std::construct_at<StoredType>(std::addressof(this->m_payload), fwd(args)...);
            this->m_has_value = true;
        }
    };

    template <typename T>
    class optional<T &> :
        public value_functions<optional<T &>>, //
        public optional_extension<T &, optional<T &>> {
        template <typename U>
        friend class optional;

      public:
        using value_type = T &;

        // 1
        constexpr optional() noexcept = default;
        constexpr optional(nullopt_t) noexcept {}

        constexpr optional(const optional &) = default; // 2
        constexpr optional(optional &&) = default;      // 3

        template <typename U>
        constexpr optional(optional<U &> x) noexcept // 4 and 5
            requires(std::convertible_to<U &, T &>) {
            m_ptr = x.m_ptr;
        }

        template <typename U>
        constexpr optional(U &&x) noexcept // 8
            requires(lvalue<U> &&std::convertible_to<U, T &>) {
            m_ptr = std::addressof(x);
        }

        // 1
        constexpr optional &operator=(nullopt_t) noexcept {
            m_ptr = nullptr;
            return *this;
        }
        constexpr optional &operator=(const optional &) noexcept = default; // 2
        constexpr optional &operator=(optional &&) noexcept = default;      // 3

        template <typename U>
        constexpr optional &operator=(U &&x) noexcept // 4
            requires(lvalue<U> &&std::convertible_to<U, T &>) {
            m_ptr = std::addressof(x);
            return *this;
        }

        template <typename U>
        constexpr optional &operator=(optional<U &> x) noexcept // 5 and 6
            requires(std::convertible_to<U &, T &>) {
            m_ptr = x.m_ptr;
            return *this;
        }

        constexpr T *operator->() noexcept {
            assert(m_ptr);
            return m_ptr;
        }
        constexpr std::add_const_t<T> *operator->() const noexcept {
            assert(m_ptr);
            return m_ptr;
        };

        constexpr T &operator*() &noexcept {
            assert(m_ptr);
            return *m_ptr;
        }

        constexpr T &operator*() &&noexcept {
            assert(m_ptr);
            return *m_ptr;
        }

        constexpr std::add_const_t<T> &operator*() const &noexcept {
            assert(m_ptr);
            return *m_ptr;
        }

        constexpr std::add_const_t<T> &operator*() const &&noexcept {
            assert(m_ptr);
            return *m_ptr;
        }

        constexpr void reset() noexcept { m_ptr = nullptr; }
        constexpr bool has_value() const noexcept { return m_ptr; }
        constexpr explicit operator bool() const noexcept { return m_ptr; }

        constexpr void swap(optional &x) noexcept {
            using std::swap;
            swap(m_ptr, x.m_ptr);
        }

      private:
        T *m_ptr = nullptr;
    };

    template <typename T>
    constexpr bool operator==(const optional<T> &x, fc::nullopt_t) noexcept {
        return !x;
    }

    template <typename T>
    constexpr std::strong_ordering operator<=>(const optional<T> &x, fc::nullopt_t) noexcept {
        return x ? std::strong_ordering::greater : std::strong_ordering::equal;
    }

    template <typename T, typename U>
    constexpr bool operator==(const optional<T> &x, const U &y) noexcept {
        return x ? *x == y : false;
    }

    template <typename T, typename U>
    constexpr std::strong_ordering operator<=>(const optional<T> &x, const U &y) noexcept {
        return x ? std::compare_strong_order_fallback(*x, y) : std::strong_ordering::less;
    }

    template <typename T, typename U>
    constexpr bool operator==(const optional<T> &x, const optional<U> &y) noexcept {
        return (x && y) ? *x == *y : false;
    }

    template <typename T, typename U>
    constexpr std::strong_ordering operator<=>(const optional<T> &x, const optional<U> &y) noexcept {
        return (x && y) ? std::compare_strong_order_fallback(*x, *y) : x.has_value() <=> y.has_value();
    }

    template <typename T>
    constexpr void swap(optional<T> &a, optional<T> &b) //
        noexcept(noexcept(a.swap(b)))                   //
        requires(std::move_constructible<T> &&std::swappable<T>) {
        a.swap(b);
    }

    template <typename T>
    constexpr optional<std::decay_t<T>> make_optional(T &&x) requires(!is_reference_wrapper_v<T>) {
        return optional<std::decay_t<T>>(fwd(x));
    }

    template <typename T>
    constexpr optional<typename T::type &> make_optional(T x) requires(is_reference_wrapper_v<T>) {
        return optional<typename T::type &>(x.get());
    }

    template <typename T, typename... Args>
    constexpr optional<T> make_optional(Args &&...args) {
        return optional<T>(std::in_place, fwd(args)...);
    }

    template <typename T, typename U, typename... Args>
    constexpr optional<T> make_optional(std::initializer_list<U> list, Args &&...args) {
        return optional<T>(std::in_place, list, fwd(args)...);
    }

    template <typename T>
    optional(T) -> optional<decay_ref_t<T>>;
} // namespace fc

namespace std {
    template <typename T>
    struct hash<fc::optional<T>> {
        constexpr std::size_t operator()(const fc::optional<T> &x) const noexcept {
            if (x) {
                return std::hash<std::remove_cvref_t<T>>()(*x);
            } else {
                return std::hash<std::nullptr_t>()(nullptr);
            }
        }
    };
} // namespace std
