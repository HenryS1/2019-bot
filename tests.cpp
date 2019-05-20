#include <gtest/gtest.h>
#include "read_data.hpp"
#include "board.hpp"

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

TEST(layer, is_correctly_constructed_from_map) {
    vector<vector<cell>> map = {{ {0, 1, "AIR"}, {1, 0, "DIRT" }}, 
                                {{ 0, 0, "DEEP_SPACE" }, { 1, 1, "AIR" }}};

    layer<2> air(map, "AIR");
    
    ASSERT_EQ(air.rows[0], 0);
    ASSERT_EQ(air.rows[1], 3);

    layer<2> dirt(map, "DIRT");

    ASSERT_EQ(dirt.rows[0], 2);
    ASSERT_EQ(dirt.rows[1], 0);

    layer<2> deep_space(map, "DEEP_SPACE");

    ASSERT_EQ(deep_space.rows[0], 1);
    ASSERT_EQ(deep_space.rows[1], 0);

}

TEST(board, is_correctly_constructed_from_map) {

    vector<vector<cell>> map = {{ {0, 1, "AIR"}, {1, 0, "DIRT" }}, 
                                {{ 0, 0, "DEEP_SPACE" }, { 1, 1, "AIR" }}};


    vector<my_worm> mine = { { 0, 56, { 0, 1 }, 3, 5, { 9, 13 } }};

    vector<worm> yours = { { 3, 79, { 1, 1 }, 12, 6 } };

    board<2> b(map, mine, yours);
    
    ASSERT_EQ(b.damage, 9);
    ASSERT_EQ(b.range, 13);
    ASSERT_EQ(b.digging_range, 3);

    layer<2>& air = b.air;
    ASSERT_EQ(air.rows[0], 0);
    ASSERT_EQ(air.rows[1], 3);

    layer<2>& dirt = b.dirt;
    ASSERT_EQ(dirt.rows[0], 2);
    ASSERT_EQ(dirt.rows[1], 0);

    layer<2>& deep_space = b.deep_space;
    ASSERT_EQ(deep_space.rows[0], 1);
    ASSERT_EQ(deep_space.rows[1], 0);

    game_worm mw = b.my_worms[0];
    ASSERT_EQ(mw.p.x, 0);
    ASSERT_EQ(mw.p.y, 1);
    ASSERT_EQ(mw.health, 56);

    game_worm yw = b.opponent_worms[0];
    ASSERT_EQ(yw.p.x, 1);
    ASSERT_EQ(yw.p.y, 1);
    ASSERT_EQ(yw.health, 79);

}

TEST(board, when_all_squares_are_available_all_directions_are_move_candidates) {
    
    uint64_t air_rows[9] = { 511, 511, 511, 511, 511, 511, 511, 511, 511 };
    uint64_t dirt_rows[9] = {0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 6, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(1, 1, 5);

    ASSERT_EQ((int)b.move_candidates(b.my_worms[0], b.my_worms), 255);
}

TEST(board, when_all_surrounding_squares_are_dirt_all_directions_are_dig_candidates) {

    uint64_t air_rows[9] = {0, 0, 4, 0, 0, 0, 0, 0, 0};
    uint64_t dirt_rows[9] = { 511, 511, 507, 511, 511, 511, 511, 511, 511 };
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 6, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(2, 2, 5);

    ASSERT_EQ((int)b.dig_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), 255);

}

TEST(board, when_all_surrounding_squares_are_deep_space_no_directions_are_move_candidates) {
    uint64_t air_rows[9] = {0, 0, 4, 0, 0, 0, 0, 0, 0};
    uint64_t dirt_rows[9] = {0};
    uint64_t deep_space_rows[9] = { 511, 511, 507, 511, 511, 511, 511, 511, 511 };

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 6, digging_range = 1;
    
    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(2, 2, 5);
    ASSERT_EQ((int)b.move_candidates(b.my_worms[0], b.my_worms), 0);
}

