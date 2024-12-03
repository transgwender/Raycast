#pragma once

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
template <> struct nlohmann::adl_serializer<vec4> {
    static void to_json(json& j, const glm::vec4& c) {
        j = json::array({c.r, c.g, c.b, c.a});
    }
    static void from_json(const json& j, glm::vec4& c) {
        j.at(0).get_to(c.r);
        j.at(1).get_to(c.g);
        j.at(2).get_to(c.b);
        j.at(3).get_to(c.a);
    }
};


////////////////////////////////////////////////////////////////
///
/// Custom JSON (de)serialization for our components
///
////////////////////////////////////////////////////////////////

// ChangeScene
inline void to_json(json& j, const ChangeScene& c) {
    j = json{{"type", "change_scene"}, {"scene", c.scene}};
}

inline void from_json(const json& j, ChangeScene& c) { j.at("scene").get_to(c.scene); }

// Zone
inline void to_json(json& j, const Zone& c) {
    j = json{
        {"type", "zone"},
        {"position", c.position},
        {"zone_type", static_cast<ZONE_TYPE>(c.type)},
        {"mass", c.mass},
        {"force_field_radius", c.force_field_radius},
    };
}

inline void from_json(const json& j, Zone& c) {
    j.at("position").get_to(c.position);
    j.at("zone_type").get_to(c.type);
    c.mass = j.value("mass", 5.0);
    c.force_field_radius = j.value("force_field_radius", 50.0);
}

// LightSource
inline void to_json(json& j, const LightSource& c) {
    j = json{{"type", "light_source"}, {"angle", (float)c.angle}};
}
inline void from_json(const json& j, LightSource& c) { j.at("angle").get_to(c.angle); }

// LinearlyInterpolatable
inline void from_json(const json& j, Lerpable& lr) {
    j.at("t").get_to(lr.t);
    j.at("should_switch_direction").get_to(lr.should_switch_direction);
    j.at("t_step").get_to(lr.t_step);
}
inline void to_json(json& j, const Lerpable& lr) {
    j = json{{"t", lr.t},
             {"should_switch_direction", lr.should_switch_direction},
             {"t_step", lr.t_step}};
}

inline void to_json(json& j, const Level& c) {
    j = json{ {"type", "level"}, {"id", c.id} };
}

inline void from_json(const json& j, Level& c) {
    j.at("id").get_to(c.id);
}

inline void to_json(json& j, const EndLevel& c) {
    j = json{ {"type", "end_level"}, {"id", c.id} };
}

inline void from_json(const json& j, EndLevel& c) {
    j.at("id").get_to(c.id);
}

inline void to_json(json& j, const Reflective& c) {
    (void)c;
    j = json{{"type", "light_source"}};
}

inline void from_json(const json& j, Reflective& c) {
    (void)j;
    (void)c;
}


inline void to_json(json& j, const LevelSelect& c) {
    (void)c;
    j = json{{"type", "level_select"}};
}

inline void from_json(const json& j, LevelSelect& c) {
    (void)j;
    (void)c;
}

inline void to_json(json& j, const Sprite& c) {
    j = json{ {"type", "sprite"}, {"position", c.position}, {"scale", c.scale}, {"angle", c.angle}, {"texture", c.texture} };
}

inline void from_json(const json& j, Sprite& c) {
    j.at("position").get_to(c.position);
    j.at("texture").get_to(c.texture);
    if(j.contains("scale")) {
        j.at("scale").get_to(c.scale);
    } else {
        c.scale = vec2({200, 200});
    }
    if (j.contains("angle")) {
        j.at("angle").get_to(c.angle);
    } else {
        c.angle = 0;
    }
    if (j.contains("color")) {
        j.at("color").get_to(c.color);
    } else {
        c.color = {255, 255, 255, 255};
    }
}

inline void to_json(json& j, const Mirror& c) {
    j = json{
        {"type", "mirror"},
        {"position", c.position},
        {"angle", c.angle},
        {"mirror-type", c.mirrorType},
        {"rail-length", c.railLength},
        {"rail-angle", c.railAngle},
    };
}

