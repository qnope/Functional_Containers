#include <gtest/gtest.h>
#include <utility>
#include <fc/optional.h>

#include <optional>
struct Trivial {};

struct Trivial2 : Trivial {};

struct NotTrivial {
    constexpr NotTrivial() noexcept {}
    constexpr NotTrivial(const NotTrivial &) noexcept {}
    constexpr NotTrivial(NotTrivial &&) noexcept {}
    constexpr ~NotTrivial() {}

    constexpr NotTrivial &operator=(NotTrivial &&) noexcept { return *this; }
    constexpr NotTrivial &operator=(const NotTrivial &) noexcept { return *this; }
};

struct CopyableAndMovable {};
struct OnlyMovable {
    constexpr OnlyMovable(const OnlyMovable &) = delete;
    constexpr OnlyMovable &operator=(const OnlyMovable &) = delete;

    constexpr OnlyMovable(OnlyMovable &&) noexcept = default;
    OnlyMovable &operator=(OnlyMovable &&) noexcept = default;
};

struct InPlaceConstructible {
    constexpr InPlaceConstructible(int a, double b) noexcept : a{a}, b{b} {};

    int a;
    double b;
};

struct InitListConstructible {
    constexpr InitListConstructible(std::initializer_list<int>, double c) noexcept : c{c} {};

    double c;
};

TEST(test_optional, test_deref) {
    fc::optional<int> a;
    static_assert(std::is_same_v<int &, decltype(*a)>);
    static_assert(std::is_same_v<const int &, decltype(*std::as_const(a))>);
    static_assert(std::is_same_v<int, decltype(*fc::optional<int>{})>);

    static_assert(std::is_same_v<int *, decltype(a.operator->())>);
    static_assert(std::is_same_v<const int *, decltype(std::as_const(a).operator->())>);
}

TEST(test_optional, test_constructor) {
    // 1
    static_assert(!std::is_trivially_constructible_v<fc::optional<Trivial>>);
    static_assert(!std::is_trivially_constructible_v<fc::optional<Trivial>, fc::nullopt_t>);

    {
        fc::optional<int> a{};
        fc::optional<int> b{fc::nullopt};
        ASSERT_FALSE(a);
        ASSERT_FALSE(b);
        ASSERT_FALSE(a.has_value());
        ASSERT_FALSE(b.has_value());
    }

    // 2
    static_assert(std::is_trivially_copy_constructible_v<fc::optional<Trivial>>);
    static_assert(!std::is_trivially_copy_constructible_v<fc::optional<NotTrivial>>);

    {
        fc::optional<int> a{5};
        fc::optional<int> b = a;
        ASSERT_TRUE(a && b);
        ASSERT_TRUE(a.has_value() && b.has_value());
        ASSERT_TRUE(*a == 5 && *b == 5);
    }

    // 3
    static_assert(std::is_trivially_move_constructible_v<fc::optional<Trivial>>);
    static_assert(!std::is_trivially_move_constructible_v<fc::optional<NotTrivial>>);

    {
        auto f = [] {
            fc::optional<int> a{5};
            return std::move(a); // prevent copy elision
        };
        fc::optional<int> a{f()};
        ASSERT_TRUE(*a == 5);
    }

    // 4
    static_assert(std::is_constructible_v<fc::optional<Trivial>, const fc::optional<Trivial2> &>);

    {
        fc::optional<unsigned int> a{8u};
        fc::optional<int> b{a};
        ASSERT_TRUE(*a == *b && *a == 8);
    }

    // 5
    static_assert(std::is_constructible_v<fc::optional<Trivial>, fc::optional<Trivial2> &&>);
    {
        auto f = [] {
            fc::optional<unsigned> a{5u};
            return std::move(a); // prevent copy elision
        };
        fc::optional<int> a{f()};
        ASSERT_TRUE(*a == 5);
    }

    // 6
    static_assert(std::is_constructible_v<fc::optional<InPlaceConstructible>, std::in_place_t, int, double>);
    {
        fc::optional<InPlaceConstructible> a{std::in_place, 5, 3.0};
        ASSERT_TRUE(a->a == 5 && a->b == 3.0);
    }

    // 7
    static_assert(std::is_constructible_v<fc::optional<InitListConstructible>, std::in_place_t,
                                          std::initializer_list<int>, double>);

    // 8
    static_assert(std::is_constructible_v<fc::optional<Trivial>, Trivial2>);
}

