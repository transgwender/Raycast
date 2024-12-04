#pragma once

#include "common.hpp"
#include "texture.hpp"

// Main data relevant to the level
struct Scene {
    std::string scene_tag;
};

// A level is also a scene, but it may store additional information.
struct Level {
    int id = 1;
};

struct EndLevel {
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
    vec2 position;
    ZONE_TYPE type;
    // NOTE: endzones behave like blackholes in that they attract light that is within 
    // "force_field_radius" of it -- this is to make the levels ever so slightly easier
    float mass = 5.0;
    float force_field_radius = 15.0;
};

// Light source captures the characteristics of a source of light, such as the
// angle the light is shot at and the time interval between firing.
struct LightSource {
    float angle = 0;
};

// Mouse cursor
struct Mouse {};

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
enum class BOUNDS_TYPE { RADIAL = 0, RECTANGULAR = 1, POINT = 2, MESH = 3 };

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

struct MiniSun {
    bool lit = false;
    float lit_duration = 0;
    float light_level_percentage = 0;
};

struct LightUp {
    float counter_ms = LIGHT_TIMER_MS;
};


// Structure to store collision information
struct Collision {
    // NOTE: The first object is stored in the ECS container.entities.
    Entity other;        // The second object involved in the collision.
    int side = 0;        // side (1 for y, 2 for x) the collision occurrs on
    float overlap = 0.f; // amount the two objects overlap
    explicit Collision(Entity& other) { this->other = other; };
};

struct Highlightable {
    bool isHighlighted = false;
};

// Object is reflective
struct Reflective {};

struct ButtonFlag {}; // Indicates it's a button, menu

// Represents a transition to another scene.
struct ChangeScene {
    std::string scene;
};

struct ResumeGame {};

struct VolumeSlider {
    std::string setting;
};

struct Toggle {
    std::string setting;
};

struct DeleteData {
    bool isDoubleChecking = false;
};

struct Gravity {};

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
    int snap_segments = 5; // the number of discrete positions the mirror snaps to in both directions
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

// Represents a value that rounds to the nearest integer multiple of
// another value
template <typename T>
struct Snaps {
    T& value;
    T multiple;
};

// Represents entities that can be rotated by the user
struct Rotatable {
    float snap_angle = 0.2617993878;
};

enum MaterialType { TEXTURED };

enum Layer {
    BACKGROUND = 0,
    FOREGROUND = 1,
    PARTICLES = 2,
    WORLD_TEXT = 3,
    UI_BACKGROUND = 4,
    UI_FOREGROUND = 5,
    UI_TEXT = 6,
};

struct TextureMaterial {
    /** base texture name */
    std::string name;
    TextureHandle albedo = 0;
    TextureHandle normal = 0;
    float h_offset = 0;
    float v_offset = 0;
    vec2 cell_size = {1, 1};
};

enum BlendMode {
    ADDITIVE = 0,
    MULTIPLY = 1
};

/**
 * Material for a visible entity.
 *
 * Color channels are in range [0, 255].
 *
 * If the material type is TEXTURED, then the albedo and normal fields should be non-zero.
 */
struct Material {
    MaterialType type = TEXTURED;
    TextureMaterial texture {};
    vec4 color = vec4(255, 255, 255, 255);
    Layer layer = FOREGROUND;
    BlendMode blend_mode = MULTIPLY;
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
    unsigned int size;
    vec3 color = vec3(255, 255, 255);
    Layer layer = UI_TEXT;
    bool centered = false;
};

/**
 * A single particle. You probably don't want to instantiate this yourself, but rather use a `ParticleSpawner` instance.
 * Choice of parameters here and in `ParticleSpawner` based on Godot's CPUParticles2D.
 */
struct Particle {
    VirtualTextureHandle texture;
    vec2 position;
    vec4 color = {1, 1, 1, 1.0};
    vec2 scale = {24, 24};
    float angle = 0.f;
    vec2 direction = {0.0, 5.0};
    float speed = 10.0f;
    float damping = 0.0f;
    float spin_velocity = 0.0f;
    float scale_change = 0.0f;
    float alpha_fall_off = 0.0f;
    float lifetime = 1.0;
};

