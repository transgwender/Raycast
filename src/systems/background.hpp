#pragma once

// internal
#include "common.hpp"
#include "render.hpp"
#include <map>

class BackgroundSystem {
  public:
    void init();
    bool try_parse_background(std::string& background_tag);
    void clear_background();

  private:
    std::vector<Entity> background_entities;

    std::map<std::string, std::string> backgrounds {
        // dynamically allocated
    };
};