TEST(test_optional, test_assign) {
    // 1
    static_assert(std::is_assignable_v<fc::optional<Trivial> &, fc::nullopt_t>);

    {
        auto f = [] {
            fc::optional<int> a{5};
            a = fc::nullopt;
            return a;
        };
        ASSERT_FALSE(f());
    }
    // 2
    static_assert(std::is_trivially_assignable_v<fc::optional<Trivial> &, const fc::optional<Trivial> &>);
    static_assert(std::is_assignable_v<fc::optional<NotTrivial> &, const fc::optional<NotTrivial> &>);
    static_assert(!std::is_trivially_assignable_v<fc::optional<NotTrivial> &, const fc::optional<NotTrivial> &>);

    {
        auto f = [] {
            fc::optional<int> a{5};
            fc::optional<int> b{3};
            a = b;
            return a;
        };
        static_assert(f() == 3);
    }
    // 3
    static_assert(std::is_trivially_assignable_v<fc::optional<Trivial> &, fc::optional<Trivial> &&>);
    static_assert(std::is_assignable_v<fc::optional<NotTrivial> &, fc::optional<NotTrivial> &&>);
    static_assert(!std::is_trivially_assignable_v<fc::optional<NotTrivial> &, fc::optional<NotTrivial> &&>);

    {
        auto f = [] {
            fc::optional<int> a{5};
            fc::optional<int> b{3};
            a = std::move(b);
            return a;
        };
        static_assert(f() == 3);
    }

    // 4
    static_assert(std::is_assignable_v<fc::optional<Trivial> &, Trivial>);
    static_assert(std::is_assignable_v<fc::optional<Trivial> &, Trivial2>);
    {
        auto f = [] {
            fc::optional<int> a{5};
            unsigned b{3u};
            a = b;
            return a;
        };
        static_assert(f() == 3);

        auto g = [] {
            fc::optional<int> a{5};
            int b{3};
            a = b;
            return a;
        };
        static_assert(g() == 3);
    }
    // 5
    static_assert(std::is_assignable_v<fc::optional<Trivial> &, const fc::optional<Trivial2> &>);
    {
        auto f = [] {
            fc::optional<int> a{5};
            fc::optional<unsigned> b{3u};
            a = b;
            return a;
        };
        static_assert(f() == 3);
    }
    // 6
    static_assert(std::is_assignable_v<fc::optional<Trivial> &, fc::optional<Trivial2> &&>);
    {
        auto f = [] {
            fc::optional<int> a{5};
            fc::optional<unsigned> b{3u};
            a = std::move(b);
            return a;
        };
        static_assert(f() == 3);
    }
}

TEST(test_optional, test_value) {
    fc::optional<int> a{{}};
    fc::optional<int> b{};

    static_assert(std::is_same_v<int &, decltype(a.value())>);
    static_assert(std::is_same_v<const int &, decltype(std::as_const(a).value())>);
    static_assert(std::is_same_v<int, decltype(std::move(a).value())>);

    ASSERT_TRUE(a.value_or(5) == 0);
    ASSERT_TRUE(b.value_or(5) == 5);
    ASSERT_TRUE(a.value() == 0);

    ASSERT_TRUE(std::move(a).value_or(5) == 0);
    ASSERT_TRUE(std::move(b).value_or(5) == 5);
}