/** Object that spawns particles according to various parameters. */
struct ParticleSpawner {
    /** The texture ID of the texture that all spawned particles will take. */
    VirtualTextureHandle texture;
    /** The position of the spawner. */
    vec2 position;
    /** The initial speed of the spawned particles. */
    float initial_speed;
    /** Controls the rate in which the particle spins, in radians/sec */
    float spin_velocity;
    /**
     * The direction the spawner is facing. This will combine with the `spread` property to determine the initial
     * direction of the particles.
     */
    vec2 direction;
    /** The color of the particle. The texture will be tinted this color. Color AND alpha channels are in range [0, 1]. */
    vec4 color = vec4(1, 1, 1, 1);
    /** The angle range that the spawner sends particles in. */
    float spread;
    /** Controls the initial scale of the particles. */
    vec2 initial_scale;
    /** Controls how many units of scale the particle loses/gains per second. Zero for no change. */
    float scale_change;
    /** Controls how fast the linear velocity stops. */
    float damping = 0.0f;
    /** Controls how much the alpha channel of the texture's color changes per second. Keep in mind that alpha channel
     * values are in range [0, 1]*/
    float alpha_change = 0.0f;
    /** How long (in seconds) a spawned particle will live before being deleted. */
    float lifetime;
    /** Determines how long the spawner itself will be alive for, before its removed from the world. */
    float time_to_live = std::numeric_limits<float>::infinity();
    /** The max number of particles from this spawner that should exist at any given time (may be off +/- 1). */
    int max_particles;
    /** Make it so that all particles are released at once instead of gradually. */
    bool explosive = false;
    /** If explosive, this determines the interval in which the explosion happens. */
    float explosion_interval = 0.0f;
    /** If explosive, controls if the particles are equally spread out or sent out in a random angle. */
    bool uniform_explosion = false;

    float _cooldown = 0.0f;
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
    vec4 color = {255, 255, 255, 255};
};

struct Mirror {
    vec2 position = {0, 0};
    float angle = 0;
    std::string mirrorType;
    float railLength = 0;
    float railAngle = 0;
    int snap_segments = 5;
    float snap_angle = 0.2617993878;
};

struct ButtonHelper {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
    std::string label;
};

struct Blackhole {
    float mass;
    // NOTE: You may notice that we do not need to use the schwarzchild radius at all in our blackhole calculations
    // since it naturally shows up in the equations that are used. It is still included here to help with level design,
    // in particular all light rays at a distance of >= 2.6 schwartzchild radius from the blackhole will not get sucked
    // in (assuming speed of light >= 50).
    float schwarzchild_radius;
};

enum class DASH_STATES { BITE, STARE, WALK, IDLE, YAWN, HIDE, DASH_ACTIONS_COUNT };
struct DashTheTurtle {
    DASH_STATES behavior;
    bool tired = false;
    vec2 nearestLightRayDirection;
    vec2 originalPosition;
};

struct SpriteSheet {
    vec2 position = {0, 0};
    float sheetWidth;
    float sheetHeight;
    float cellWidth;
    float cellHeight;
    unsigned int currFrame = 0;
    unsigned int currState = 0;
    float timeElapsed = 0.f;
    bool animationActive = false;

    // Index -> different animation states
    // Value -> Number of frames in that animation
    std::vector<unsigned int> animationFrames;
};

struct Portal {
    Entity other_portal;
    vec2 position;
    float length = 45.f;

    // Angle of the portal itself.
    // The normal direction of projectile exits can be calculated using this angle.
    float angle;
};

enum class LEVER_STATES { LEFT, MIDDLE_1, MIDDLE_2, MIDDLE_3, MIDDLE_4, RIGHT, LEVER_STATES_COUNT };
enum class LEVER_MOVEMENT_STATES { STILL, PUSHED_RIGHT, PUSHED_LEFT, LEVER_MOVEMENT_STATES_COUNT };
enum class LEVER_EFFECTS { NONE, REMOVE, LEVER_EFFECTS_COUNT };

struct Lever {
    LEVER_STATES state;
    LEVER_MOVEMENT_STATES movementState;
    LEVER_EFFECTS effect;
    LEVER_STATES activeLever;
    Entity affectedEntity;
    float sfx_timer = 0.f;

    // lever_state -- is it pushed all the way right, all the way left, or somewhere in the middle?
    // lever_movement_states -- is the lever currently being pushed in a certain direction? If so, which direction?
    // lever_effect -- what effect does the lever have when active on the affected entity?
    // active_lever -- is the lever active when pushed right? or left?
    // affectedEntity -- which entity is affected by this lever?
    // pushed -- has this lever been pushed?
};

struct ColoredVertex {
    vec3 position;
    vec3 color;
};

struct Mesh {
    vec2 original_size = {1, 1};
    std::vector<ColoredVertex> vertices;
    std::vector<uint16_t> vertex_indices;
};

struct InOrbit {
    float prevAngle;
    float totalAngle = 0.0f;
    Entity bodyOfMass;
    Entity bodyOfMassJustOrbited;
};

struct EndCutsceneCount {
    int lightCount = 0;
    int sequence = 0;
    int maxInclusive;
    vec2 position;
};

struct AmbientLight {
    vec3 color = {255, 255, 255};
};

struct Setting {
    std::string setting;
    float position_y;
};

struct Invisible {

};