#pragma once

// internal
#include "common.hpp"

// stlib
#include <random>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "systems/render.hpp"

// Container for all our entities and game logic. Individual rendering / update
// is deferred to the relative update() methods
class WorldSystem {
  public:
    WorldSystem();

    // Creates a window
    GLFWwindow* create_window();

    // starts the game
    void init(RenderSystem* renderer);

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

    // restart level
    void restart_game();

    // add entities
    bool try_parse_scene(SCENE_ASSET_ID scene);

    // OpenGL window handle
    GLFWwindow* window;

    // Game state
    Entity scene_state_entity;
    RenderSystem* renderer;
    float current_speed;

    // music references
    Mix_Music* background_music;

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    // Scenes of the game
    const std::array<std::string, scene_count> scene_paths = {
        scene_path("test.json"),
        scene_path("mainmenu.json"),
        scene_path("level1.json")
    };
};