TEST(test_optional, test_modifiers) {
    {
        auto f = [] {
            fc::optional<int> x{5};
            x.reset();
            return x;
        };
        ASSERT_FALSE(f());
    }
    {
        auto f = [] {
            fc::optional<InPlaceConstructible> x{std::in_place, 8, 2.0};
            x.emplace(3, 5.);
            return x;
        };

        auto x = f();
        ASSERT_TRUE(x->a == 3 && x->b == 5.0);
    }
    {
        auto f = [] {
            fc::optional<InitListConstructible> x{std::in_place, {0, 1}, 3.0};
            x.emplace({0, 3, 4}, 2.0);
            return x;
        };

        ASSERT_TRUE(f()->c == 2.0);
    }

    {
        auto f = [] {
            fc::optional<int> a{5}, b{42};
            swap(a, b);
            return std::pair{a, b};
        };
        auto x = f();
        ASSERT_TRUE(*x.first == 42 && *x.second == 5);
    }

    {
        auto x = fc::make_optional(5);
        auto y = fc::make_optional<InPlaceConstructible>(5, 3.0);
        auto z = fc::make_optional<InitListConstructible>({3}, 5.0);

        auto w = fc::optional{5};
        static_assert(std::is_same_v<std::decay_t<decltype(w)>, fc::optional<int>>);

        ASSERT_TRUE(*x == 5);
        ASSERT_TRUE(y->a == 5 && y->b == 3.0);
        ASSERT_TRUE(z->c == 5.0);

        ASSERT_EQ(std::hash<fc::optional<int>>()(x), std::hash<int>()(5));
    }
}

TEST(test_optional, test_comparison) {
    auto x = fc::optional<int>{};
    auto y = fc::optional<int>{42};
    auto z = fc::optional<int>{5};
    auto y2 = fc::optional<int>{42};

    ASSERT_TRUE(x == fc::nullopt);
    ASSERT_TRUE(y > fc::nullopt);
    ASSERT_TRUE(z > fc::nullopt);
    ASSERT_TRUE(fc::nullopt == x);
    ASSERT_TRUE(fc::nullopt < y);
    ASSERT_TRUE(fc::nullopt < z);

    ASSERT_TRUE(x < y);
    ASSERT_TRUE(y > z);
    ASSERT_TRUE(z < y);
    ASSERT_TRUE(y2 == y);

    ASSERT_TRUE(x < 0);
    ASSERT_TRUE(y < 100);
    ASSERT_TRUE(y > 32);
    ASSERT_TRUE(z > 3);
    ASSERT_TRUE(z < 23);
    ASSERT_TRUE(y == 42);
    ASSERT_TRUE(z != 42);

    ASSERT_TRUE(0 > x);
    ASSERT_TRUE(100 > y);
    ASSERT_TRUE(42 == y);
    ASSERT_TRUE(42 != z);
}

TEST(test_optional_and_optional_ref, test_extensions) {
    struct Person {
        std::string name;
    };

    {
        fc::optional<std::string> x(std::in_place, 'a', 1000);
        std::string a;
        fc::optional<std::string &> y = a;
        static_assert(std::is_same_v<decltype(x.take()), std::string>);
        static_assert(std::is_same_v<decltype(std::move(x).take()), std::string>);

        static_assert(std::is_same_v<decltype(y.take()), std::string &>);
        static_assert(std::is_same_v<decltype(std::move(y).take()), std::string &>);

        auto *p = &(*x)[0];
        a = x.take();
        ASSERT_FALSE(x);
        ASSERT_EQ(p, &a[0]);
        ASSERT_EQ(&(*y)[0], p);

        auto &w = y.take();
        ASSERT_EQ(&w, &a);
        ASSERT_FALSE(y);
    }
    {
        fc::optional<std::string> x = "minor";
        fc::optional<std::string> y;

        auto toupper = [](auto s) {
            for (auto &c : s)
                c = std::toupper(c);
            return s;
        };

        auto tolower = [](auto s) {
            for (auto &c : s)
                c = std::tolower(c);
            return s;
        };
        auto X = x.map(toupper);
        auto Y = y.map(toupper);

        ASSERT_EQ(X, "MINOR");

        X.reset();
        X = std::as_const(x).map(toupper);

        ASSERT_EQ(X, "MINOR");

        ASSERT_EQ(X.map(tolower), x);
        ASSERT_EQ(X.map(tolower).map(toupper), X);
        ASSERT_EQ(std::move(X).map(tolower), x);
        ASSERT_TRUE(X->empty());

        ASSERT_FALSE(Y);

        Person me = {.name = "Antoine"};

        auto p_me = fc::make_optional(std::cref(me));
        fc::optional<Person> p;
        static_assert(std::is_same_v<fc::optional<std::string &>, decltype(p.map(&Person::name))>);
        static_assert(std::is_same_v<fc::optional<const std::string &>, decltype(std::as_const(p).map(&Person::name))>);
        static_assert(std::is_same_v<fc::optional<std::string>, decltype(std::move(p).map(&Person::name))>);

        auto &my_name = *p_me.map(&Person::name);
        ASSERT_EQ(&my_name, &me.name);
    }

    {
        auto me = fc::make_optional<Person>(Person{.name = "Antoine"});
        auto no_one = fc::make_optional<Person>(Person{.name = ""});
        auto dead = fc::optional<Person>{};
        auto f = [](const auto &p) -> fc::optional<const std::string &> {
            if (p.name.empty())
                return fc::nullopt;
            else
                return p.name;
        };

        const auto &ref_me = *me.and_then(f);
        auto ref_no_one = no_one.and_then(f);
        auto ref_dead = dead.and_then(f);

        ASSERT_EQ(&ref_me, &me->name);
        ASSERT_FALSE(ref_no_one);
        ASSERT_FALSE(ref_dead);
    }
}

