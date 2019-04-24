#ifndef _BOARD_H_
#define _BOARD_H_

#include "data.hpp"
#include <stdint.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <math.h>

using namespace std;

template <uint8_t WIDTH>
struct layer {

    layer(const vector<vector<cell>>& map, const string& type) {
        assert(map.size() == WIDTH);
        for (auto row : map) {
            assert(row.size() == WIDTH);
            for (auto c : row) {
                uint8_t row_index = c.y;
                uint64_t current_position_mask = 1ULL << c.x;
                if (c.type == type) {
                    rows[row_index] |= current_position_mask;
                }
                current_position_mask <<= 1;
            }
        }
    }

    explicit layer(uint64_t* src_rows) {
        memcpy(rows, src_rows, WIDTH * sizeof(uint64_t));
    }

    uint64_t rows[WIDTH] = {0};

};

enum direction : uint8_t {
    NONE = 0,
    N = 1,
    NE = 2,
    E = 4,
    SE = 8,
    S = 16,
    SW = 32,
    W = 64,
    NW = 128
};

enum action : uint8_t {
    NOTHING = 0,
    MOVE = 1,
    DIG = 2,
    SHOOT = 4
};

struct selected_action {
    uint8_t del_x = 0;
    uint8_t del_y = 0;
    action a = NOTHING;
};

struct game_worm {

    game_worm() {}

    game_worm(uint8_t x, uint8_t y, uint16_t health) : x(x), y(y), health(health) {} 

    bool is_alive() { return health > 0; }

    uint8_t x;
    uint8_t y;
    uint16_t health;
    selected_action action;

};

