#include "read_data.hpp"
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

void to_json(json& j, const weapon& w) {
    j = json{{ "damage", w.damage }, { "range", w.range }};
}

void from_json(const json& j, weapon& w) {
    w.damage = j.at("damage").get<int>();
    w.range = j.at("range").get<int>();
}

void to_json(json& j, const cell& it) {
    j = json{{ "x", it.x }, { "y", it.y }, { "type", it.type }};
}
void from_json(const json& j, cell& it) {
    j.at("x").get_to(it.x);
    j.at("y").get_to(it.y);
    j.at("type").get_to(it.type);
}

void to_json(json& j, const map_position& it) {
    j = json{{ "x", it.x }, { "y", it.y }};
}

void from_json(const json& j, map_position& it) {
    it.x = j.at("x").get<int>();
    it.y = j.at("y").get<int>();
}

void to_json(json& j, const worm& it) {
    j = json{{ "id", it.id }, { "health", it.health }, { "position", it.position },
             { "diggingRange", it.diggingRange }, { "movementRange", it.movementRange }};
}

void from_json(const json& j, worm& it) {
    it.id = j.at("id").get<int>();
    it.health = j.at("health").get<int>();
    j.at("position").get_to(it.position);
    j.at("diggingRange").get_to(it.diggingRange);
    j.at("movementRange").get_to(it.movementRange);
}

void to_json(json& j, const opponent& it) {
    j = json{{ "id", it.id }, { "score", it.score }, { "worms", it.worms }};
}

void from_json(const json& j, opponent& it) {
    j.at("id").get_to(it.id);
    j.at("score").get_to(it.score);
    j.at("worms").get_to(it.worms);
}

void to_json(json& j, const my_worm& it) {
    j["id"] = it.id;
    j["health"] = it.health;
    j["position"] = it.position;
    j["diggingRange"] = it.diggingRange;
    j["movementRange"] = it.movementRange;
    j["weapon"] = it.weapon;
}

void from_json(const json& j, my_worm& it) {
    j.at("id").get_to(it.id);
    j.at("health").get_to(it.health);
    j.at("position").get_to(it.position);
    j.at("diggingRange").get_to(it.diggingRange);
    j.at("movementRange").get_to(it.movementRange);
    j.at("weapon").get_to(it.weapon);
}

void to_json(json& j, const powerup& it) {
    j["type"] = it.type;
    j["value"] = it.value;
}

void from_json(const json& j, powerup& it) {
    j.at("type").get_to(it.type);
    j.at("value").get_to(it.value);
}

void to_json(json& j, const my_player& it) {
    j["id"] = it.id;
    j["score"] = it.score;
    j["health"] = it.health;
    j["worms"] = it.worms;
}

void from_json(const json& j, my_player& it) {
    j.at("id").get_to(it.id);
    j.at("score").get_to(it.score);
    j.at("health").get_to(it.health);
    j.at("worms").get_to(it.worms);
}

void to_json(json& j, const game_state& it) {
    j["currentRound"] = it.currentRound;
    j["maxRounds"] = it.maxRounds;
    j["currentWormId"] = it.currentWormId;
    j["consecutiveDoNothingCount"] = it.consecutiveDoNothingCount;
    j["myPlayer"] = it.myPlayer;
    j["opponents"] = it.opponents;
    j["map"] = it.map;
}

void from_json(const json& j, game_state& it) {
    j.at("currentRound").get_to(it.currentRound);
    j.at("maxRounds").get_to(it.maxRounds);
    j.at("currentWormId").get_to(it.currentWormId);
    j.at("consecutiveDoNothingCount").get_to(it.consecutiveDoNothingCount);
    j.at("myPlayer").get_to(it.myPlayer);
    j.at("opponents").get_to(it.opponents);
    j.at("map").get_to(it.map);
}

game_state read_data::read_state(string filepath) {
    ifstream input(filepath);
    if (!input) {
        throw runtime_error("failed to open json file");
    }
    json state_json;
    input >> state_json;
    return state_json.get<game_state>();
}
