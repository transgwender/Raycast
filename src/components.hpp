#pragma once

#include "common.hpp"

// Main data relevant to the level
struct Scene {
    std::string scene_tag;
};

// A level is also a scene, but it may store additional information.
struct Level {
    int id = 1;
};

// Scene should generate buttons
struct LevelSelect {};

// Is a menu currently open?
struct Menu {
    bool canClose = false;
    bool shouldBlockSteps = false;
    bool shouldBlockInput = false;
};

struct MenuItem {};

////////////////////////////////////////////////////////////////
///
/// Component Definitions
///
////////////////////////////////////////////////////////////////

// A zone represents the start and end points of light.
enum class ZONE_TYPE { START = 0, END = 1, ZONE_TYPE_COUNT };
struct Zone {
    vec2 position = {0, 0};
    ZONE_TYPE type;
};

// Entities that are `renderable` are visible in scenes.
struct Renderable {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
    float angle = 0;
};

// Light source captures the characteristics of a source of light, such as the
// angle the light is shot at and the time interval between firing.
struct LightSource {
    float angle = 0;
};

// Mouse cursor
struct Mouse {

};

struct Light {
    Entity last_reflected;
    float last_reflected_timeout;
};

// All data relevant to the shape and motion of entities
struct Motion {
    vec2 position = {0, 0};
    float angle = 0;
    vec2 velocity = {0, 0};
    vec2 scale = {10, 10};
};

// Represents the different types of bounding boxes
enum class BOUNDS_TYPE { RADIAL = 0, RECTANGULAR = 1, POINT = 2 };

// Represents an object that is able to collide with a specific type of bounding box
struct Collider {
    BOUNDS_TYPE bounds_type = BOUNDS_TYPE::RADIAL;
    BOUNDS_TYPE user_interaction_bounds_type = BOUNDS_TYPE::RECTANGULAR;
    std::array<vec2, 4> rotated_bounds = {};
    bool needs_update = true;
    float width = 1.f;
    float height = 1.f;
    float angle = 0.f;
};

// Actively collideable
struct Collideable {};

// Actively interactable
struct Interactable {};

// Structure to store collision information
struct Collision {
    // NOTE: The first object is stored in the ECS container.entities.
    Entity other; // The second object involved in the collision.
    int side = 0; // side (1 for y, 2 for x) the collision occurrs on
    explicit Collision(Entity& other) { this->other = other; };
};

struct Highlightable {
    bool isHighlighted = false;
};

// Object is reflective
struct Reflective {};

struct Button {}; // Indicates it's a button, menu

// Represents a transition to another scene.
struct ChangeScene {
    std::string scene;
};

struct ResumeGame {};

/**
 * Single Vertex Buffer element for textured sprites (textured.vs.glsl)
 */
struct TexturedVertex {
    vec3 position;
    vec2 texcoord;
};

/**
 * Represents a material for a renderable entity.
 * `texture` is the name of the base texture found in the textures folder.
 * The normal map texture can then be derived from the base texture name by
 * adding "_n" to the end of the base texture name.
 */
// Entities that are mounted on linear rails can move along a straight line.
struct OnLinearRails {
    float angle = 0;
    float length = 100; // This is really the "half" length.
    vec2 firstEndpoint = {0, 0};
    vec2 secondEndpoint = {0, 0};
    vec2 direction = {0, 0};
};

// Entities that are linearly interpolatable can make use of linear
// interpolation to perform smoothening of animations.
struct Lerpable {
    float t;
    bool should_switch_direction;
    float t_step = 0.01;
};

// Represents entities that can be rotated by the user
struct Rotateable {};

struct Material {
    std::string texture;
    std::string shader = "textured";
};

/**
 * Represents a point light in space. An entity with this component should also
 * have a position. Uses a quadratic light attenuation equation, see
 * https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
 * Control the colour of the light using the diffuse property, its colour
 * channels should be in the range [0, 255].
 */
struct PointLight {
    vec3 diffuse;
    float linear;
    float quadratic;
    float constant = 1.0;
};

struct Text {
    std::string text;
    vec2 position;
    int fontSize;
};

/**
 * This is used more as a general purpose helper for constructing more complex objects like sprites.
 * Not a component in its own right.
 * TODO: Move to its own file
 */
struct Sprite {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
    float angle = 0;
    std::string texture;
};

struct Mirror {
    vec2 position = {0, 0};
    float angle = 0;
};


struct ButtonHelper {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
    std::string label;
};

enum class DASH_STATES { WALK = 0, IDLE = 1, STARE = 2, DASH_ACTIONS_COUNT };
struct DashTheTurtle {
    DASH_STATES behavior;
    vec2 nearestLightRayDirection;
};