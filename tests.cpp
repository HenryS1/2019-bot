#include <gtest/gtest.h>
#include "read_data.hpp"

using namespace std;

TEST(read_data, reads_expected_player) {
    
    read_data reader;

    game_state s = reader.read_state("test-data/test-state.json");

    ASSERT_EQ(s.currentRound, 50);
    ASSERT_EQ(s.maxRounds, 200);
    ASSERT_EQ(s.currentWormId, 1);
    ASSERT_EQ(s.consecutiveDoNothingCount, 10);
    
    my_player& me = s.myPlayer;
    ASSERT_EQ(me.id, 1);
    ASSERT_EQ(me.score, 100);
    ASSERT_EQ(me.health, 300);

    vector<my_worm>& worms = me.worms;
    ASSERT_EQ(worms.size(), 2);
    
    my_worm& first_worm = worms[0];
    ASSERT_EQ(first_worm.id, 1);
    ASSERT_EQ(first_worm.health, 100);

    ASSERT_EQ(first_worm.mapPosition, my_worm({ 24, 29 }));
    ASSERT_EQ(first_worm.weapon, weapon({ 1, 3 }));    
    

}

