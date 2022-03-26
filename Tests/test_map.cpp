#include <gtest/gtest.h>

#include <vector>
#include <fc/iter.h>
#include <fc/algos.h>

TEST(test_map, map_lvalue_lazy) {
    std::vector x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto iter_x = fc::iter(x);

    ASSERT_TRUE(iter_x.map([](auto x) { return x * 2; }, [](auto x) { return x + 3; })
                    .equal({3, 5, 7, 9, 11, 13, 15, 17, 19, 21}));
}

TEST(test_map, map_rvalue_lazy) {
    std::vector x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto y = fc::into_iter(std::move(x)).map([](auto x) { return x * 2; }, [](auto x) { return x + 3; });

    ASSERT_TRUE(y.equal({3, 5, 7, 9, 11, 13, 15, 17, 19, 21}));
}

TEST(test_map, map_eager) {
    auto f = [] { return std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; };
    {
        std::vector x = f();
        auto iter_x = fc::iter(x);
        auto y = iter_x.map<std::vector>([](auto x) { return x * 2; }, [](auto x) { return x + 3; });
        auto z = iter_x.map<std::vector<int>>([](auto x) { return x * 2; }, [](auto x) { return x + 3; });

        static_assert(std::is_same_v<decltype(z), decltype(y)>);
        static_assert(std::is_same_v<decltype(z), std::vector<int>>);

        ASSERT_TRUE(y == z);
        ASSERT_TRUE(fc::iter(y).equal({3, 5, 7, 9, 11, 13, 15, 17, 19, 21}));
    }

    {
        auto x = fc::into_iter(f()).map<std::vector>([](auto x) { return x * 2; }, [](auto x) { return x + 3; });
        auto y = fc::into_iter(f()).map<std::vector<int>>([](auto x) { return x * 2; }, [](auto x) { return x + 3; });
        static_assert(std::is_same_v<decltype(x), decltype(y)>);
        static_assert(std::is_same_v<decltype(x), std::vector<int>>);

        ASSERT_TRUE(y == x);
        ASSERT_TRUE(fc::iter(y).equal({3, 5, 7, 9, 11, 13, 15, 17, 19, 21}));
    }
}
