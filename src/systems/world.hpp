#pragma once

// internal
#include "common.hpp"

// stlib
#include <random>
#include <vector>
#define SDL_MAIN_HANDLED
#include "registry.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

constexpr size_t LIGHT_SPAWN_DELAY_MS = 2000.f;
constexpr size_t DOUBLE_REFLECTION_TIMEOUT = 800.f;
constexpr size_t MAX_LIGHT_ON_SCREEN = 20;
#include "rails.hpp"
#include "render/render.hpp"
#include "systems/scenes.hpp"

// Container for all our entities and game logic. Individual rendering / update
// is deferred to the relative update() methods
class WorldSystem {
  public:
    WorldSystem();
    GLFWwindow* create_window();

    // Entrypoint to the game
    void init();

    // Releases all associated resources
    ~WorldSystem();

    // Steps the game ahead by ms milliseconds
    bool step(float elapsed_ms);

    // Check for collisions
    void handle_collisions();

    // Should the game be over ?
    bool is_over() const;

  private:
    // Input callback functions
    void on_key(int key, int, int action, int mod);
    void on_mouse_move(vec2 pos);
    void on_mouse_button(int key, int action, int mod, double xpos, double ypos);

    // Handle different collision cases
    void handle_reflection(Entity& reflective, Entity& reflected);
    void handle_non_reflection(Entity& collider, Entity& other);

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
    float current_speed;

    // Game systems
    RailSystem rails;

    // Music references
    Mix_Music* background_music;
    Mix_Chunk* reflection_sfx;
    Mix_Chunk* click_sfx;

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