TEST(board, can_shoot_a_worm_in_any_direction) {
    uint64_t air_rows[9] = {511, 511, 511, 511, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 6, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(2, 4, 5);

    b.opponent_worms[0] = game_worm(2, 2, 5);
    b.opponent_worms[1] = game_worm(2, 6, 5);
    b.opponent_worms[2] = game_worm(4, 4, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), N | E | S);

    b.opponent_worms[0] = game_worm(0, 4, 5);
    b.opponent_worms[1] = game_worm(0, 2, 5);
    b.opponent_worms[2] = game_worm(0, 6, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), W | NW | SW);

    b.opponent_worms[0] = game_worm(4, 2, 5);
    b.opponent_worms[1] = game_worm(4, 6, 5);
    b.opponent_worms[2] = game_worm(0, 0, 0);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NE | SE);
}

TEST(board, cant_shoot_a_worm_thats_out_of_range) {

    uint64_t air_rows[9] = {511, 511, 511, 511, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 1, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(4, 4, 5);

    b.opponent_worms[0] = game_worm(4, 1, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(4, 7, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(1, 4, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(7, 4, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(1, 1, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(1, 7, 5);
    
    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(7, 7, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);

    b.opponent_worms[0] = game_worm(7, 1, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);
}

TEST(board, shoots_when_target_might_move_into_firing_line) {
    
    uint64_t air_rows[9] = {511, 511, 511, 511, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 3, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(4, 4, 5);

    b.opponent_worms[0] = game_worm(3, 2, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), N | NW);

    b.opponent_worms[0] = game_worm(2, 3, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), W | NW);

    b.opponent_worms[0] = game_worm(2, 5, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), W | SW);

    b.opponent_worms[0] = game_worm(3, 6, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), SW | S);

    b.opponent_worms[0] = game_worm(5, 6, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), S | SE);

    b.opponent_worms[0] = game_worm(6, 5, 5);
    
    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), SE | E);

    b.opponent_worms[0] = game_worm(6, 3, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), E | NE);

    b.opponent_worms[0] = game_worm(5, 2, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NE | N);

}

TEST(board, should_include_enemies_that_can_move_into_range_in_shoot_candidates) {

    uint64_t air_rows[9] = {511, 511, 511, 511, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 2, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(4, 4, 5);

    b.opponent_worms[0] = game_worm(3, 1, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), N);

    b.opponent_worms[0] = game_worm(3, 2, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), N | NW);

    b.opponent_worms[0] = game_worm(2, 3, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), W | NW);

    b.opponent_worms[0] = game_worm(2, 5, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), W | SW);

    b.opponent_worms[0] = game_worm(3, 6, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), SW | S);

    b.opponent_worms[0] = game_worm(5, 6, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), S | SE);

    b.opponent_worms[0] = game_worm(6, 5, 5);
    
    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), SE | E);

    b.opponent_worms[0] = game_worm(6, 3, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), E | NE);

    b.opponent_worms[0] = game_worm(5, 2, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NE | N);
}

TEST(board, should_not_be_able_to_shoot_when_obstructed) {

    uint64_t air_rows[9] = {511, 511, 511, 503, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0, 0, 0, 8, 0, 0, 0, 0, 0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 2, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(3, 4, 5);

    b.opponent_worms[0] = game_worm(3, 1, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), NONE);
    
}

TEST(board, should_not_be_able_to_shoot_when_a_worm_cant_move_into_firing_line) {

    uint64_t air_rows[9] = {511, 511, 511, 503, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0, 0, 0, 8, 0, 0, 0, 0, 0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 2, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(3, 4, 5);

    b.opponent_worms[1] = game_worm(2, 3, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), W | NW);

}

TEST(board, should_be_able_to_shoot_if_dirt_might_get_dug_out_by_enemy) {

    uint64_t air_rows[9] = {511, 511, 511, 503, 511, 511, 511, 511, 511};
    uint64_t dirt_rows[9] = {0, 0, 0, 8, 0, 0, 0, 0, 0};
    uint64_t deep_space_rows[9] = {0};

    layer<9> air(air_rows);
    layer<9> dirt(dirt_rows);
    layer<9> deep_space(deep_space_rows);

    uint8_t damage = 4, range = 2, digging_range = 1;

    board<9> b(dirt, air, deep_space, damage, range, digging_range);

    b.my_worms[0] = game_worm(3, 4, 5);

    b.opponent_worms[0] = game_worm(3, 1, 5);

    b.opponent_worms[1] = game_worm(2, 3, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), N | NW | W);

    b.opponent_worms[1] = game_worm(2, 2, 5);

    ASSERT_EQ((int)b.shoot_candidates(b.my_worms[0], b.my_worms, b.opponent_worms), N | NW);

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