inline void from_json(const json& j, Mirror& c) {
    j.at("position").get_to(c.position);
    j.at("angle").get_to(c.angle);

    if (j.contains("mirror-type")) {
        j.at("mirror-type").get_to(c.mirrorType);
    } else {
        c.mirrorType = "rails";
    }

    if (j.contains("rail-length")) {
        j.at("rail-length").get_to(c.railLength);
    } else {
        c.railLength = 0;
    }

    if (j.contains("rail-angle")) {
        j.at("rail-angle").get_to(c.railAngle);
    } else {
        c.railAngle = 0;
    }

    if (j.contains("snap-segments")) {
        c.snap_segments = j.at("snap-segments").get_to(c.snap_segments);
    } else {
        c.snap_segments = 5;
    }

    if (j.contains("snap-angle")) {
        c.snap_angle = j.at("snap-angle").get_to(c.snap_angle);
    } else {
        c.snap_angle = 0.2617993878;
    }
}

inline void to_json(json& j, const PointLight& c) {
    j = json{
        {"type", "point_light"},
        {"diffuse", c.diffuse},
        {"linear", c.linear},
        {"quadratic", c.quadratic},
        {"constant", c.constant},
    };
}

inline void from_json(const json& j, PointLight& c) {
    j.at("diffuse")[0].get_to(c.diffuse);
    j.at("linear")[0].get_to(c.linear);
    j.at("quadratic")[0].get_to(c.quadratic);
    j.at("constant")[0].get_to(c.constant);
}

inline void to_json(json& j, const Highlightable& c) {
    j = json{{"type", "highlightable"}, {"isHighlighted", c.isHighlighted}};
}

inline void from_json(const json& j, Highlightable& c) {
    j.at("isHighlighted").get_to(c.isHighlighted);
}

inline void to_json(json& j, const DashTheTurtle& c) {
    j = json{{"type", "dashTheTurtle"},
             {"behavior", c.behavior},
             {"minimumDisplacement", c.nearestLightRayDirection},
             {"originalPosition", c.originalPosition}};
}

inline void from_json(const json& j, DashTheTurtle& c) {

    j.at("behavior").get_to(c.behavior);
    j.at("minimumDisplacement").get_to(c.nearestLightRayDirection);
    j.at("originalPosition").get_to(c.originalPosition);
}

inline void to_json(json& j, const Motion& c) {
    j = json{{"type", "motion"},
             {"position", c.position},
             {"velocity", c.velocity},
             {"scale", c.scale},
             {"angle", c.angle}}; //, {"collides", c.collides}};
}

inline void from_json(const json& j, Motion& c) {
    j.at("position").get_to(c.position);
    j.at("velocity").get_to(c.velocity);
    j.at("scale").get_to(c.scale);
    j.at("angle").get_to(c.angle);
    // j.at("collides").get_to(c.collides);
}

inline void to_json(json& j, const ButtonHelper& c) {
    j = json{{"type", "button"}, {"position", c.position}, {"scale", c.scale}, {"label", c.label}};
}

inline void from_json(const json& j, ButtonHelper& c) {
    j.at("position").get_to(c.position);
    j.at("scale").get_to(c.scale);
    j.at("label").get_to(c.label);
}

inline void to_json(json& j, const Collider& c) {
    j = json{{{"type", "collider"},
        {"bounds", c.bounds_type},
        {"width"}, c.width},
        {"height", c.height}
    };
}

inline void from_json(const json& j, Collider& c) {
    j.at("bounds").get_to(c.bounds_type);
    j.at("width").get_to(c.width);
    j.at("height").get_to(c.height);
}

// Collideable
inline void to_json(json& j, const Collideable& c) {
    (void)c;
    j = json{{"type", "collideable"}};
}
inline void from_json(const json& j, Collideable& c) {
    (void)j;
    (void)c;
}

