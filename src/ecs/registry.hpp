#pragma once
#include <vector>
#include "components.hpp"
#include "ecs.hpp"
#include "logging/log.hpp"

class ECSRegistry {
    // Callbacks to remove a particular or all entities in the system
    std::vector<ContainerInterface*> registry_list;

  public:
    // Manually created list of all components this game has
    ComponentContainer<Scene> scenes;
    ComponentContainer<Motion> motions;
    ComponentContainer<Collision> collisions;
    ComponentContainer<Interactable> interactables;
    ComponentContainer<ChangeScene> changeScenes;
    ComponentContainer<VolumeSlider> volumeSliders;
    ComponentContainer<Toggle> toggles;
    ComponentContainer<ResumeGame> resumeGames;
    ComponentContainer<Zone> zones;
    ComponentContainer<LightSource> lightSources;
    ComponentContainer<Light> lightRays;
    ComponentContainer<Material> materials;
    ComponentContainer<PointLight> pointLights;
    ComponentContainer<Reflective> reflectives;
    ComponentContainer<Level> levels;
    ComponentContainer<OnLinearRails> entitiesOnLinearRails;
    ComponentContainer<Lerpable> lerpables;
    ComponentContainer<Rotatable> rotatable;
    ComponentContainer<Highlightable> highlightables;
    ComponentContainer<Collider> colliders;
    ComponentContainer<Collideable> collideables;
    ComponentContainer<Menu> menus;
    ComponentContainer<MenuItem> menuItems;
    ComponentContainer<LevelSelect> levelSelects;
    ComponentContainer<DashTheTurtle> turtles;
    ComponentContainer<ButtonFlag> buttons;
    ComponentContainer<Text> texts;
    ComponentContainer<Mouse> mice;
    ComponentContainer<Blackhole> blackholes;
    ComponentContainer<SpriteSheet> spriteSheets;
    ComponentContainer<MiniSun> minisuns;
    ComponentContainer<Gravity> gravities;
    ComponentContainer<Lever> levers;
    ComponentContainer<Particle> particles;
    ComponentContainer<ParticleSpawner> particleSpawners;
    ComponentContainer<Mesh> meshes;
    ComponentContainer<LightUp> litEntities;
    ComponentContainer<DeleteData> deleteDatas;
    ComponentContainer<InOrbit> inOrbits;
    ComponentContainer<Portal> portals;
    ComponentContainer<EndLevel> endLevels;
    ComponentContainer<EndCutsceneCount> endCutsceneCounts;
    ComponentContainer<AmbientLight> ambientLights;
    ComponentContainer<Invisible> invisibles;

    // constructor that adds all containers for looping over them
    // IMPORTANT: Don't forget to add any newly added containers!
    ECSRegistry() {
        registry_list.push_back(&scenes);
        registry_list.push_back(&motions);
        registry_list.push_back(&collisions);
        registry_list.push_back(&interactables);
        registry_list.push_back(&changeScenes);
        registry_list.push_back(&volumeSliders);
        registry_list.push_back(&toggles);
        registry_list.push_back(&resumeGames);
        registry_list.push_back(&zones);
        registry_list.push_back(&lightSources);
        registry_list.push_back(&lightRays);
        registry_list.push_back(&materials);
        registry_list.push_back(&pointLights);
        registry_list.push_back(&reflectives);
        registry_list.push_back(&levels);
        registry_list.push_back(&entitiesOnLinearRails);
        registry_list.push_back(&lerpables);
        registry_list.push_back(&rotatable);
        registry_list.push_back(&highlightables);
        registry_list.push_back(&colliders);
        registry_list.push_back(&collideables);
        registry_list.push_back(&menus);
        registry_list.push_back(&menuItems);
        registry_list.push_back(&levelSelects);
        registry_list.push_back(&turtles);
        registry_list.push_back(&buttons);
        registry_list.push_back(&texts);
        registry_list.push_back(&mice);
        registry_list.push_back(&blackholes);
        registry_list.push_back(&spriteSheets);
        registry_list.push_back(&minisuns);
        registry_list.push_back(&gravities);
        registry_list.push_back(&levers);
        registry_list.push_back(&particles);
        registry_list.push_back(&particleSpawners);
        registry_list.push_back(&meshes);
        registry_list.push_back(&litEntities);
        registry_list.push_back(&deleteDatas);
        registry_list.push_back(&inOrbits);
        registry_list.push_back(&portals);
        registry_list.push_back(&endLevels);
        registry_list.push_back(&endCutsceneCounts);
        registry_list.push_back(&ambientLights);
        registry_list.push_back(&invisibles);
    }

    void clear_all_components() {
        for (ContainerInterface* reg : registry_list)
            if (reg != &scenes) {
                reg->clear();
            }
    }

    void list_all_components() const {
        LOG_INFO("Debug info on all registry entries:");
        for (ContainerInterface* reg : registry_list)
            if (reg->size() > 0)
                LOG_INFO("{} components of type {}", (int)reg->size(),
                       typeid(*reg).name());
    }

    void list_all_components_of(Entity e) const {
        LOG_INFO("Debug info on components of entity {}: ", (unsigned int)e);
        for (ContainerInterface* reg : registry_list)
            if (reg->has(e))
                LOG_INFO("type {}", typeid(*reg).name());
    }

    void remove_all_components_of(Entity e) {
        for (ContainerInterface* reg : registry_list)
            reg->remove(e);
    }
};

extern ECSRegistry registry;
