#pragma once

// internal
#include "common.hpp"

// stlib
#include <random>
#include <vector>
#define SDL_MAIN_HANDLED
#include "registry.hpp"

// Systems
#include "persistence.hpp"
#include "background.hpp"
#include "menu.hpp"
#include "physics.hpp"
#include "scenes.hpp"
#include "sounds.hpp"
#include "utils/input_manager.hpp"

constexpr size_t LIGHT_SPAWN_DELAY_MS = 1000.f;
constexpr size_t DOUBLE_REFLECTION_TIMEOUT = 800.f;
constexpr size_t MAX_LIGHT_ON_SCREEN = 20;

// Container for all our entities and game logic. Individual rendering / update
// is deferred to the relative update() methods
class WorldSystem {
  public:
    WorldSystem();
    GLFWwindow* create_window();

    // Entrypoint to the game
    void init(PersistenceSystem *persistence_ptr);

    // Releases all associated resources
    ~WorldSystem();

    // Steps the game ahead by ms milliseconds
    bool step(float elapsed_ms);

    // Check for collisions
    void handle_collisions();

    void handle_minisun_collision(Entity& minisun_entity);

    // Should the game be over ?
    bool is_over() const;


  private:
    InputManager input_manager;
    // Input callback functions
    void on_key(int key, int, int action, int mod);
    void on_mouse_move(vec2 pos);
    std::vector<Entity> clicked_entities(double xpos, double ypos);
    void on_mouse_button(int key, int action, int mod, double xpos, double ypos);
    void on_resize(int width, int height);

    // Handle different collision cases
    void handle_reflection(Entity& reflective, Entity& reflected, int side, float overlap);
    void handle_non_reflection(Entity& collider, Entity& other);
    void handle_turtle_collisions(int i);
    void handle_portal_collisions(Entity& portal, Entity& light);

    void handle_end_cutscene_collision(Entity& end_cutscene_count_entity);

    // Restart level
    void restart_game();
    void change_scene(std::string &scene_tag);

    // OpenGL window handle
    GLFWwindow* window;

    // Time to fire
    float next_light_spawn;

    // Game state
    Entity scene_state_entity;
    SceneSystem scenes;
    MenuSystem menus;
    float current_speed;

    int dashSpeed = 30;
    float gravity = 2;

    // Music references
    SoundSystem sounds;

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    Entity frame_rate_entity;
    bool frame_rate_enabled = false;
    bool do_restart = false;

    bool shouldStep();
    bool shouldAllowInput();
    static bool isInLevel();

    void updateDash();
  
    PersistenceSystem *persistence;
};
