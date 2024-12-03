#include "utils/input_manager.hpp"
#include "utils/collisions.h"
#include "systems/render/util.hpp"

/// Interface to query and udpate state of keyboard
void InputManager::update_key_state(int key, int action, int mod) {
    key_states[key] = action;
    this->mod = mod;
}

bool InputManager::is_pressed(int key) {
    if (key_states.find(key) != key_states.end()) {
        return key_states[key] == GLFW_PRESS;
    }
    return false;
}

bool InputManager::is_released(int key) {
    if (key_states.find(key) != key_states.end()) {
        return key_states[key] == GLFW_RELEASE;
    }
    return false;
}

bool InputManager::is_repeated(int key) {
    if (key_states.find(key) != key_states.end()) {
        return key_states[key] == GLFW_REPEAT;
    }
    return false;
}

/// Interface to query and update state of mouse
void InputManager::update_mouse_button_state(int key, int action, int mod, vec2 pos) {
    switch(key) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mouse_button_states.first = action;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mouse_button_states.second = action;
            break;
        default:
            LOG_WARN("Unidentified mouse button\n");
    }
    this->mod = mod;
    this->mouse_position = pos;
}

void InputManager::update_mouse_position(vec2 pos) {
    this->mouse_position = pos;
}

void InputManager::update_mouse_to_entity_displacement(vec2 pos) {
        displacement_from_entity = screenToWorld(this->mouse_position) - pos;
}

bool InputManager::is_mouse_button_pressed(int mouse_button) {
    switch(mouse_button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return mouse_button_states.first == GLFW_PRESS;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return mouse_button_states.second == GLFW_PRESS;
        default:
            LOG_WARN("Unidentified mouse button\n");
            return false;
    }
}

bool InputManager::is_mouse_button_released(int mouse_button) {
    switch (mouse_button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        return mouse_button_states.first == GLFW_RELEASE;
    case GLFW_MOUSE_BUTTON_RIGHT:
        return mouse_button_states.second == GLFW_RELEASE;
    default:
        LOG_WARN("Unidentified mouse button\n");
        return false;
    }
}

// get the relative displacement of the mouse since it was pressed down.
vec2 InputManager::displacement_to_entity() {
    return displacement_from_entity;
}

/// Additional helpers to filter entities/components based on input state
std::vector<Entity> InputManager::get_entities_at_mouse_pos() {
    if (registry.mice.size() == 0) {
        const auto mouse = Entity();
        registry.mice.emplace(mouse);
        registry.motions.emplace(mouse);
        Collider& mouse_collider = registry.colliders.emplace(mouse);
        mouse_collider.bounds_type = BOUNDS_TYPE::POINT;
        mouse_collider.needs_update = false;
        mouse_collider.height = 0.5f;
        mouse_collider.width = 0.5f;
    }
    std::vector<Entity> entities;

    Entity& mouse = registry.mice.entities[0];
    Motion& mouse_motion = registry.motions.get(mouse);
    mouse_motion.position = screenToWorld(mouse_position);
    for (int i = 0; i < registry.interactables.size(); i++) {
        if (Collisions::overlap(mouse, registry.interactables.entities[i], true).x == 1)
            if (!registry.invisibles.has(registry.interactables.entities[i])) {
                entities.push_back(registry.interactables.entities[i]);           
            }
    }
    return entities;
}
