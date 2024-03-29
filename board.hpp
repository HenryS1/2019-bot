#ifndef _BOARD_H_
#define _BOARD_H_

#include "data.hpp"
#include <stdint.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <algorithm>

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

struct position {

    position() {}

    position(int8_t x, int8_t y) : x(x), y(y) {}

    position operator+(position other) { return position(x + other.x, y + other.y); }
    bool operator==(position other) { return x == other.x && y == other.y; }
    void operator+=(position other) { x += other.x; y += other.y; }

    int8_t x;
    int8_t y;
};

struct selected_action {

    position p;
    action a = NOTHING;
};

struct game_worm {

    game_worm() {}

    game_worm(uint8_t x, uint8_t y, uint16_t health) : p(x, y), health(health) {}

    bool is_alive() { return health > 0; }

    position p;
    int16_t health;
    selected_action action;

};

template<uint8_t WIDTH>
struct board {

    board(const layer<WIDTH>& dirt,
          const layer<WIDTH>& air,
          const layer<WIDTH>& deep_space,
          uint8_t damage, uint8_t range,
          uint8_t digging_range) : dirt(dirt), air(air), deep_space(deep_space),
                                   damage(damage), range(range), digging_range(digging_range) {}

    board(const vector<vector<cell>>& map,
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

    direction direction_between(position one, position other) {
        if (one.x == other.x) {
            if (one.y > other.y) {
                return S;
            } else if (one.y < other.y) {
                return N;
            }
        } else if (one.y == other.y) {
            if (one.x > other.x) {
                return E;
            } else if (one.x < other.x) {
                return W;
            }
        } else if (one.y - other.y == one.x - other.x) {
            if (one.x > other.x) {
                return SE;
            } else if (one.x < other.x) {
                return NW;
            }
        } else if (one.y - other.y == other.x - one.x) {
            if (one.x > other.x) {
                return NE;
            } else if (one.x < other.x) {
                return SW;
            }
        }
        return NONE;
    }

    bool enemy_between(position one, position other, game_worm* enemies) {
        for (game_worm* it = enemies; it != enemies + 3; it++) {
            game_worm enemy = *it;
            if (!in_range(one, enemy.p)) continue;
            if (direction_between(one, enemy.p) == direction_between(enemy.p, other)) {
                return true;
            }
        }
        return false;
    }

    bool will_open_shoot_path_to_friendly(position p, game_worm* mine, game_worm* enemies) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            game_worm one = *it;
            if (one.action.a == SHOOT) {
                for (game_worm* it_other = mine; it_other != mine + 3; it_other++) {
                    game_worm other = *it_other;
                    if (!in_range(one.p, other.p)) continue;
                    if (direction_between(one.p, other.p) == NONE) continue;
                    if (enemy_between(one.p, other.p, enemies)) continue;
                    if (has_clear_path(one.p, p, mine) && has_clear_path(other.p, p, mine)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    uint8_t dig_candidates(game_worm w, game_worm* mine, game_worm* enemies) {
        uint8_t result = 0;
        if (w.p.y > 0) {
            uint64_t up_one_row = dirt.rows[w.p.y - 1];
            result |= ((1ULL << w.p.x) & up_one_row) &&
                !will_open_shoot_path_to_friendly(position(w.p.x, w.p.y - 1), mine, enemies)
                ? N : 0;
            result |= w.p.x > 0 && ((1ULL << (w.p.x - 1)) & up_one_row) &&
                !will_open_shoot_path_to_friendly(position(w.p.x - 1, w.p.y - 1), mine, enemies)
                ? NW : 0;
            result |= w.p.x < WIDTH && ((1ULL << (w.p.x + 1)) & up_one_row) &&
                !will_open_shoot_path_to_friendly(position(w.p.y - 1, w.p.x + 1), mine, enemies)
                ? NE : 0;
        }

        uint64_t row = dirt.rows[w.p.y];
        result |= w.p.x > 0 && ((1ULL << (w.p.x - 1)) & row) &&
            !will_open_shoot_path_to_friendly(position(w.p.y, w.p.x - 1), mine, enemies) ? W : 0;
        result |= w.p.x < WIDTH && ((1ULL << (w.p.x + 1)) & row) &&
            !will_open_shoot_path_to_friendly(position(w.p.y, w.p.x + 1), mine, enemies) ? E : 0;

        if (w.p.y < WIDTH) {
            uint64_t down_one_row = dirt.rows[w.p.y + 1];
            result |= ((1ULL << w.p.x) & down_one_row) &&
                !will_open_shoot_path_to_friendly(position(w.p.y + 1, w.p.x), mine, enemies)
                ? S : 0;
            result |= w.p.x > 0 && ((1ULL << (w.p.x - 1)) & down_one_row) &&
                !will_open_shoot_path_to_friendly(position(w.p.y + 1, w.p.x - 1), mine, enemies)
                ? SW : 0;
            result |= w.p.x < WIDTH && ((1ULL << (w.p.x + 1)) & down_one_row) &&
                !will_open_shoot_path_to_friendly(position(w.p.y + 1, w.p.x + 1), mine, enemies)
                ? SE : 0;
        }

        return result;
    }

    bool between(uint8_t start, uint8_t between, uint8_t end) {
        return start <= between && between <= end;
    }

    double euclidean_distance(position one, position other) {
        if (one.x == other.x) return abs(one.y - other.y);
        if (one.y == other.y) return abs(one.x - other.x);
        int16_t delx = one.x - other.x;
        int16_t dely = one.y - other.y;
        return sqrt(delx * delx + dely * dely);
    }

    bool in_range(position one, position other) {
        return euclidean_distance(one, other) <= range + sqrt(2);
    }

    bool friendly_is_digging_cell(position p, game_worm* mine) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            game_worm w = *it;
            if (w.action.a == DIG && (w.p + w.action.p) == p) {
                return true;
            }
        }
        return false;
    }

    bool has_clear_path(position one, position other, game_worm* mine) {
        for (uint8_t x = one.x, y = one.y; (x != other.x || y != other.y);
             y = min<uint8_t>(y+1, other.y), x = min<uint8_t>(x + 1, other.x)) {
            uint64_t row_mask = 1ULL << x;
            if (deep_space.rows[y] & row_mask) return false;
            if (dirt.rows[y] & row_mask && !friendly_is_digging_cell(position(x, y), mine)) {
                return false;
            }
        }
        return true;
    }

    bool friendly_worm_will_shoot_at_position(game_worm* mine, position p) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            game_worm w = *it;
            if (w.action.a != SHOOT) continue;
            if (direction_between(w.p, p) == NONE) continue;
            if (!in_range(w.p, p)) continue;
            if (!between(w.p.x, w.p.x + w.action.p.x, p.x) &&
                !between(p.x, w.p.x + w.action.p.x, w.p.x)) continue;
            if (!between(w.p.y, w.p.y + w.action.p.y, p.y) &&
                !between(p.y, w.p.y + w.action.p.y, w.p.y)) continue;
            return has_clear_path(w.p + w.action.p, p, mine);
        }
        return false;
    }

