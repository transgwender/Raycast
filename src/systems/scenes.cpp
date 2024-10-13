#include "scenes.hpp"

#include "components_json.hpp"
#include "json.hpp"
#include "registry.hpp"
#include "world_init.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

// NOTE: Expects the `data`, `entity`, and `registry` identifiers to be in scope.
#define PARSE_COMPONENT(ty, container)                                         \
ty __ty{};                                                                     \
data.get_to(__ty);                                                             \
(registry).container.insert(entity, __ty);

namespace fs = std::filesystem;

void SceneSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;

//  https://www.cppstories.com/2019/04/dir-iterate/
    fs::path folder = scene_path("levels");
    for (const auto& entry : fs::directory_iterator(folder)) {
        const auto& path = entry.path();
        auto filename = entry.path().filename().replace_extension();
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
                        Sprite c{};
                        data.get_to(c);
                        createSprite(entity, renderer, c.position, c.scale, c.angle, c.texture);
                    } else if (type == "interactable") {
                        PARSE_COMPONENT(Interactable, interactables);
                    } else if (type == "change_scene") {
                        PARSE_COMPONENT(ChangeScene, changeScenes);
                    } else if (type == "bounding_box") {
                        PARSE_COMPONENT(BoundingBox, boundingBoxes);
                    } else if (type == "zone") {
                        PARSE_COMPONENT(Zone, zones);
                    } else if (type == "light_source") {
                        PARSE_COMPONENT(LightSource, lightSources);
                    } else if (type == "on_linear_rails") {
                        PARSE_COMPONENT(OnLinearRails, entitiesOnLinearRails);
                    } else if (type == "linearly_interpolatable") {
                        PARSE_COMPONENT(LinearlyInterpolatable, linearlyInterpolatables);
                    } else if (type == "rotateable") {
                        PARSE_COMPONENT(Rotateable, rotateables);
                    } else if (type == "reflective") {
                        PARSE_COMPONENT(Reflective, reflectives);
                    } else if (type == "level") {
                        PARSE_COMPONENT(Level, levels);
                    } else if (type == "mirror") {
                        Mirror c{};
                        data.get_to(c);
                        createMirror(entity, renderer, c.position, c.angle);
                    } else if (type == "highlightable") {
                        PARSE_COMPONENT(Highlightable, highlightables);
                    
                    }
                }
            }
        } catch (...) {
            LOG_ERROR("Parsing file failed for file; {}", filename);
            return false;
        }
    }
    else
    {
        LOG_ERROR("Failed to open file: {}", filename);
        return false;
    }

    LOG_INFO("Successfully loaded scene")
    return true;
}
