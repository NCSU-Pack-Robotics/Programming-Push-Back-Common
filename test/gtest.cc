#include <gtest/gtest.h>

// Tests if the GTEST framework is working
// Copied from: https://google.github.io/googletest/quickstart-cmake.html
TEST(TestSuiteName, TestName) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}
