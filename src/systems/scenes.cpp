#include "scenes.hpp"

#include "components_json.hpp"
#include "json.hpp"
#include "registry.hpp"
#include "world_init.hpp"
#include "common.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void SceneSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;

//  https://www.cppstories.com/2019/04/dir-iterate/
    fs::path folder = scene_path("levels");
    for (const auto& entry : fs::directory_iterator(folder)) {
        auto filename = entry.path().filename().replace_extension();
        auto path = entry.path();
        levels.insert(std::make_pair<std::string, std::string>(filename.string(), path.string()));
    }

    scene_paths.insert(levels.begin(), levels.end());
}

// Attempts to parse a specified scene. Returns true if successful. False if
// not.
bool SceneSystem::try_parse_scene(std::string &scene_tag) {
    std::string filename = scene_paths.at(scene_tag);
    std::ifstream entity_file(filename);

    if (entity_file.is_open())
    {
        nlohmann::json j;
        entity_file >> j;

        entity_file.close();

        // Iterate through every entity specified, and add the component specified
        try {
            for(auto &array : j["objList"]) {
                const auto entity = Entity();
                for(auto &data : array["data"]) {
                    std::string type = data["type"];
                    if (type == "sprite") {
                        vec2 position = {data["position"][0], data["position"][1]};
                        TEXTURE_ASSET_ID texture = data["texture"];
                        createSprite(entity, renderer, position, texture);
                    } else if (type == "interactable") {
                        Interactable c{};
                        data.get_to(c);
                        registry.interactables.insert(entity, c);
                    } else if (type == "change_scene") {
                        ChangeScene c{};
                        data.get_to(c);
                        registry.changeScenes.insert(entity, c);
                    } else if (type == "bounding_box") {
                        BoundingBox c{};
                        data.get_to(c);
                        registry.boundingBoxes.insert(entity, c);
                    } else if (type == "zone") {
                        Zone c{};
                        data.get_to(c);
                        registry.zones.insert(entity, c);
                    } else if (type == "level") {
                        Level c{};
                        data.get_to(c);
                        registry.levels.insert(entity, c);
                    } else if (type == "light_source") {
                        LightSource c{};
                        data.get_to(c);
                        registry.lightSources.insert(entity, c);
                    }
                }
            }
        } catch (...) {
            std::cout << "ERROR: issue with the formatting of the file: " << filename << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "ERROR: failed to open file: " << filename << std::endl;
        return false;
    }
    return true;
}
