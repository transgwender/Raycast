#include "scenes.hpp"

#include "common.hpp"
#include "components_json.hpp"
#include "json.hpp"
#include "menu.hpp"
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

void SceneSystem::init(Entity &scene_state_entity, PersistenceSystem *persistence_ptr) {
    persistence = persistence_ptr;

    Scene& scene = registry.scenes.emplace(scene_state_entity);
    scene.scene_tag = "mainmenu";

    //  https://www.cppstories.com/2019/04/dir-iterate/
    fs::path folder = scene_path("levels");
    for (const auto& entry : fs::directory_iterator(folder)) {
        const auto& path = entry.path();
        auto filename = entry.path().filename().replace_extension();
        levels.insert(std::make_pair<std::string, std::string>(filename.string(), path.string()));
    }

    scene_paths.insert(levels.begin(), levels.end());

    folder = scene_path("test");
    for (const auto& entry : fs::directory_iterator(folder)) {
        const auto& path = entry.path();
        auto filename = entry.path().filename().replace_extension();
        scene_paths.insert(std::make_pair<std::string, std::string>(filename.string(), path.string()));
    }

    folder = scene_path("end");
    for (const auto& entry : fs::directory_iterator(folder)) {
        const auto& path = entry.path();
        auto filename = entry.path().filename().replace_extension();
        scene_paths.insert(std::make_pair<std::string, std::string>(filename.string(), path.string()));
    }

    background.init();
}