TEST(test_optional_ref, test_deref) {
    fc::optional<int &> x;
    fc::optional<const int &> y;

    static_assert(std::is_same_v<int &, decltype(*x)>);
    static_assert(std::is_same_v<const int &, decltype(*std::as_const(x))>);

    static_assert(std::is_same_v<const int &, decltype(*y)>);
    static_assert(std::is_same_v<const int &, decltype(*std::as_const(y))>);
}

TEST(test_optional_ref, test_constructor) {
    // 1
    static_assert(std::is_constructible_v<fc::optional<int &>>);
    static_assert(std::is_constructible_v<fc::optional<int &>, fc::nullopt_t>);

    {
        fc::optional<int &> x;
        ASSERT_FALSE(x);
        ASSERT_FALSE(x.has_value());
    }

    // 2
    static_assert(std::is_copy_constructible_v<fc::optional<int &>>);
    {
        int x;
        fc::optional<int &> y = x;
        fc::optional<int &> z = y;
        ASSERT_EQ(&*y, &*z);
        ASSERT_EQ(&x, &*y);
        ASSERT_EQ(&x, &*z);
        ASSERT_TRUE(y && z);
    }

    // 3
    static_assert(std::is_move_constructible_v<fc::optional<int &>>);
    {
        int x;
        fc::optional<int &> y = x;
        fc::optional<int &> z = std::move(y);
        ASSERT_EQ(&x, &*z);
        ASSERT_TRUE(z);
    }

    // 4-5
    static_assert(std::is_constructible_v<fc::optional<Trivial &>, fc::optional<Trivial2 &>>);
    static_assert(!std::is_constructible_v<fc::optional<Trivial &>, fc::optional<NotTrivial &>>);
    {
        Trivial2 obj;
        fc::optional<Trivial2 &> x = obj;
        fc::optional<Trivial &> y = x;
        ASSERT_EQ(&*y, &*x);
        ASSERT_EQ(&*y, &obj);
        ASSERT_TRUE(x && y);
    }

    // 8
    static_assert(!std::is_constructible_v<fc::optional<Trivial &>, Trivial>);
    static_assert(std::is_constructible_v<fc::optional<const Trivial &>, Trivial &>);
    static_assert(std::is_constructible_v<fc::optional<Trivial &>, Trivial &>);
    static_assert(!std::is_constructible_v<fc::optional<Trivial &>, const Trivial &>);

    {
        Trivial2 x;
        fc::optional<Trivial &> y = x;
        ASSERT_EQ(&*y, &x);
        ASSERT_TRUE(y);
    }
}

