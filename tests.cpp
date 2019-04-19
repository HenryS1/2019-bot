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
    ASSERT_EQ(worms.size(), 2ULL);
    
    my_worm& first_worm = worms[0];
    ASSERT_EQ(first_worm.id, 1);
    ASSERT_EQ(first_worm.health, 100);

    map_position& worm_pos = first_worm.position;
    ASSERT_EQ(worm_pos.x, 24);
    ASSERT_EQ(worm_pos.y, 29);
        
    weapon& wpn = first_worm.weapon;
    ASSERT_EQ(wpn.damage, 1);
    ASSERT_EQ(wpn.range, 3);

    ASSERT_EQ(first_worm.diggingRange, 1);
    ASSERT_EQ(first_worm.movementRange, 1);

    vector<opponent>& opponents = s.opponents;
    ASSERT_EQ(opponents.size(), 1ULL);

    opponent& my_op = opponents[0];
    ASSERT_EQ(my_op.id, 2);
    ASSERT_EQ(my_op.score, 91);

    vector<worm>& op_worms = my_op.worms;
    ASSERT_EQ(op_worms.size(), 2ULL);
    
    worm& op_first_worm = op_worms[0];
    ASSERT_EQ(op_first_worm.id, 10);
    ASSERT_EQ(op_first_worm.health, 82);

    map_position op_worm_pos = op_first_worm.position;
    ASSERT_EQ(op_worm_pos.x, 31);
    ASSERT_EQ(op_worm_pos.y, 16);
    
    ASSERT_EQ(op_first_worm.diggingRange, 4);
    ASSERT_EQ(op_first_worm.movementRange, 2);

    vector<vector<cell>>& map = s.map;
    ASSERT_EQ(map.size(), 1ULL);

    vector<cell>& row_one = map[0];
    ASSERT_EQ(row_one.size(), 3ULL);
    
    cell& first_cell = row_one[0];
    ASSERT_EQ(first_cell.x, 10);
    ASSERT_EQ(first_cell.y, 19);
    ASSERT_EQ(first_cell.type, "DEEP_SPACE");

    cell& second_cell = row_one[1];
    ASSERT_EQ(second_cell.x, 23);
    ASSERT_EQ(second_cell.y, 7);
    ASSERT_EQ(second_cell.type, "AIR");

    cell& third_cell = row_one[2];
    ASSERT_EQ(third_cell.x, 15);
    ASSERT_EQ(third_cell.y, 21);
    ASSERT_EQ(third_cell.type, "DIRT");

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
