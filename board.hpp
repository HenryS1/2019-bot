#ifndef _BOARD_H_
#define _BOARD_H_

#include <stdint.h>
#include <vector>
#include "data.hpp"
#include <assert.h>

using namespace std;

struct layer {

    explicit layer(const vector<vector<cell>>& map, const string& type) {
        uint8_t row_index = 0;
        assert(map.size() == 33);
        for (auto row : map) {
            assert(row.size() == 33);
            uint64_t current_row = 0;
            uint64_t current_position_mask = 1;
            uint8_t col_index = 0;
            for (auto c : row) {
                assert(c.x == col_index);
                assert(c.y == row_index);
                if (c.type == type) {
                    current_row |= current_position_mask;
                }
                current_position_mask <<= 1;
            }
            rows[row_index] = current_row;
        }
    }

    uint64_t rows[33];

};

struct game_worm {

    game_worm() {}

    game_worm(uint8_t x, uint8_t y,
              uint16_t health) : x(x), y(y), health(health) {} 

    uint8_t x;
    uint8_t y;
    uint16_t health;

};

struct board {

    explicit board(const vector<vector<cell>>& map, 
                   const vector<my_worm>& mine,
                   const vector<worm>& yours) : 
        deep_space(map, "DEEP_SPACE"),
        air(map, "AIR"),
        dirt(map, "DIRT") {

        assert(mine.size() <= 3);
        assert(yours.size() <= 3);

        uint8_t index = 0;
        
        for (auto& w : mine) {
            damage = w.weapon.damage;
            range = w.weapon.range;
            digging_range = w.diggingRange;
            my_worms[index++] = game_worm(w.position.x, w.position.y, w.health);
        }

        index = 0;
        for (auto& w : yours) {
            opponent_worms[index++] = game_worm(w.position.x, w.position.y, w.health);
        }

    }

    uint8_t damage;
    uint8_t range;
    uint8_t digging_range;
    layer deep_space;
    layer air;
    layer dirt;
    game_worm my_worms[3] = {};
    game_worm opponent_worms[3] = {};

};

#endif
