#pragma once

// internal
#include "common.hpp"
#include "render.hpp"
#include <map>

class SceneSystem {
  public:
    void init(RenderSystem* renderer_arg);

    // add entities
    bool try_parse_scene(std::string &scene_tag);

  private:
    RenderSystem* renderer;

    std::map<std::string, std::string> levels {
        // dynamically allocated
    };

    // Scenes of the game
    std::map<std::string, std::string> scene_paths {
        {"test", scene_path("test.json")},
        {"mainmenu", scene_path("mainmenu.json")},
        {"levelmenu.json", scene_path("levelmenu.json")}
    };

};