// Attempts to parse a specified scene. Returns true if successful. False if
// not.
bool SceneSystem::try_parse_scene(std::string& scene_tag) {
    std::string filename = scene_paths.at(scene_tag);
    std::ifstream entity_file(filename);

    if (entity_file.is_open()) {
        nlohmann::json j;
        entity_file >> j;

        entity_file.close();

        // Iterate through every entity specified, and add the component
        // specified
        try {
            for (auto& array : j["objList"]) {
                const auto entity = Entity();
                for (auto& data : array["data"]) {
                    std::string type = data["type"];
                    if (type == "sprite") {
                        Sprite c{};
                        data.get_to(c);
                        createSprite(entity, c.position, c.scale, c.angle, c.texture, FOREGROUND, c.color);
                    } else if (type == "change_scene") {
                        PARSE_COMPONENT(ChangeScene, changeScenes);
                    } else if (type == "zone") {
                        Zone c{};
                        data.get_to(c);
                        setZone(entity, c.type, c.position);
                    } else if (type == "light_source") {
                        PARSE_COMPONENT(LightSource, lightSources);
                    } else if (type == "lerpable") {
                        PARSE_COMPONENT(Lerpable, lerpables);
                    } else if (type == "reflective") {
                        PARSE_COMPONENT(Reflective, reflectives);
                    } else if (type == "level") {
                        PARSE_COMPONENT(Level, levels);
                    } else if (type == "mirror") {
                        Mirror c{};
                        data.get_to(c);
                        createMirror(entity, c);
                    } else if (type == "highlightable") {
                        PARSE_COMPONENT(Highlightable, highlightables);
                    } else if (type == "level_select") {
                        PARSE_COMPONENT(LevelSelect, levelSelects);
                    } else if (type == "dash_the_turtle") {
                        //createDashTheTurtle(entity, data["position"]);
                        PARSE_COMPONENT(DashTheTurtle, turtles);
                        // int x = registry.motions.has(entity);
                        // printf("Your boolean variable is: %s\n", x ? "true" : "false");
                    } else if (type == "button") {
                        ButtonHelper c{};
                        data.get_to(c);
                        createEmptyButton(entity, c.position, c.scale, c.label);
                    } else if (type == "collider") {
                        PARSE_COMPONENT(Collider, colliders);
                    } else if (type == "collideable") {
                        PARSE_COMPONENT(Collideable, collideables);
                    } else if (type == "interactable") {
                        PARSE_COMPONENT(Interactable, interactables);
                    } else if (type == "blackhole") {
                        PARSE_COMPONENT(Blackhole, blackholes);
                        // the blackhole is itself a point light source as well
                        PointLight& point_light = registry.pointLights.emplace(entity);
                        point_light.diffuse = 6.0f * vec3(255, 233, 87);
                        point_light.linear = -0.25f; // the render magic lies in this value
                        point_light.quadratic = 0.01;
                    } else if (type == "sprite_sheet") {
                        SpriteSheet ss{};
                        data.get_to(ss);
                        createSpriteSheet(entity, ss.position, ss.sheetWidth, ss.sheetHeight, ss.cellWidth,
                                          ss.cellHeight, ss.animationFrames, data["texture"], data["imageWidth"],
                                          data["imageHeight"]);
                    } else if (type == "minisun") {
                        PARSE_COMPONENT(MiniSun, minisuns);
                    } else if (type == "gravity") {
                        PARSE_COMPONENT(Gravity, gravities);
                    } else if (type == "menu_item") {
                        PARSE_COMPONENT(MenuItem, menuItems);
                    } else if (type == "text") {
                        PARSE_COMPONENT(Text, texts);
                    } else if (type == "lever") { // This is used to attach a lever entity that can exhibit some effect on the CURRENT ENTITY
                        createLever(entity, data["position"], data["state"], data["effect"], data["activeLever"]);
                    } else if (type == "mesh") {
                        initMesh(entity, data["path"], data["position"], data["angle"], data["scale"]);
                    } else if (type == "delete_data") {
                        PARSE_COMPONENT(DeleteData, deleteDatas);
                    } else if (type == "background") {
                        std::string background_tag;
                        data["id"].get_to(background_tag);
                        background.try_parse_background(background_tag);
                    } else if (type == "portal_pair") {
                        createPortals(data["portal_position"], data["portal_angle"], data["other_portal_position"], data["other_portal_angle"]);
                    } else if (type == "end_level") {
                        PARSE_COMPONENT(EndLevel, endLevels);
                    } else if (type == "end_cutscene_count") {
                        EndCutsceneCount end{};
                        data.get_to(end);
                        registry.endCutsceneCounts.insert(entity, end);
                        Motion motion{};
                        motion.position = end.position;
                        motion.scale = {3, 3};
                        registry.motions.insert(entity, motion);
                    } else if (type == "ambient_light") {
                        PARSE_COMPONENT(AmbientLight, ambientLights);
                    } else if (type == "setting") {
                        Setting setting{};
                        data.get_to(setting);
                        if (setting.setting == "music") {
                            registry.texts.insert(entity, {"Music Volume", {120, setting.position_y}, 36, vec4(255.0), UI_TEXT, true});
                            int index = floor(persistence->get_settings_music_volume() * 26);
                            if (index > 25) index = 25;
                            std::string slider_texture = "slider" + std::to_string(index);
                            createEmptyButton(entity, {200, setting.position_y}, {52, 10}, "", slider_texture);
                            registry.volumeSliders.insert(entity, {setting.setting});
                        } else if (setting.setting == "sfx") {
                            registry.texts.insert(entity, {"SFX Volume", {120, setting.position_y}, 36, vec4(255.0), UI_TEXT, true});
                            int index = floor(persistence->get_settings_sfx_volume() * 26);
                            if (index > 25) index = 25;
                            std::string slider_texture = "slider" + std::to_string(index);
                            createEmptyButton(entity, {200, setting.position_y}, {52, 10}, "", slider_texture);
                            registry.volumeSliders.insert(entity, {setting.setting});
                        } else if (setting.setting == "hard") {
                            registry.texts.insert(entity, {"Hard Mode", {120, setting.position_y}, 36, vec4(255.0), UI_TEXT, true});
                            int index = persistence->get_settings_hard_mode();
                            std::string toggle_texture = "toggle" + std::to_string(index);
                            createEmptyButton(entity, {200, setting.position_y}, {10, 10}, "", toggle_texture);
                            registry.toggles.insert(entity, {setting.setting});
                        }
                    }
                }
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

    LOG_INFO("Successfully loaded scene\n");
    return true;
}

void SceneSystem::reload_background(std::string& background_tag) {
    background.clear_background();
    background.try_parse_background(background_tag);
}
