#include "components.hpp"
#include "json.hpp"

using json = nlohmann::json;

// https://json.nlohmann.me/features/arbitrary_types/

// Vec2 and Vec3
template<> struct nlohmann::adl_serializer<vec2> {
    static void to_json(json& j, const glm::vec2& c) {
        j = json::array({c.x, c.y});
    }
    static void from_json(const json& j, glm::vec2& c) {
        j.at(0).get_to(c.x);
        j.at(1).get_to(c.y);
    }
};
template<> struct nlohmann::adl_serializer<vec3> {
    static void to_json(json& j, const glm::vec3& c) {
        j = json::array({c.x, c.y, c.z});
    }
    static void from_json(const json& j, glm::vec3& c) {
        j.at(0).get_to(c.x);
        j.at(1).get_to(c.y);
        j.at(2).get_to(c.z);
    }
};

// Our own structs
// TODO: krarpit: include (de)serialzation code along with struct definitions (or find a way to derive these from the struct definitions)

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
             {"position", c.position.x},
             {"scale", c.scale} };
}

void from_json(const json& j, BoundingBox& c) {
    j.at("position").get_to(c.position);
    j.at("scale").get_to(c.scale);
}

void to_json(json& j, const Zone& c) {
    j = json{{"type", "zone"},
             {"position", c.position},
             {"zone_type", (ZONE_TYPE)c.type},
    };
}

void from_json(const json& j, Zone& c) {
    j.at("position").get_to(c.position);
    j.at("zone_type").get_to(c.type);
}


void to_json(json& j, const LightSource& c) {
    j = json{{"type", "light_source"},
             {"angle", (float)c.angle}};
}

void from_json(const json& j, LightSource& c) {
    j.at("angle").get_to(c.angle);
}

void to_json(json& j, const OnLinearRails &r) {
  j = json{{"type", "on_linear_rails"},
    {"angle", (float)r.angle},
    {"length"}, (float)r.length};
}

void from_json(const json& j, OnLinearRails &r) {
  j.at("angle").get_to(r.angle);
  j.at("length").get_to(r.length);
}

