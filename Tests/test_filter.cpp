#include <gtest/gtest.h>

#include <vector>
#include <fc/iter.h>
#include <fc/algos.h>

TEST(test_filter, filter_lvalue_lazy) {
    std::vector x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto iter_x = fc::iter(x);

    auto is_odd = [](auto x) { return x % 2 == 1; };
    auto is_even = [](auto x) { return x % 2 == 0; };
    ASSERT_TRUE(iter_x.filter(is_odd).equal({1, 3, 5, 7, 9}));
    ASSERT_TRUE(iter_x.filter(is_even).equal({0, 2, 4, 6, 8}));
}

TEST(test_filter, filter_rvalue_lazy) {
    auto f = [] { return std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; };
    auto is_odd = [](auto x) { return x % 2 == 1; };
    auto is_even = [](auto x) { return x % 2 == 0; };
    auto x = fc::into_iter(f()).filter(is_odd);
    auto y = fc::into_iter(f()).filter(is_even);
    ASSERT_TRUE(x.equal({1, 3, 5, 7, 9}));
    ASSERT_TRUE(y.equal({0, 2, 4, 6, 8}));
}
