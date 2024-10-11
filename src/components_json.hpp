#include "components.hpp"
#include "json.hpp"

using json = nlohmann::json;

/**
 * Defines the JSON serializer/deserialzer for the third-party GLM vec2 and vec3
 * types. Read more here: https://json.nlohmann.me/features/arbitrary_types/
 */
template <> struct nlohmann::adl_serializer<vec2> {
    static void to_json(json& j, const glm::vec2& c) {
        j = json::array({c.x, c.y});
    }
    static void from_json(const json& j, glm::vec2& c) {
        j.at(0).get_to(c.x);
        j.at(1).get_to(c.y);
    }
};
template <> struct nlohmann::adl_serializer<vec3> {
    static void to_json(json& j, const glm::vec3& c) {
        j = json::array({c.x, c.y, c.z});
    }
    static void from_json(const json& j, glm::vec3& c) {
        j.at(0).get_to(c.x);
        j.at(1).get_to(c.y);
        j.at(2).get_to(c.z);
    }
};


////////////////////////////////////////////////////////////////
///
/// Custom JSON (de)serialization for our components
///
////////////////////////////////////////////////////////////////

// ChangeScene
void to_json(json& j, const ChangeScene& c) {
    j = json{{"type", "change_scene"}, {"scene", (int)c.scene}};
}
void from_json(const json& j, ChangeScene& c) { j.at("scene").get_to(c.scene); }

// Interactable
void to_json(json& j, const Interactable& c) {
    (void)c;
    j = json{{"type", "interactable"}};
}
void from_json(const json& j, Interactable& c) {
    (void)j;
    (void)c;
}

// BoundingBox
void to_json(json& j, const BoundingBox& c) {
    j = json{{"type", "bounding_box"},
             {"position", c.position.x},
             {"scale", c.scale}};
}
void from_json(const json& j, BoundingBox& c) {
    j.at("position").get_to(c.position);
    j.at("scale").get_to(c.scale);
}

// Zone
void to_json(json& j, const Zone& c) {
    j = json{
        {"type", "zone"},
        {"position", c.position},
        {"zone_type", (ZONE_TYPE)c.type},
    };
}
void from_json(const json& j, Zone& c) {
    j.at("position").get_to(c.position);
    j.at("zone_type").get_to(c.type);
}

// LightSource
void to_json(json& j, const LightSource& c) {
    j = json{{"type", "light_source"}, {"angle", (float)c.angle}};
}
void from_json(const json& j, LightSource& c) { j.at("angle").get_to(c.angle); }

// OnLinearRails
void to_json(json& j, const OnLinearRails& r) {
    j = json{{"type", "on_linear_rails"},
             {"angle", (float)r.angle},
             {"length"},
             (float)r.length};
}
void from_json(const json& j, OnLinearRails& r) {
    j.at("angle").get_to(r.angle);
    j.at("length").get_to(r.length);
}

// LinearlyInterpolatable
void from_json(const json& j, LinearlyInterpolatable& lr) {
    j.at("t").get_to(lr.t);
    j.at("should_switch_direction").get_to(lr.should_switch_direction);
    j.at("t_step").get_to(lr.t_step);
}
void to_json(json& j, const LinearlyInterpolatable& lr) {
    j = json{{"t", lr.t},
             {"should_switch_direction", lr.should_switch_direction},
             {"t_step", lr.t_step}};
}