// Interactable
inline void to_json(json& j, const Interactable& c) {
    (void)c;
    j = json{{"type", "interactable"}};
}

inline void from_json(const json& j, Interactable& c) {
    (void)j;
    (void)c;
}

// Blackhole 
inline void to_json(json& j, const Blackhole& bh) {
    j = json{
        {"type", "blackhole"},
        {"mass", bh.mass},
        {"schwarzchild_radius", bh.schwarzchild_radius}
    };
}

inline void from_json(const json& j, Blackhole& bh) {
    j.at("mass").get_to(bh.mass);
    j.at("schwarzchild_radius").get_to(bh.schwarzchild_radius);
} 
inline void to_json(json& j, const SpriteSheet& c) {
    j = json{
        {"type", "sprite_sheet"},
        {"position", c.position},
        {"sheetWidth", c.sheetWidth},
        {"sheetHeight", c.sheetWidth },
        { "cellWidth", c.cellWidth},
        {"cellHeight", c.cellHeight},
        {"animationFrames", c.animationFrames}
    };
}

inline void from_json(const json& j, SpriteSheet& c) {
    j.at("position").get_to(c.position);
    j.at("sheetWidth").get_to(c.sheetWidth);
    j.at("sheetHeight").get_to(c.sheetHeight);
    j.at("cellWidth").get_to(c.cellWidth);
    j.at("cellHeight").get_to(c.cellHeight);
    j.at("animationFrames").get_to(c.animationFrames);
}

inline void to_json(json& j, const MiniSun& c) {
    j = json{{"type", "minisun"},
             {"lit", c.lit},
             {"lit_duration", c.lit_duration}};
}

inline void from_json(const json& j, MiniSun& c) {
    j.at("lit").get_to(c.lit);
    j.at("lit_duration").get_to(c.lit_duration);
}

inline void to_json(json& j, const Text& c) {
    j = json{
        {"type", "text"},
        {"position", c.position},
        {"size", c.size},
        {"text", c.text },
        {"color", c.color},
        {"layer", c.layer},
        {"centered", c.centered}
    };
}

inline void from_json(const json& j, Text& c) {
    j.at("position").get_to(c.position);
    j.at("size").get_to(c.size);
    j.at("text").get_to(c.text);
    j.at("color").get_to(c.color);
    j.at("layer").get_to(c.layer);
    j.at("centered").get_to(c.centered);
}

inline void to_json(json& j, const MenuItem& c) {
    (void)c;
    j = json{{"type", "menu_item"}};
}

inline void from_json(const json& j, MenuItem& c) {
    (void)j;
    (void)c;
}

inline void to_json(json& j, const Gravity& c) {
    (void)c;
    j = json{{"type", "gravity"}};
}

inline void from_json(const json& j, Gravity& c) {
    (void)j;
    (void)c;
}

inline void to_json(json& j, const DeleteData& c) {
    (void)c;
    j = json{{"type", "delete_data"}};
}

inline void from_json(const json& j, DeleteData& c) {
    (void)j;
    (void)c;
}

inline void to_json(json& j, const EndCutsceneCount& c) {
    j = json{{"type", "end_cutscene_count"}, {"position", c.position}, {"maxInclusive", c.maxInclusive}};
}

inline void from_json(const json& j, EndCutsceneCount& c) {
    j.at("position").get_to(c.position);
    j.at("maxInclusive").get_to(c.maxInclusive);
    c.lightCount = 0;
    c.sequence = 0;
}

inline void to_json(json& j, const AmbientLight& c) {
    j = json {
        {"type", "ambient_light"},
        {"color", c.color},
    };
}

inline void from_json(const json& j, AmbientLight& c) {
    j.at("color").get_to(c.color);
}

inline void to_json(json& j, const Setting& c) {
    j = json{{"type", "setting"}, {"setting", c.setting}, {"position_y", c.position_y}};
}

inline void from_json(const json& j, Setting& c) {
    j.at("setting").get_to(c.setting);
    j.at("position_y").get_to(c.position_y);
}