template<uint8_t WIDTH>
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

    uint8_t dig_candidates(game_worm w) {
        uint8_t result = 0;
        uint64_t up_one_row = w.y > 0 ? dirt.rows[w.y - 1] : 0;
        result |= (1ULL << w.x) & up_one_row;
        result |= w.x > 0 ? (1ULL << (w.x - 1)) & up_one_row : 0;
        result |= w.x < WIDTH ? (1ULL << (w.x + 1)) & up_one_row : 0;
        
        uint64_t row = dirt.rows[w.y];
        result |= w.x > 0 ? (1ULL << (w.x - 1)) & row : 0;
        result |= w.x < WIDTH ? (1ULL << (w.x + 1)) & row : 0;

        uint64_t down_one_row = w.y < WIDTH ? dirt.rows[w.y + 1] : 0;
        result |= (1ULL << w.x) & down_one_row;
        result |= w.x > 0 ? (1ULL << (w.x - 1)) & down_one_row : 0;
        result |= w.x < WIDTH ? (1ULL << (w.x + 1)) & down_one_row : 0;

        return result;
    }

    uint8_t move_candidates(game_worm w) {
        uint8_t result = 0;
        if (w.y > 0) {
            uint64_t up_one_row = air.rows[w.y - 1];
            result |= !friendly_worm_will_be_at_position(w.x, w.y - 1) 
                ? ((1ULL << w.x) & up_one_row) : 0;
            result |= w.x > 0 && !friendly_worm_will_be_at_position(w.x - 1, w.y - 1) 
                ? (1ULL << (w.x - 1)) & up_one_row : 0;
            result |= w.x < WIDTH && !friendly_worm_will_be_at_position(w.x + 1, w.y - 1) 
                            ? (1ULL << (w.x + 1)) & up_one_row : 0;
        }

        uint64_t row = air.rows[w.y];
        result |= w.x > 0 && !friendly_worm_will_be_at_position(w.x - 1, w.y)
                            ? (1ULL << (w.x - 1)) & row : 0;
        result |= w.x < WIDTH && !friendly_worm_will_be_at_position(w.x + 1, w.y) 
                        ? (1ULL << (w.x + 1)) & row : 0;

        if (w.y < WIDTH) {
            uint64_t down_one_row = air.rows[w.y + 1];
            result |= !friendly_worm_will_be_at_position(w.x, w.y + 1)
                ? ((1ULL << w.x) & down_one_row) : 0;
            result |= w.x > 0 && !friendly_worm_will_be_at_position(w.x - 1, w.y) 
                ? (1ULL << (w.x - 1)) & down_one_row : 0;
            result |= w.x < WIDTH && !friendly_worm_will_be_at_position(w.x + 1, w.y) 
                            ? (1ULL << (w.x + 1)) & down_one_row : 0;
        }
        return result;
    }

    bool friendly_worm_will_be_at_position(uint8_t x, uint8_t y) {
        for (auto it = my_worms; it < my_worms + 3; it++) {
            game_worm w = *it;
            if (!w.is_alive()) continue;
            if (w.action.a == MOVE && w.x + w.action.del_x == x && w.y + w.action.del_y == y) 
                return true;
            if (w.x == x && w.y == y) return true;
        }
        return false;
    }
    
    bool might_shoot_north(game_worm w, game_worm in_range_enemy) {
        if (abs(w.x - in_range_enemy.x) > 1) return false;
        uint8_t distance = abs(w.y - in_range_enemy.y);
        uint64_t row_mask = 1ULL << w.x;    
        for (uint8_t i = 1; i < distance; i++) {
            uint8_t y = w.y - i;
            if (dirt.rows[y] & row_mask) return false;
            if (friendly_worm_will_be_at_position(w.x, y)) return false;
        }
        return true;
    }

    bool might_shoot_south(game_worm w, game_worm in_range_enemy) {
        if (abs(w.x - in_range_enemy.x) > 1) return false;
        uint8_t distance = abs(w.y - in_range_enemy.y);
        uint64_t row_mask = 1ULL << w.x;
        for (uint8_t i = 1; i < distance; i++) {
            uint8_t y = w.y + i;
            if (dirt.rows[y] & row_mask) return false;
            if (friendly_worm_will_be_at_position(w.x, y)) return false;
        }
        return true;
    }

    bool might_shoot_west(game_worm w, game_worm in_range_enemy) {
        if (abs(w.y - in_range_enemy.y) > 1) return false;
        uint8_t distance = abs(w.x - in_range_enemy.x);
        uint64_t current_row = dirt.rows[w.y];
        for (uint8_t i = 1; i < distance; i++) {
            uint8_t x = w.x - i;
            if (current_row & (1ULL << x)) return false;
            if (friendly_worm_will_be_at_position(x, w.y)) return false;
        }
        return true;
    }

    bool might_shoot_east(game_worm w, game_worm in_range_enemy) {
        if (abs(w.y - in_range_enemy.y) > 1) return false;
        uint8_t distance = abs(w.x - in_range_enemy.x);
        uint64_t current_row = dirt.rows[w.y];
        for (uint8_t i = 1; i < distance; i++) {
            uint8_t x = w.x + i;
            if (current_row & (1ULL << x)) return false;
            if (friendly_worm_will_be_at_position(x, w.y)) return false;
        }
        return true;
    }

    bool might_shoot_ne(game_worm w, game_worm in_range_enemy, double distance) {
        if (abs(w.y - in_range_enemy.y) > 1) return false;
        double root_two = sqrt(2);
        for (uint8_t i = 1; i * root_two < distance; i++) {
            uint8_t x = w.x + i;
            uint8_t y = w.y - i;
            if (dirt.rows[y] & (1ULL << x)) return false;
            if (friendly_worm_will_be_at_position(x, y)) return false;
        }
        return true;
    }

    bool might_shoot_se(game_worm w, game_worm in_range_enemy, double distance) {
        if (abs(w.y - in_range_enemy.y) > 1) return false;
        double root_two = sqrt(2);
        for (uint8_t i = 1; i * root_two < distance; i++) {
            uint8_t x = w.x + i;
            uint8_t y = w.y + i;
            if (dirt.rows[y] & (1ULL << x)) return false;
            if (friendly_worm_will_be_at_position(x, y)) return false;
        }
        return true;
    }

    bool might_shoot_nw(game_worm w, game_worm in_range_enemy, double distance) {
        if (abs(w.y - in_range_enemy.y) > 1) return false;
        double root_two = sqrt(2);
        for (uint8_t i = 1; i * root_two < distance; i++) {
            uint8_t x = w.x - i;
            uint8_t y = w.y - i;
            if (dirt.rows[y] & (1ULL << x)) return false;
            if (friendly_worm_will_be_at_position(x, y)) return false;
        }
        return true;
    }

    bool might_shoot_sw(game_worm w, game_worm in_range_enemy, double distance) {
        if (abs(w.y - in_range_enemy.y) > 1) return false;
        double root_two = sqrt(2);
        for (uint8_t i = 1; i * root_two < distance; i++) {
            uint8_t x = w.x - i;
            uint8_t y = w.y + i;
            if (dirt.rows[y] & (1ULL << x)) return false;
            if (friendly_worm_will_be_at_position(x, y)) return false;
        }
        return true;
    }

    double euclidean_distance(game_worm one, game_worm other) {
        if (one.x == other.x) return abs(one.x - other.x);
        if (one.y == other.y) return abs(one.y - other.y);
        uint8_t delx = one.x - other.x;
        uint8_t dely = one.y - other.y;
        return sqrt(delx * delx + dely * dely);
    }

    uint8_t shoot_candidates(game_worm w) {
        uint8_t result = 0;
        for (auto it = opponent_worms; it < opponent_worms + 3; it++) {
            game_worm other = *it;
            double distance = euclidean_distance(w, other);
            if (other.is_alive() && distance <= range) {
                if (might_shoot_north(w, other)) {
                    result |= N;
                    continue;
                }
                if (might_shoot_ne(w, other, distance)) { 
                    result |= NE;
                    continue;
                }
                if (might_shoot_east(w, other, distance)) {
                    result |= E;
                    continue;
                }
                if (might_shoot_se(w, other, distance)) {
                    result |= SE;
                    continue;
                }
                if (might_shoot_south(w, other)) {
                    result |= S;
                    continue;
                }
                if (might_shoot_sw(w, other, distance)) {
                    result |= SW;
                    continue;
                }
                if (might_shoot_west(w, other)) {
                    result |= W;
                    continue;
                }
                if (might_shoot_nw(w, other, distance)) {
                    result |= NW;
                }
            }
        }
        return result;
    }

    uint8_t damage;
    uint8_t range;
    uint8_t digging_range;
    layer<WIDTH> deep_space;
    layer<WIDTH> air;
    layer<WIDTH> dirt;
    game_worm my_worms[3] = {};
    game_worm opponent_worms[3] = {};
    selected_action allocated_moves[3] = {};

};

#endif
