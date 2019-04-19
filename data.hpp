#ifndef _DATA_H_
#define _DATA_H_

#include "json.hpp"
#include <vector>

using namespace std;

struct weapon {
    int damage;
    int range;
};

struct cell {
    int x;
    int y;
    string type;
};

struct map_position {
    int x;
    int y;
};

struct worm {
    int id;
    int health;
    map_position position;
    int diggingRange;
    int movementRange;
};

struct opponent {
    int id;
    int score;
    vector<worm> worms;
};

struct my_worm {
    int id;
    int health;
    map_position position;
    int diggingRange;
    int movementRange;
    weapon weapon;
};

struct powerup {
    string type;
    int value;
};

struct my_player {
    int id;
    int score;
    int health;
    vector<my_worm> worms;
};

struct game_state {
    int currentRound;
    int maxRounds;
    int currentWormId;
    int consecutiveDoNothingCount;
    my_player myPlayer;
    vector<opponent> opponents;
};

#endif
