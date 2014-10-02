#include <iostream>
#include "gtest/gtest.h"

TEST(HelloWorld, FirstTest) {
    int i = 0;
    std::cout << "test " << i;
    EXPECT_EQ(0, i);
}
