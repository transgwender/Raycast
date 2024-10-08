#include "components.hpp"
#include "json.hpp"

using json = nlohmann::json;

// https://json.nlohmann.me/features/arbitrary_types/

void to_json(json& j, const ChangeScene& c) {
    j = json{ {"type", "change_scene"}, {"scene", (int)c.scene} };
}

void from_json(const json& j, ChangeScene& c) {
    j.at("scene").get_to(c.scene);
}


void to_json(json& j, const Interactable& c) {
    (void)c;
    j = json{ {"type", "interactable"} };
}

void from_json(const json& j, Interactable& c) {
    (void)j;
    (void)c;
}

void to_json(json& j, const BoundingBox& c) {
    j = json{ {"type", "bounding_box"},
             {"position", json::array({c.position.x, c.position.y})},
             {"scale", json::array({c.scale.x, c.scale.y})} };
}

void from_json(const json& j, BoundingBox& c) {
    j.at("position").at(0).get_to(c.position.x);
    j.at("position").at(1).get_to(c.position.y);
    j.at("scale").at(0).get_to(c.scale.x);
    j.at("scale").at(1).get_to(c.scale.y);
}

void to_json(json& j, const Zone& c) {
    j = json{ {"type", "zone"},
             {"position", json::array({c.position.x, c.position.y})},
             {"zone_type", (int)c.type} };
}

void from_json(const json& j, Zone& c) {
    j.at("position").at(0).get_to(c.position.x);
    j.at("position").at(1).get_to(c.position.y);
    j.at("zone_type").get_to(c.type);
}