    bool safe_to_move_to(game_worm* mine, position p) {
        return !friendly_worm_will_shoot_at_position(mine, p) &&
            !friendly_worm_will_be_at_position(p, mine);
    }

    uint8_t move_candidates(game_worm w, game_worm* mine) {
        uint8_t result = 0;
        if (w.p.y > 0) {
            uint64_t up_one_row = air.rows[w.p.y - 1];
            result |= safe_to_move_to(mine, position(w.p.x, w.p.y - 1))
                     && ((1ULL << w.p.x) & up_one_row) ? N : 0;
            result |= w.p.x > 0 &&
                safe_to_move_to(mine, position(w.p.x - 1, w.p.y - 1))
                && ((1ULL << (w.p.x - 1)) & up_one_row) ? NW : 0;
            result |= w.p.x < WIDTH &&
                              safe_to_move_to(mine, position(w.p.x + 1, w.p.y - 1))
                              && ((1ULL << (w.p.x + 1)) & up_one_row) ? NE : 0;
        }

        uint64_t row = air.rows[w.p.y];
        result |= w.p.x > 0 && safe_to_move_to(mine, position(w.p.x - 1, w.p.y))
            && ((1ULL << (w.p.x - 1)) & row) ? W : 0;
        result |= w.p.x < WIDTH &&
                          safe_to_move_to(mine, position(w.p.x + 1, w.p.y))
                        && ((1ULL << (w.p.x + 1)) & row) ? E : 0;

        if (w.p.y < WIDTH) {
            uint64_t down_one_row = air.rows[w.p.y + 1];
            result |= safe_to_move_to(mine, position(w.p.x, w.p.y + 1))
                && ((1ULL << w.p.x) & down_one_row) ? S : 0;
            result |= w.p.x > 0 &&
                safe_to_move_to(mine, position(w.p.x - 1, w.p.y))
                && ((1ULL << (w.p.x - 1)) & down_one_row) ? SW : 0;
            result |= w.p.x < WIDTH &&
                              safe_to_move_to(mine, position(w.p.x + 1, w.p.y))
                            && ((1ULL << (w.p.x + 1)) & down_one_row) ? SE : 0;
        }
        return result;
    }

