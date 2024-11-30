#include "background.hpp"
#include "ecs.hpp"
#include "json.hpp"
#include "registry.hpp"
#include "world_init.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void BackgroundSystem::init() {
    //  https://www.cppstories.com/2019/04/dir-iterate/
    fs::path folder = background_path("");
    for (const auto& entry : fs::directory_iterator(folder)) {
        const auto& path = entry.path();
        auto filename = entry.path().filename().replace_extension();
        backgrounds.insert(std::make_pair<std::string, std::string>(filename.string(), path.string()));
    }
}

// Attempts to parse a specified scene. Returns true if successful. False if
// not.
bool BackgroundSystem::try_parse_background(std::string& background_tag) {
    background_entities.clear();

    if (background_tag.empty()) return true; // No background, deemed a success
    std::string filename = backgrounds.at(background_tag);
    std::ifstream background_file(filename);

    if (background_file.is_open()) {
        nlohmann::json j;
        background_file >> j;

        background_file.close();

        // Iterate through every entity specified, and add the component
        // specified
        try {
            std::string main_background;
            j["background"].get_to(main_background);
            Entity background = Entity();
            createSprite(background, vec2(native_width/2, native_height/2), vec2(native_width, native_height), 0, main_background, BACKGROUND);
            background_entities.push_back(background);
            for (auto& data : j["extra"]) {
                vec2 position;
                vec2 scale;
                std::string texture;
                data["position"][0].get_to(position.x);
                data["position"][1].get_to(position.y);
                data["scale"][0].get_to(scale.x);
                data["scale"][1].get_to(scale.y);
                data["texture"].get_to(texture);
                Entity sprite = Entity();
                createSprite(sprite, position, scale, 0, texture, FOREGROUND);
                background_entities.push_back(sprite);
            }
            // catches all errors deriving from the standard exception class, this is better than a catch(...) statement since it gives
            // us information about the error via e.what()
        } catch (const std::exception& e) {
            LOG_ERROR("Parsing file {} failed with error: {}\n", filename, e.what());
            return false;

            // catches any other error -- practically this case should never be reached since its good practice to have all errors extend the std::exception class
        } catch (...) {
            LOG_ERROR("Parsing file {} failed with an unknown error, check formatting\n", filename);
            return false;
        }
    }
    else {
        LOG_ERROR("Failed to open file: {}\n", filename);
    }

    LOG_INFO("Successfully loaded backgrounds\n");
    return true;
}

void BackgroundSystem::clear_background() {
    for (auto &it : background_entities) {
        registry.remove_all_components_of(it);
    }
    background_entities.clear();
}
