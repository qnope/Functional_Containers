#include <gtest/gtest.h>

#include <array>
#include <fc/zip.h>
#include <fc/iter.h>
#include <fc/ints.h>

TEST(test_zip, test_zip) {
    std::vector a = {0, 5, 4, 6};
    const auto b = a;
    auto zip = fc::zip(a, b, std::vector{10, 65, 4, 1});

    static_assert(std::is_same_v<decltype(zip)::value_type, std::tuple<int &, const int &, int>>);

    auto begin = zip.begin();

    static_assert(std::is_same_v<decltype(*begin), std::tuple<int &, const int &, int>>);
    int result = 0;

    for (auto [x, y, z] : fc::zip(a, b, std::vector{10, 65, 4, 1})) {
        result += x * y * z;
    }

    ASSERT_EQ(result, 0 + 5 * 65 * 5 + 4 * 4 * 4 + 6 * 6 * 1);

    for (auto [x, y] : fc::zip(a, b)) {
        x *= y;
    }

    ASSERT_EQ(a[0], 0);
    ASSERT_EQ(a[1], 25);
    ASSERT_EQ(a[2], 16);
    ASSERT_EQ(a[3], 36);
}

TEST(test_zip, test_zip_not_same_size) {
    std::vector a = {0, 5, 4, 6};

    int result = 0;
    for (auto [x, y] : fc::zip(a, std::vector{10, 65, 4, 1, 25, 34})) {
        result += x * y;
    }

    ASSERT_EQ(result, 65 * 5 + 4 * 4 + 6);

    ASSERT_TRUE(fc::zip(a, std::vector{10, 65, 4, 1, 25, 34})
                    .equal(std::initializer_list<std::tuple<int, int>>{{0, 10}, {5, 65}, {4, 4}, {6, 1}}));
}

TEST(test_zip, test_not_movable_object) {
    struct MovableOnly {
        constexpr MovableOnly(int x) noexcept : a{x} {};
        constexpr MovableOnly(MovableOnly &&) noexcept = default;
        constexpr MovableOnly &operator=(MovableOnly &&) noexcept = default;

        int a;
        constexpr bool operator==(const MovableOnly &) const = default;
    };

    static_assert(!std::is_copy_assignable_v<MovableOnly>);
    static_assert(!std::is_copy_constructible_v<MovableOnly>);

    static_assert(std::is_move_assignable_v<MovableOnly>);
    static_assert(std::is_move_constructible_v<MovableOnly>);

    auto f = [] {
        std::vector<MovableOnly> x;
        x.emplace_back(0);
        x.emplace_back(1);
        x.emplace_back(2);
        return x;
    };

    auto g = [f] {
        std::vector<std::tuple<MovableOnly>> x;
        for (auto &&y : f()) {
            x.push_back(std::move(y));
        }
        return x;
    };

    ASSERT_TRUE(fc::zip(f()).equal(g()));
}

TEST(test_zip, test_zip_with) {
    using namespace std::string_literals;
    std::vector x = {"My"s, "Antoine"s};

    auto cat = [](auto &&...xs) { return (... + xs); };

    ASSERT_TRUE(fc::zip_with(cat, x, std::array{"NameIs"s, "Morrier"s}).equal({"MyNameIs", "AntoineMorrier"}));
    ASSERT_TRUE(fc::zip_with(cat, x, std::array{"NameIs"s, "Morrier"s}).sum() == "MyNameIsAntoineMorrier");
}

TEST(test_zip, test_enumerate) {
    using namespace std::string_literals;
    ASSERT_TRUE(fc::ints(0, 10).equal({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    std::vector x = {"Antoine"s, "Baptiste"s, "Michou"s};
    std::vector y = {"Carlitus"s, "Papitus"s, "Christianus"s, "Lulu"s};
    ASSERT_TRUE(fc::enumerate(x, y).equal(
        {std::tuple{0, "Antoine"s, "Carlitus"s}, {1, "Baptiste"s, "Papitus"s}, {2, "Michou"s, "Christianus"s}}));
}

TEST(test_zip, test_build_map) {
    using namespace std::string_literals;
    std::vector x = {"Antoine"s, "Baptiste"s, "Michou"s};
    std::vector y = {27, 28, 62};

    auto map = fc::zip_for_map(x, y).to<std::map>();

    ASSERT_EQ(map[x[0]], 27);
    ASSERT_EQ(map[x[1]], 28);
    ASSERT_EQ(map[x[2]], 62);
}
