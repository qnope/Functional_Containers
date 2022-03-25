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

    ASSERT_TRUE(fc::into_iter(std::move(x))
                    .map([](auto x) { return x * 2; }, [](auto x) { return x + 3; })
                    .equal({3, 5, 7, 9, 11, 13, 15, 17, 19, 21}));
}