    bool obstructed(position p) {
        return get_obstructions(p.y) & 1ULL << p.x;
    }

    bool friendly_worm_will_be_at_position(position p, game_worm* mine) {
        for (auto it = mine; it < mine + 3; it++) {
            game_worm w = *it;
            if (!w.is_alive()) continue;
            if (w.action.a == MOVE && w.p + w.action.p == p)
                return true;
            else if (w.p == p) return true;
        }
        return false;
    }

    bool dirt_might_get_dug_out(position p, game_worm enemy_to_shoot,
                                game_worm* mine, game_worm* opponents) {
        for (game_worm* it = mine; it < mine + 3; it++) {
            if (dirt_cell_might_get_dug_out_by_my_worm(p, *it)) return true;
        }
        for (game_worm* it = opponents; it < opponents + 3; it++) {
            game_worm w = *it;
            if (w.p == enemy_to_shoot.p) continue;
            if (abs(w.p.y - p.y) <= 1 && abs(w.p.x - p.x) <= 1) return true;
        }
        return false;
    }

    bool dirt_cell_might_get_dug_out_by_my_worm(position p, game_worm w) {
        return (w.action.a == DIG) && w.p + w.action.p == p;
    }

    bool might_shoot_north(game_worm w, game_worm in_range_enemy,
                           game_worm* mine, game_worm* opponents) {
        if (abs(w.p.x - in_range_enemy.p.x) > 1 || w.p.y < in_range_enemy.p.y) return false;
        uint8_t distance = min(range, (uint8_t)abs(w.p.y - in_range_enemy.p.y));
        uint64_t row_mask = 1ULL << w.p.x;
        for (uint8_t i = 1; i <= distance; i++) {
            position p(w.p.x, w.p.y - i);
            if ((get_obstructions(p.y) & row_mask) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    bool might_shoot_south(game_worm w, game_worm in_range_enemy,
                           game_worm* mine, game_worm* opponents) {
        if (abs(w.p.x - in_range_enemy.p.x) > 1 || w.p.y > in_range_enemy.p.y) return false;
        uint8_t distance = min(range, (uint8_t)abs(w.p.y - in_range_enemy.p.y));
        uint64_t row_mask = 1ULL << w.p.x;
        for (uint8_t i = 1; i <= distance; i++) {
            position p(w.p.x, w.p.y + i);
            if ((get_obstructions(p.y) & row_mask) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    bool might_shoot_west(game_worm w, game_worm in_range_enemy,
                          game_worm* mine, game_worm* opponents) {
        if (abs(w.p.y - in_range_enemy.p.y) > 1 || w.p.x < in_range_enemy.p.x) return false;
        uint8_t distance = min(range, (uint8_t)abs(w.p.x - in_range_enemy.p.x));
        uint64_t current_row = get_obstructions(w.p.y);
        for (uint8_t i = 1; i <= distance; i++) {
            position p(w.p.x - i, w.p.y);
            if ((current_row & (1ULL << p.x)) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    bool might_shoot_east(game_worm w, game_worm in_range_enemy,
                          game_worm* mine, game_worm* opponents) {
        if (abs(w.p.y - in_range_enemy.p.y) > 1 || w.p.x > in_range_enemy.p.x) return false;
        uint8_t distance = min(range, (uint8_t)abs(w.p.x - in_range_enemy.p.x));
        uint64_t current_row = get_obstructions(w.p.y);
        for (uint8_t i = 1; i <= distance; i++) {
            position p(w.p.x + i, w.p.y);
            if ((current_row & (1ULL << p.x)) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    bool might_shoot_ne(game_worm w, game_worm in_range_enemy,
                        double distance, game_worm* mine, game_worm* opponents) {
        if (abs(w.p.x - in_range_enemy.p.x - (in_range_enemy.p.y - w.p.y)) > 1 ||
            w.p.x > in_range_enemy.p.x) return false;
        distance = min((double)range, distance);
        double root_two = sqrt(2);
        for (uint8_t i = 1; i * root_two <= distance; i++) {
            position p(w.p.x + i, w.p.y - i);
            if ((get_obstructions(p.y) & (1ULL << p.x)) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    bool might_shoot_se(game_worm w, game_worm in_range_enemy,
                        double distance, game_worm* mine, game_worm* opponents) {
        if (abs(w.p.x - w.p.y - (in_range_enemy.p.x - in_range_enemy.p.y)) > 1 ||
            w.p.x > in_range_enemy.p.x) return false;
        double root_two = sqrt(2);
        distance = min((double)range, distance);
        for (uint8_t i = 1; i * root_two <= distance; i++) {
            position p(w.p.x + i, w.p.y + i);
            if ((get_obstructions(p.y) & (1ULL << p.x)) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    bool might_shoot_nw(game_worm w, game_worm in_range_enemy,
                        double distance, game_worm* mine, game_worm* opponents) {
        if (abs(w.p.x - w.p.y - (in_range_enemy.p.x - in_range_enemy.p.y)) > 1 ||
            w.p.x < in_range_enemy.p.x) return false;
        double root_two = sqrt(2);
        distance = min((double)range, distance);
        for (uint8_t i = 1; i * root_two <= distance; i++) {
            position p(w.p.x - i, w.p.y - i);
            if ((get_obstructions(p.y) & (1ULL << p.x)) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    uint64_t get_obstructions(uint8_t row) {
        return dirt.rows[row] | deep_space.rows[row];
    }

    bool might_shoot_sw(game_worm w, game_worm in_range_enemy,
                        double distance, game_worm* mine, game_worm* opponents) {
        if (abs(w.p.x - in_range_enemy.p.x - (in_range_enemy.p.y - w.p.y)) > 1 ||
            w.p.x < in_range_enemy.p.x) return false;
        double root_two = sqrt(2);
        distance = min((double)range, distance);
        for (uint8_t i = 1; i * root_two <= distance; i++) {
            position p(w.p.x - i, w.p.y + i);
            if ((get_obstructions(p.y) & (1ULL << p.x)) &&
                !dirt_might_get_dug_out(p, in_range_enemy, mine, opponents))
                return false;
            if (friendly_worm_will_be_at_position(p, mine)) return false;
        }
        return true;
    }

    void reset_actions() {
        for (game_worm* it = my_worms; it != my_worms + 3; it++) {
            it->action = {};
        }
        for (game_worm* it = opponent_worms; it != opponent_worms + 3; it++) {
            it->action = {};
        }
    }

    uint8_t shoot_candidates(game_worm w, game_worm* mine, game_worm* opponents) {
        uint8_t result = 0;
        for (auto it = opponents; it < opponents + 3; it++) {
            game_worm other = *it;
            double distance = euclidean_distance(w.p, other.p);
            if (other.is_alive() && in_range(w.p, other.p)) {
                if (might_shoot_north(w, other, mine, opponents)) {
                    result |= N;
                }
                if (might_shoot_ne(w, other, distance, mine, opponents)) {
                    result |= NE;
                }
                if (might_shoot_east(w, other, mine, opponents)) {
                    result |= E;
                }
                if (might_shoot_se(w, other, distance, mine, opponents)) {
                    result |= SE;
                }
                if (might_shoot_south(w, other, mine, opponents)) {
                    result |= S;
                }
                if (might_shoot_sw(w, other, distance, mine, opponents)) {
                    result |= SW;
                }
                if (might_shoot_west(w, other, mine, opponents)) {
                    result |= W;
                }
                if (might_shoot_nw(w, other, distance, mine, opponents)) {
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
