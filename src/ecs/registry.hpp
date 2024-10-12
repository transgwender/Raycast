#pragma once
#include <vector>

#include "components.hpp"
#include "ecs.hpp"

class ECSRegistry {
    // Callbacks to remove a particular or all entities in the system
    std::vector<ContainerInterface*> registry_list;

  public:
    // Manually created list of all components this game has
    ComponentContainer<Scene> scenes;
    ComponentContainer<Motion> motions;
    ComponentContainer<Collision> collisions;
    ComponentContainer<Mesh*> meshPtrs;
    ComponentContainer<RenderRequest> renderRequests;
    ComponentContainer<Interactable> interactables;
    ComponentContainer<ChangeScene> changeScenes;
    ComponentContainer<BoundingBox> boundingBoxes;
    ComponentContainer<Renderable> renderables;
    ComponentContainer<Zone> zones;
    ComponentContainer<LightSource> lightSources;
    ComponentContainer<Light> lightRays;
    ComponentContainer<Reflective> reflectives;
    ComponentContainer<Level> levels;

    // constructor that adds all containers for looping over them
    // IMPORTANT: Don't forget to add any newly added containers!
    ECSRegistry() {
        registry_list.push_back(&scenes);
        registry_list.push_back(&motions);
        registry_list.push_back(&collisions);
        registry_list.push_back(&meshPtrs);
        registry_list.push_back(&renderRequests);
        registry_list.push_back(&interactables);
        registry_list.push_back(&changeScenes);
        registry_list.push_back(&boundingBoxes);
        registry_list.push_back(&renderables);
        registry_list.push_back(&zones);
        registry_list.push_back(&lightSources);
        registry_list.push_back(&lightRays);
        registry_list.push_back(&reflectives);
        registry_list.push_back(&levels);
    }

    void clear_all_components() {
        for (ContainerInterface* reg : registry_list)
            if (reg != &scenes) {
                reg->clear();
            }
    }

    void list_all_components() const {
        printf("Debug info on all registry entries:\n");
        for (ContainerInterface* reg : registry_list)
            if (reg->size() > 0)
                printf("%4d components of type %s\n", (int)reg->size(),
                       typeid(*reg).name());
    }

    void list_all_components_of(Entity e) const {
        printf("Debug info on components of entity %u:\n", (unsigned int)e);
        for (ContainerInterface* reg : registry_list)
            if (reg->has(e))
                printf("type %s\n", typeid(*reg).name());
    }

    void remove_all_components_of(Entity e) {
        for (ContainerInterface* reg : registry_list)
            reg->remove(e);
    }
};

extern ECSRegistry registry;
