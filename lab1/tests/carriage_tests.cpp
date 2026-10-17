#include <gtest/gtest.h>

#include "post/Carriage.h"

using post::Carriage;

TEST(Carriage, StartsAtGivenPosition) {
    EXPECT_EQ(Carriage().position(), 0);
    EXPECT_EQ(Carriage(7).position(), 7);
}

TEST(Carriage, MoveToChangesPosition) {
    Carriage carriage;
    carriage.moveTo(-4);
    EXPECT_EQ(carriage.position(), -4);
}

TEST(Carriage, PrefixIncrementAndDecrementMoveAndReturnSelf) {
    Carriage carriage(3);
    EXPECT_EQ((++carriage).position(), 4);
    EXPECT_EQ((--carriage).position(), 3);
    EXPECT_EQ((--carriage).position(), 2);
}

TEST(Carriage, PostfixIncrementAndDecrementReturnPreviousPosition) {
    Carriage carriage(3);
    EXPECT_EQ((carriage++).position(), 3);
    EXPECT_EQ(carriage.position(), 4);
    EXPECT_EQ((carriage--).position(), 4);
    EXPECT_EQ(carriage.position(), 3);
}

TEST(Carriage, EqualityComparesPositions) {
    EXPECT_TRUE(Carriage(2) == Carriage(2));
    EXPECT_FALSE(Carriage(2) != Carriage(2));
    EXPECT_TRUE(Carriage(2) != Carriage(3));
}
