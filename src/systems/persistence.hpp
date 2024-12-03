#pragma once

#include "json.hpp"
#include <map>
#include "utils/defines.hpp"

using json = nlohmann::json;

struct GameSettings {
    float sfxVolume = 0.7;
    float musicVolume = 0.7;
    bool hardMode = false;
};

inline void to_json(json& j, const GameSettings& c) {
    j = json{{"sfxVolume", c.sfxVolume}, {"musicVolume", c.musicVolume}, {"hardMode", c.hardMode}};
}

inline void from_json(const json& j, GameSettings& c) {
    if (j.contains("sfxVolume")) j.at("sfxVolume").get_to(c.sfxVolume);
    if (j.contains("musicVolume")) j.at("musicVolume").get_to(c.musicVolume);
    if (j.contains("hardMode")) j.at("hardMode").get_to(c.hardMode);
}

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
    float get_settings_music_volume();
    float get_settings_sfx_volume();
    bool get_settings_hard_mode();
    void set_settings_music_volume(float volume);
    void set_settings_sfx_volume(float volume);
    void set_settings_hard_mode(bool hard);

#ifdef ALLOW_DEBUG_FUNCTIONS
    void debug_set_all_accessible(int levelCount);
#endif


  private:
    std::map<int, LEVEL_STATE> data;
    GameSettings current_settings;
};