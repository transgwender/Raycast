#pragma once

#include <unordered_map>
#include <utility>
#include "ecs/ecs.hpp"
#include "common.hpp"
#include "ecs/registry.hpp"

/// Some helpful macros to check key event
/// NOTE: assumes action, mod, and key identifiers are in scope
#define IS_PRESSED(__key) (action == GLFW_PRESS && key == __key)
#define IS_RELEASED(__key) (action == GLFW_RELEASE && key == __key)
#define IS_REPEATED(__key) (action == GLFW_REPEAT && key == __key)
#define IS_PRESSED_WITH_SHIFT(__key) (action == GLFW_PRESS && (mod & GLFW_MOD_SHIFT) && key == __key)
#define IS_RELEASED_WITH_SHIFT(__key) (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == __key)

/// @brief Keeps track of the current state of peripherals (in our case, just the keyboard and mouse).
class InputManager {
    public:
        /// Interface to query and udpate state of keyboard
        void update_key_state(int key, int action, int mod);
        bool is_pressed(int key);
        bool is_released(int key);
        bool is_repeated(int key);

        /// Interface to query and update state of mouse
        void update_mouse_button_state(int key, int action, int mod, vec2 pos);
        void update_mouse_position(vec2 pos);
        void update_mouse_to_entity_displacement(vec2 pos);
        bool is_mouse_button_pressed(int mouse_button);
        bool is_mouse_button_released(int mouse_button);

        vec2 displacement_to_entity();

        /// Additional helpers to filter entities/components based on input state
        std::vector<Entity> get_entities_at_mouse_pos();

        /// Entities currently clicked
        std::vector<Entity> active_entities;

    private:
        /// Internal data structures to keep track of input state
        std::unordered_map<int, int> key_states;
        std::pair<int, int> mouse_button_states;
        int mod;
        vec2 mouse_position;
        vec2 displacement_from_entity;
        Entity* selected_entity;
};
