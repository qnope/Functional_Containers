#include <gtest/gtest.h>

#include <array>
#include <fc/iter.h>

TEST(test_algos, test_iter) {
    std::vector a = {0, 1, 2, 3, 4};
    std::vector b = a;
    std::vector c = a;

    auto ref_a = fc::iter(a);
    auto steal_c = fc::into_iter(std::move(c));

    ASSERT_TRUE(c.empty());
    ASSERT_EQ(&a[0], &*ref_a.begin());

    ASSERT_TRUE(std::equal(a.begin(), a.end(), ref_a.begin()));
    ASSERT_TRUE(std::equal(a.begin(), a.end(), steal_c.begin()));

    auto view_a = fc::as_view(begin(a), end(a));

    ASSERT_EQ(&*view_a.begin(), a.data());
}

TEST(test_algos, test_generic_algos) {
    auto f = []() { return fc::into_iter(std::vector{1, 2, 3, 4, 5, 6}); };
    auto iterable = f();

    ASSERT_TRUE(iterable.none_of([](auto &x) { return x > 8; }));
    ASSERT_TRUE(!iterable.none_of([](auto &x) { return x == 1; }));

    ASSERT_TRUE(iterable.all_of([](auto &x) { return x >= 1; }));
    ASSERT_TRUE(!iterable.all_of([](auto &x) { return x > 10; }));

    ASSERT_TRUE(iterable.any_of([](auto &x) { return x == 4; }));
    ASSERT_TRUE(!iterable.any_of([](auto &x) { return x == 9; }));

    ASSERT_EQ(1, iterable.count(4));
    ASSERT_EQ(3, iterable.count_if([](auto &x) { return x > 3; }));

    {
        auto x = iterable.find(5);
        auto y = std::as_const(iterable).find(0);
        static_assert(std::is_same_v<decltype(x), fc::optional<int &>>);
        static_assert(std::is_same_v<decltype(y), fc::optional<const int &>>);
        static_assert(std::is_same_v<decltype(f().find(4)), fc::optional<int>>);
        ASSERT_TRUE(x);
        ASSERT_FALSE(y);
        ASSERT_EQ(&*x, &*(iterable.begin() + 4));
    }

    {
        auto x = iterable.find_it(5);
        auto y = iterable.find_it(0);
        ASSERT_EQ(x, iterable.begin() + 4);
        ASSERT_EQ(y, iterable.end());
    }

    {
        auto has_number = [](int x) { return [x](auto y) { return x == y; }; };
        auto x = iterable.find_if(has_number(5));
        auto y = std::as_const(iterable).find_if(has_number(0));
        static_assert(std::is_same_v<decltype(x), fc::optional<int &>>);
        static_assert(std::is_same_v<decltype(y), fc::optional<const int &>>);
        static_assert(std::is_same_v<decltype(f().find_if(has_number(4))), fc::optional<int>>);
        ASSERT_TRUE(x);
        ASSERT_FALSE(y);
        ASSERT_EQ(&*x, &*(iterable.begin() + 4));
    }

    {
        auto x = iterable.index_of(5);
        auto y = iterable.index_of(0);

        static_assert(std::is_same_v<decltype(x), fc::optional<std::size_t>>);
        ASSERT_TRUE(x);
        ASSERT_FALSE(y);
        ASSERT_EQ(*x, 4);
    }

    {
        auto x = iterable.index_if([](auto x) { return x == 5; });
        auto y = iterable.index_if([](auto x) { return x == 0; });

        static_assert(std::is_same_v<decltype(x), fc::optional<std::size_t>>);
        ASSERT_TRUE(x);
        ASSERT_FALSE(y);
        ASSERT_EQ(*x, 4);
    }

    {
        auto x = iterable.sum();
        ASSERT_EQ(x, 1 + 2 + 3 + 4 + 5 + 6);
        ASSERT_TRUE(iterable.contains(5));
        ASSERT_FALSE(iterable.contains(0));
        ASSERT_TRUE(iterable.equal(std::array{1, 2, 3, 4, 5, 6}));
        ASSERT_FALSE(iterable.equal(std::array{1, 2, 3, 4, 5, 7}));
    }

    {
        auto x = iterable.max_element();
        auto y = std::as_const(iterable).min_element();
        static_assert(std::is_same_v<decltype(x), fc::optional<int &>>);
        static_assert(std::is_same_v<decltype(y), fc::optional<const int &>>);
        auto z = iterable.minmax_element();
        ASSERT_TRUE(z && x && y);
        ASSERT_TRUE(y == 1);
        ASSERT_TRUE(x == 6);
        auto [m, M] = *z;
        ASSERT_TRUE(m == 1 && M == 6);
    }
}

TEST(test_algos, test_sort_algos) {
    auto f = [] { return std::vector{3, 1, 2, 9, 8, 6, 7, 10, 23, 10, 4}; };
    auto iterable_ = f();
    auto iterable = fc::iter(iterable_);

    {
        auto &x = iterable.sort();
        ASSERT_EQ(&x, &iterable);
        ASSERT_TRUE(x.is_sorted());
        ASSERT_TRUE(iterable.equal(std::array{1, 2, 3, 4, 6, 7, 8, 9, 10, 10, 23}));
        auto &y = iterable.sort().unique();

        ASSERT_TRUE(y.is_sorted());
        ASSERT_EQ(&y, &iterable);
        ASSERT_TRUE(iterable.equal(std::array{1, 2, 3, 4, 6, 7, 8, 9, 10, 23}));
    }

    {
        auto x = fc::into_iter(f()).sort().unique();
        ASSERT_TRUE(x.equal(std::array{1, 2, 3, 4, 6, 7, 8, 9, 10, 23}));
    }

    {
        std::vector x = {0, 1, 2, 3, 4, 5, 5, 5, 5, 6, 7, 8, 9};
        auto iter_x = fc::iter(x);

        auto lower_bound = iter_x.lower_bound(5);
        auto upper_bound = iter_x.upper_bound(5);
        auto equal_range = iter_x.equal_range(5);

        ASSERT_EQ(&*lower_bound, &x[5]);
        ASSERT_EQ(&*upper_bound, &x[9]);
        ASSERT_TRUE(equal_range.equal({5, 5, 5, 5}));
        ASSERT_EQ(&*equal_range.begin(), &x[5]);
    }
}

