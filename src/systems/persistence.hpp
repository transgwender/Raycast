#pragma once

#include "json.hpp"
#include <map>
#include "utils/defines.hpp"

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

#ifdef ALLOW_DEBUG_FUNCTIONS
    void debug_set_all_accessible(int levelCount);
#endif

  private:
    std::map<int, LEVEL_STATE> data;
};