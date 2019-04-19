#ifndef _READ_DATA_H_
#define _READ_DATA_H_

#include "data.hpp"
#include "json.hpp"

using namespace std;

using json = nlohmann::json;

void to_json(json& j, const weapon& w);
void from_json(const json& j, weapon& w);
void to_json(json& j, const cell& w);
void from_json(const json& j, cell& w);
void to_json(json& j, const map_position& it);
void from_json(const json& j, map_position& it);
void to_json(json& j, const worm& it);
void from_json(const json& j, worm& it);
void to_json(json& j, const opponent& it);
void from_json(const json& j, opponent& it);
void to_json(json& j, const my_worm& it);
void from_json(const json& j, my_worm& it);
void to_json(json& j, const powerup& it);
void from_json(const json& j, powerup& it);
void to_json(json& j, const my_player& it);
void from_json(const json& j, my_player& it);
void to_json(json& j, const game_state& it);
void from_json(const json& j, game_state& it);


struct read_data {
    game_state read_state(string filepath);
};

#endif
