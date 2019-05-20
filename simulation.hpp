#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "board.hpp"
#include <random>

using namespace std;

struct moves {
    selected_action actions[3];
};

template<uint8_t WIDTH>
struct simulation {

    board<WIDTH> b;
    mt19937 mt;

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

    selected_action select_action(direction direction, action a) {
        switch (direction) {
        case NONE:
            assert(false);
            return { 0, 0, a };
        case N:
            return { 0, -1, a };
        case NE:
            return { 1, -1, a };
        case E:
            return { 1, 0, a };
        case SE:
            return { 1, 1, a };
        case S:
            return { 0, 1, a };
        case SW:
            return { -1, 1, a };
        case W:
            return { -1, 0, a };
        case NW:
            return { -1, -1, a };
        }
    }

    selected_action select_action(game_worm w, game_worm* mine, game_worm* yours) {
        uint64_t draw = mt() % 2;
        if (draw == 0) {
            uint8_t move_candidates = b.move_candidates(w, mine, yours);
            if (move_candidates) {
                direction d = select_direction(move_candidates);
                return select_action(d, MOVE);
            } else {
                return {0, 0, NOTHING};
            }
        } else if (draw == 1) {
            uint8_t dig_candidates = b.dig_candidates(w);
            if (dig_candidates) {
                direction d = select_direction(dig_candidates);
                return select_action(d, DIG);
            } else {
                return {0, 0, NOTHING};
            }
        } else {
            uint8_t shoot_candidates = b.shoot_candidates(w, mine, yours);
            if (shoot_candidates) {
                direction d = select_direction(shoot_candidates);
                return select_action(d, SHOOT);
            } else {
                return {0, 0, NOTHING};
            }
        }
    }

    void step(board<WIDTH>& b) {
        
    }

};

#endif