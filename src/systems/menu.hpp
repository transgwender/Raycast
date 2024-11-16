#pragma once

#include "persistence.hpp"
class MenuSystem {
  public:
    void init(PersistenceSystem *persistence_ptr);
    void generate_level_select_buttons(int levelCount);
    void generate_level_win_popup(int levelCurrent, int levelCount);
    void generate_pause_popup(int levelCurrent);
    bool try_close_menu();
    bool is_menu_open();
    void handle_button_press();

  private:
    PersistenceSystem* persistence;
};