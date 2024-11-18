#include "menu.hpp"
#include "ecs.hpp"
#include "mesh_utils.hpp"
#include "registry.hpp"
#include "scenes.hpp"
#include "world_init.hpp"

void MenuSystem::generate_level_select_buttons(int levelCount) {
    int columnMax = 9;
    int left = native_width/2 - 3*40;
    int top = 40;

    for (int i = 1; i <= levelCount; ++i) {
        Entity level = Entity();
        std::string label = std::to_string(i);
        std::string tag = "level" + std::to_string(i);
        int xPos = left + 30*((i-1)%columnMax);
        int yPos = top + 30*((i-1)/columnMax);
        createChangeSceneButton(level, vec2(xPos, yPos), vec2(20, 20), label, "button_square", tag, {0, 0, 0});
        registry.menuItems.emplace(level);
        if (!persistence->get_is_accessible(i)) {
            registry.interactables.remove(level);
        }
        if (persistence->get_is_locked(i)) {
            createSprite(Entity(), vec2(xPos, yPos + 10), vec2(30, 30), 0, "lock");
        }
        if (persistence->get_is_beaten(i)) {
            createSprite(Entity(), vec2(xPos, yPos), vec2(30, 30), 0, "star");
        }
    }

    Menu& menu = registry.menus.emplace(Entity()); // Menu Active
    menu.canClose = false;
    menu.shouldBlockSteps = false;
    menu.shouldBlockInput = false;
}

void MenuSystem::generate_level_win_popup(int levelCurrent, int levelCount) {
    // restart, main menu, next
    // spawn sprite background
    // indicate that menu is up
    // spawn restart
    // spawn main menu
    // spawn next if there
    int xPos = native_width/2;
    int yPos = native_height/2;

    if (levelCurrent < levelCount) {
        Entity popup_background = Entity();
        createSprite(popup_background, vec2(xPos, yPos), vec2(200, 70), 0, "popup_win", UI_BACKGROUND);
        registry.menuItems.emplace(popup_background);

        Entity restart_button = Entity();
        createChangeSceneButton(restart_button, vec2(xPos - 60, yPos), vec2(50, 50), "Restart", "button_square", "level" + std::to_string(levelCurrent), {0, 0, 0});
        registry.menuItems.emplace(restart_button);

        Entity main_menu_button = Entity();
        createChangeSceneButton(main_menu_button, vec2(xPos, yPos), vec2(50, 50), "Main Menu", "button_square", "mainmenu", {0, 0, 0});
        registry.menuItems.emplace(main_menu_button);

        Entity next_button = Entity();
        createChangeSceneButton(next_button, vec2(xPos + 60, yPos), vec2(50, 50), "Continue", "button_square", "level" + std::to_string(levelCurrent+1), {0, 0, 0});
        registry.menuItems.emplace(next_button);
    } else {
        Entity popup_background = Entity();
        createSprite(popup_background, vec2(xPos, yPos), vec2(130, 70), 0, "popup_win", UI_BACKGROUND);
        registry.menuItems.emplace(popup_background);

        Entity restart_button = Entity();
        createChangeSceneButton(restart_button, vec2(xPos - 30, yPos), vec2(50, 50), "Restart", "button_square", "level" + std::to_string(levelCurrent), {0, 0, 0});
        registry.menuItems.emplace(restart_button);

        Entity main_menu_button = Entity();
        createChangeSceneButton(main_menu_button, vec2(xPos + 30, yPos), vec2(50, 50), "Main Menu", "button_square", "mainmenu", {0, 0, 0});
        registry.menuItems.emplace(main_menu_button);
    }

    Menu& menu = registry.menus.emplace(Entity()); // Menu Active
    menu.canClose = false;
    menu.shouldBlockSteps = false;
    menu.shouldBlockInput = true;
}

void MenuSystem::generate_pause_popup(int levelCurrent) {

    int xPos = native_width/2;
    int yPos = native_height/2;

    Entity popup_background = Entity();
    createSprite(popup_background, vec2(xPos, yPos), vec2(80, 115), 0, "popup_pause", UI_BACKGROUND);
    registry.menuItems.emplace(popup_background);

    LOG_INFO("Pausing");

    Entity restart_button = Entity();
    createChangeSceneButton(restart_button, vec2(xPos, yPos-30), vec2(50, 25), "Restart", "button", "level" + std::to_string(levelCurrent));
    registry.menuItems.emplace(restart_button);

    Entity main_menu_button = Entity();
    createChangeSceneButton(main_menu_button, vec2(xPos, yPos), vec2(50, 25), "Main Menu", "button", "mainmenu");
    registry.menuItems.emplace(main_menu_button);

    Entity resume_button = Entity();
    createResumeButton(resume_button, vec2(xPos, yPos+30), vec2(50, 25), "Resume",  "button");
    registry.menuItems.emplace(resume_button);

    Menu& menu = registry.menus.emplace(Entity()); // Menu Active
    menu.canClose = true;
    menu.shouldBlockSteps = true;
    menu.shouldBlockInput = true;
}

bool MenuSystem::is_menu_open() {
    assert(registry.menus.size() <= 1);
    return !registry.menus.components.empty();
}

bool MenuSystem::try_close_menu() {
    if(is_menu_open()) {
        if (registry.menus.components.front().canClose) {
            for (int i = registry.menuItems.entities.size()-1; i >= 0; i--) {
                registry.remove_all_components_of(registry.menuItems.entities[i]);
            }
            registry.menuItems.clear();
            registry.menus.clear();
            LOG_INFO("Closing Menu");
            return true;
        }
    }
    return false;
}

void MenuSystem::init(PersistenceSystem *persistence_ptr) {
    this->persistence = persistence_ptr;
}
