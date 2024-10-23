#include "menu.hpp"
#include "ecs.hpp"
#include "registry.hpp"
#include "scenes.hpp"
#include "world_init.hpp"

void MenuSystem::generate_level_select_buttons(int levelCount) {
    int columnMax = 6;
    int left = native_width/2 - 3*40;
    int top = 40;

    for (int i = 1; i <= levelCount; ++i) {
        Entity level = Entity();
        std::string label = "Level " + std::to_string(i);
        std::string tag = "level" + std::to_string(i);
        int xPos = left + 40*((i-1)%columnMax);
        int yPos = top+ 40*((i-1)/columnMax);
        createChangeSceneButton(level, vec2(xPos, yPos), vec2(20, 20), label, tag);
        registry.menuItems.emplace(level);
    }

    registry.menus.emplace(Entity()); // Menu Active
}

void MenuSystem::generate_level_win_popup(int levelCurrent, int levelCount) {
    // restart, main menu, next
    // spawn sprite background
    // indicate that menu is up
    // spawn restart
    // spawn main menu
    // spawn next if there
    int xPos = native_width/2;

    Entity popup_background = Entity();
    createSprite(popup_background, vec2(xPos, 100), vec2(200, 100), 0, "button_rectangle_depth_border");
    registry.menuItems.emplace(popup_background);

    if (levelCurrent < levelCount) {
        Entity restart_button = Entity();
        createChangeSceneButton(restart_button, vec2(xPos - 60, 100), vec2(50, 50), "Restart", "level" + std::to_string(levelCurrent));
        registry.menuItems.emplace(restart_button);

        Entity main_menu_button = Entity();
        createChangeSceneButton(main_menu_button, vec2(xPos, 100), vec2(50, 50), "Main Menu", "mainmenu");
        registry.menuItems.emplace(main_menu_button);

        Entity next_button = Entity();
        createChangeSceneButton(next_button, vec2(xPos + 60, 100), vec2(50, 50), "Continue", "level" + std::to_string(levelCurrent+1));
        registry.menuItems.emplace(next_button);
    } else {
        Entity restart_button = Entity();
        createChangeSceneButton(restart_button, vec2(xPos - 30, 100), vec2(50, 50), "Restart", "level" + std::to_string(levelCurrent));
        registry.menuItems.emplace(restart_button);

        Entity main_menu_button = Entity();
        createChangeSceneButton(main_menu_button, vec2(xPos + 30, 100), vec2(50, 50), "Main Menu", "mainmenu");
        registry.menuItems.emplace(main_menu_button);
    }

    registry.menus.emplace(Entity()); // Menu Active
}

bool MenuSystem::is_menu_open() {
    assert(registry.menus.size() <= 1);
    return !registry.menus.components.empty();
}

bool MenuSystem::try_close_menu() {
    if(is_menu_open()) {
        if (registry.menus.components.front().canClose) {
            for (auto i = registry.menuItems.entities.rbegin(); i != registry.menuItems.entities.rend(); ++i) {
                registry.remove_all_components_of(*i);
            }
            registry.menus.clear();
        }
    }
    return false;
}
