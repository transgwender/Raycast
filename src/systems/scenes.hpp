#pragma once

// internal
#include "background.hpp"
#include "common.hpp"
#include "persistence.hpp"
#include "render.hpp"
#include <map>

class SceneSystem {
  public:
    void init(Entity &scene_state_entity, PersistenceSystem *persistence_ptr);

    // add entities
    bool try_parse_scene(std::string &scene_tag);

    size_t level_count() {
        return levels.size();
    }

    void reload_background(std::string& background_tag);

  private:
    BackgroundSystem background;
    PersistenceSystem *persistence;

    std::map<std::string, std::string> levels {
        // dynamically allocated
    };

    // Scenes of the game
    std::map<std::string, std::string> scene_paths {
        {"mainmenu", scene_path("mainmenu.json")},
        {"levelmenu", scene_path("levelmenu.json")},
        {"gamefinish", scene_path("gamefinish.json")},
        {"settings", scene_path("settings.json")},
    };
};
