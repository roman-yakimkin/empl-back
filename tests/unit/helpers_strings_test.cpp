//
// Created by Roman Yakimkin on 29.05.2026.
//

#include <gtest/gtest.h>
#include "helpers/strings.h"

TEST(StringsHelperTest, Trim_RemoveSpaces) {
    EXPECT_EQ(helpers::trim("  test string   "), "test string");
    EXPECT_EQ(helpers::trim("\ttest string\n"), "test string");
}

TEST(StringsHelperTest, Trim_EmptyString) {
    EXPECT_EQ(helpers::trim(""), "");
    EXPECT_EQ(helpers::trim("   "), "");
}

