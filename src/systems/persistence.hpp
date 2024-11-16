#pragma once

#include "json.hpp"
#include <map>

using json = nlohmann::json;

class PersistenceSystem {
    enum LEVEL_STATE {
        LOCKED = 0,
        ACCESSIBLE = 1,
        BEATEN = 2
    };

  public:
    void init();
    bool try_write_save();
    void clear_data();
    bool get_is_accessible(int levelNum);
    bool get_is_beaten(int levelNum);
    bool get_is_locked(int levelNum);
    void set_beaten(int levelNum);
    void set_accessible(int levelNum);

  private:
    std::map<int, LEVEL_STATE> data;
};