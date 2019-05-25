#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "board.hpp"
#include <random>

using namespace std;

struct moves {
    selected_action actions[3];
};

enum result_score : uint8_t {
    YOU_WIN = 0,
    DRAW = 1,
    I_WIN = 2
};

template<uint8_t WIDTH>
struct simulation {

    board<WIDTH> b;
    mt19937 mt;

    explicit simulation(board<WIDTH> b) : b(b) {}

    direction select_direction(uint8_t available) {
        uint8_t set_bits = __builtin_popcount(available);
        uint8_t selected_bit = mt() % set_bits;
        for (uint8_t i = 0; i < 8; i++) {
            uint8_t current_bit = set_bits & 1 << i;
            if (current_bit && selected_bit == 0) return (direction)current_bit;
            else if (current_bit) {
                selected_bit--;
            }
            assert(false);
        }
    }

    selected_action select_safe_shot(game_worm me, game_worm* mine) {
        uint8_t options = NE | NW | SE | SW;
        for (game_worm* it = mine; it != mine + 3; it++) {
            game_worm w = *it;
            position p = w.action.a == MOVE ? w.p + w.action.p : w.p;
            if (p.x > me.p.x && p.y < me.p.y) options ^= NE;
            else if (p.x < me.p.x && p.y < me.p.y) options ^= NW;
            else if (p.x > me.p.x && p.y > me.p.y) options ^= SE;
            else if (p.x < me.p.x && p.y > me.p.y) options ^= SW;
        }
        return selected_action(select_direction(options), SHOOT);
    }

    selected_action select_action(direction direction, action a) {
        switch (direction) {
        case NONE:
            assert(false);
            return { {0, 0}, a };
        case N:
            return { {0, -1}, a };
        case NE:
            return { {1, -1}, a };
        case E:
            return { {1, 0}, a };
        case SE:
            return { {1, 1}, a };
        case S:
            return { {0, 1}, a };
        case SW:
            return { {-1, 1}, a };
        case W:
            return { {-1, 0}, a };
        case NW:
            return { {-1, -1}, a };
        }
    }

    selected_action select_action(game_worm w, game_worm* mine, game_worm* yours) {

        uint8_t move_candidates = b.move_candidates(w, mine);
        uint8_t dig_candidates = b.dig_candidates(w, mine, yours);
        uint8_t shoot_candidates = b.shoot_candidates(w, mine, yours);
        if (move_candidates && dig_candidates && shoot_candidates) {
            uint8_t draw = mt() % 3;
            if (draw == 0) {
                direction d = select_direction(move_candidates);
                return select_action(d, MOVE);
            } else if (draw == 1) {
                direction d = select_direction(dig_candidates);
                return select_action(d, DIG);
            } else {
                direction d = select_direction(shoot_candidates);
                return select_action(d, SHOOT);
            }
        }
        if (move_candidates && dig_candidates) {
            uint8_t draw = mt() % 2;
            if (draw == 0) {
                direction d = select_direction(move_candidates);
                return select_action(d, MOVE);
            } else {
                direction d = select_direction(dig_candidates);
                return select_action(d, DIG);
            }
        }
        if (move_candidates && shoot_candidates) {
            uint8_t draw = mt() % 2;
            if (draw == 0) {
                direction d = select_direction(move_candidates);
                return select_action(d, MOVE);
            } else {
                direction d = select_direction(shoot_candidates);
                return select_action(d, SHOOT);
            }
        }
        if (dig_candidates && shoot_candidates) {
            uint8_t draw = mt() % 2;
            if (draw == 0) {
                direction d = select_direction(dig_candidates);
                return select_action(d, DIG);
            } else {
                direction d = select_direction(shoot_candidates);
                return select_action(d, SHOOT);
            }
        }
        if (move_candidates) {
            direction d = select_direction(move_candidates);
            return select_action(d, MOVE);
        }
        if (dig_candidates) {
            direction d = select_direction(dig_candidates);
            return select_action(d, DIG);
        }
        if (shoot_candidates) {
            direction d = select_direction(shoot_candidates);
            return select_action(d, SHOOT);
        }
        return b.select_safe_shot(w, mine, yours);
    }

    void move(game_worm* wrm) {
        game_worm w = *wrm;
        assert(w.action.a == MOVE);
        assert(!b.obstructed(w.p + w.action.p));
        w.p += w.action.p;
        *wrm = w;
    }

    void dig(game_worm w) {
        position dirt_position = w.p + w.action.p;
        b.dirt.rows[dirt_position.y] ^= 1ULL << dirt_position.x;
    }

    bool hit_friendly(position p, game_worm* mine) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            game_worm w = *it;
            if (w.is_alive() && p == w.p) {
                w.health -= b.damage;
                return true;
            }
        }
        return false;
    }

    bool hit_enemy(position p, game_worm* enemies) {
        for (game_worm* it = enemies; it != enemies + 3; it++) {
            game_worm enemy = *it;
            if (enemy.is_alive() && p == enemy.p) {
                enemy.health -= b.damage;
                return true;
            }
        }
        return false;
    }

    void shoot(game_worm w, game_worm* mine, game_worm* enemies) {
        assert(w.action.a == SHOOT);
        position p = w.p + w.action.p;
        while (b.in_range(w.p, p)) {
            if (b.obstructed(p)) return;
            if (hit_enemy(p, enemies)) return;
            if (hit_friendly(p, mine)) return;
        }
    }

    void apply_moves(game_worm* mine) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            if (it->action.a == MOVE) {
                move(it);
            }
        }
    }

    void apply_shots(game_worm* mine, game_worm* yours) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            if (it->action.a == SHOOT) {
                shoot(*it, mine, yours);
            }
        }
    }

    void apply_digs(game_worm* mine, game_worm* yours) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            if (it->action.a == DIG) {
                shoot(*it, mine, yours);
            }
        }
    }

    void select_actions(game_worm* mine, game_worm* yours) {
        for (game_worm* it = mine; it != mine + 3; it++) {
            if (it->is_alive()) it->action = select_action(*it, mine, yours);
        }
    }

    void step() {
        select_actions(b.my_worms, b.opponent_worms);
        select_actions(b.opponent_worms, b.my_worms);
        apply_moves(b.my_worms);
        apply_moves(b.opponent_worms);
        apply_digs(b.my_worms, b.opponent_worms);
        apply_digs(b.opponent_worms, b.my_worms);
        apply_shots(b.my_worms, b.opponent_worms);
        apply_shots(b.opponent_worms, b.my_worms);
        b.reset_actions();
    }

};

#endif
