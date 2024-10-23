#pragma once

class MenuSystem {
  public:
    void generate_level_select_buttons(int levelCount);
    void generate_level_win_popup(int levelCurrent, int levelCount);
    bool try_close_menu();
    bool is_menu_open();
};