TEST(test_optional_ref, test_value) {
    fc::optional<int &> x;
    fc::optional<const int &> y;
    static_assert(std::is_same_v<int &, decltype(x.value())>);
    static_assert(std::is_same_v<const int &, decltype(std::as_const(x).value())>);
    static_assert(std::is_same_v<const int &, decltype(y.value())>);
    static_assert(std::is_same_v<const int &, decltype(std::move(y).value())>);
    static_assert(std::is_same_v<const int &, decltype(std::as_const(y).value())>);

    int z = 0;
    x = z;
    y = z;

    ASSERT_EQ(z, x.value());

    *x = 5;
    ASSERT_EQ(y.value(), z);
    ASSERT_EQ(z, 5);

    y = fc::nullopt;
    ASSERT_FALSE(y);
    ASSERT_EQ(y.value_or(1), 1);
    ASSERT_EQ(x.value_or(3), 5);
}

TEST(test_optional_ref, test_modifiers) {
    int a = 5;
    int b = 8;
    fc::optional<int &> x = a;
    fc::optional<int &> y = b;
    swap(x, y);
    ASSERT_EQ(&*x == &b, &*y == &a);

    x.reset();
    y.reset();
    ASSERT_FALSE(x);
    ASSERT_FALSE(y);

    auto not_ref = fc::make_optional(a);
    auto ref_a = fc::make_optional(std::ref(a));
    auto cref_a = fc::make_optional(std::cref(a));

    a = 18;

    static_assert(std::is_same_v<fc::optional<int>, decltype(not_ref)>);
    static_assert(std::is_same_v<fc::optional<int &>, decltype(ref_a)>);
    static_assert(std::is_same_v<fc::optional<const int &>, decltype(cref_a)>);
    ASSERT_TRUE(ref_a == a);
    ASSERT_TRUE(cref_a == a);

    ASSERT_TRUE(*ref_a == 18 && *cref_a == 18);
    ASSERT_TRUE(*not_ref == 5);
}

TEST(test_optional_ref, test_assign) {
    // 1
    static_assert(std::is_assignable_v<fc::optional<int &> &, fc::nullopt_t>);

    {
        int a;
        fc::optional<int &> x = a;
        x = fc::nullopt;
        ASSERT_FALSE(x);
    }

    // 2
    static_assert(std::is_copy_assignable_v<fc::optional<int &>>);
    {
        int a;
        fc::optional<int &> x = a;
        fc::optional<int &> y;
        y = x;
        ASSERT_TRUE(x == y);
    }

    // 3
    static_assert(std::is_move_assignable_v<fc::optional<int &>>);
    {
        int a;
        fc::optional<int &> x = a;
        fc::optional<int &> y;
        y = std::move(x);
        ASSERT_TRUE(a == y);
    }

    // 4
    static_assert(std::is_assignable_v<fc::optional<int &> &, int &>);
    static_assert(!std::is_assignable_v<fc::optional<int &> &, double &>);
    {
        int a;
        fc::optional<int &> x;
        x = a;
        ASSERT_TRUE(x == a);
    }

    // 5 - 6
    static_assert(std::is_assignable_v<fc::optional<Trivial &> &, fc::optional<Trivial2 &>>);
    static_assert(!std::is_assignable_v<fc::optional<NotTrivial &> &, fc::optional<Trivial2 &>>);
    {
        Trivial2 a;
        fc::optional<Trivial &> x;
        x = a;
        ASSERT_TRUE(&a == &*x);
    }
}

TEST(test_optional_ref, test_comparison) {
    int a = 5;
    int b = 5;

    fc::optional<int &> x = a;
    fc::optional<int &> y = b;
    fc::optional<int &> z;
    fc::optional<int &> w = x;

    ASSERT_TRUE(x != fc::nullopt);
    ASSERT_TRUE(z == fc::nullopt);

    ASSERT_TRUE(x == a);
    ASSERT_TRUE(x == b);
    ASSERT_TRUE(z != a);
    ASSERT_TRUE(z != b);
    ASSERT_TRUE(w == a);

    ASSERT_TRUE(x == w);
    ASSERT_TRUE(y == w);
    ASSERT_TRUE(z != w);
}