TEST(test_algos, test_set_algos) {
    const std::vector a = {1, 3, 5, 7, 9, 11};
    const std::vector b = {2, 4, 6, 8, 10, 12};
    const std::vector c = {1, 2, 3, 4, 5, 6};
    auto iter_a = fc::iter(a);
    auto iter_c = fc::iter(c);
    {
        auto c = iter_a.merge(b);
        ASSERT_TRUE(c.equal({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
    }

    {
        auto c = iter_a.merge(b);
        ASSERT_TRUE(c.includes(a));
        ASSERT_TRUE(c.includes(b));
    }

    {
        auto d = iter_c.set_intersection(b);
        ASSERT_TRUE(d.equal({2, 4, 6}));
    }

    {
        auto d = iter_c.set_difference(b);
        ASSERT_TRUE(d.equal({1, 3, 5}));
    }

    {
        auto d = iter_c.set_union(b);
        ASSERT_TRUE(d.equal({1, 2, 3, 4, 5, 6, 8, 10, 12}));
    }
}

TEST(test_algos, test_heap_algos) {
    std::vector<int> v{3, 1, 4, 1, 5, 9};
    auto iter = fc::iter(v);

    iter.make_heap();
    ASSERT_TRUE(iter.equal({9, 5, 4, 1, 1, 3}));
    iter.push_heap(6);
    ASSERT_TRUE(iter.equal({9, 5, 6, 1, 1, 3, 4}));

    auto x = iter.pop_heap();
    ASSERT_EQ(iter.size(), 6);
    ASSERT_TRUE(x);
    ASSERT_EQ(x, 9);

    ASSERT_TRUE(iter.push_heap(9).sort_heap().equal({1, 1, 3, 4, 5, 6, 9}));
}

TEST(test_algos, test_modifyers) {
    {
        std::vector<int> v(10);
        fc::iter(v).iota(0).reverse().replace(9, 42);
        ASSERT_TRUE(fc::iter(v).equal({42, 8, 7, 6, 5, 4, 3, 2, 1, 0}));
    }

    {
        auto v = fc::into_iter(std::vector<int>(10)).iota(0).reverse().replace(9, 42);
        ASSERT_TRUE(v.equal({42, 8, 7, 6, 5, 4, 3, 2, 1, 0}));
    }

    auto is_odd = [](auto x) { return x % 2 == 1; };
    auto generator = [i = 0]() mutable { return i++; };
    {
        auto v = std::vector<int>(10);
        fc::iter(v).generate(generator).replace_if(is_odd, 42);
        ASSERT_TRUE(fc::iter(v).equal({0, 42, 2, 42, 4, 42, 6, 42, 8, 42}));
    }

    {
        auto v = fc::into_iter(std::vector<int>(10)).generate(generator).replace_if(is_odd, 42);
        ASSERT_TRUE(v.equal({0, 42, 2, 42, 4, 42, 6, 42, 8, 42}));
    }

    {
        auto v = fc::into_iter(std::vector<int>(10)).fill(42);
        ASSERT_TRUE(v.equal({42, 42, 42, 42, 42, 42, 42, 42, 42, 42}));
    }
}

TEST(test_algos, test_partition) {
    auto is_odd = [](auto x) { return x % 2 == 1; };
    auto is_even = [](auto x) { return x % 2 == 0; };

    {
        std::vector<int> v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        auto iter_v = fc::iter(v);
        ASSERT_FALSE(iter_v.is_partitioned(is_even));

        auto [a, b] = iter_v.partition(is_even);
        ASSERT_TRUE(a.all_of(is_even));
        ASSERT_TRUE(b.all_of(is_odd));

        ASSERT_TRUE(iter_v.is_partitioned(is_even));

        auto [c, d] = iter_v.partition_point(is_even);
        ASSERT_TRUE(c.equal(a));
        ASSERT_TRUE(d.equal(b));
    }

    {
        auto [a, b] = fc::into_iter(std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}).stable_partition(is_even);
        ASSERT_TRUE(a.equal({0, 2, 4, 6, 8}));
        ASSERT_TRUE(b.equal({1, 3, 5, 7, 9}));
    }
}

TEST(test_algos, test_tuple_like) {
    {
        std::vector<std::pair<int, double>> x = {{0, 3.5}, {1, 4.5}};
        auto iter_x = fc::iter(x);
        auto f = [](auto &&...xs) { ((xs *= 2), ...); };
        iter_x.for_each(f);
        ASSERT_TRUE((x[0].first == 0 && x[0].second == 7.0));
        ASSERT_TRUE((x[1].first == 2 && x[1].second == 9.0));
    }
    {
        std::vector<std::pair<int, double>> x = {{0, 3.5}, {1, 4.5}};
        auto iter_x = fc::iter(x);
        auto f = [](auto &&x) {
            auto &[a, b] = x;
            a *= 2;
            b *= 2;
        };
        iter_x.for_each(f);
        ASSERT_TRUE((x[0].first == 0 && x[0].second == 7.0));
        ASSERT_TRUE((x[1].first == 2 && x[1].second == 9.0));
    }
}
