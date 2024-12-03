#include "persistence.hpp"
#include "log.hpp"
#include <fstream>
#include <iostream>

void PersistenceSystem::init() {

    std::ifstream save_file("save");

    if (save_file.is_open()) {
        nlohmann::json j;
        save_file >> j;
        j.get_to(data);

        LOG_INFO("Loading data: {}", j.dump());

        save_file.close();
    }

    if (data[1] != LEVEL_STATE::BEATEN) {
        data[1] = LEVEL_STATE::ACCESSIBLE;
    }

    std::ifstream setting_file("settings");

    if (setting_file.is_open()) {
        nlohmann::json j;
        setting_file >> j;
        j.get_to(current_settings);

        LOG_INFO("Loading settings: {}", j.dump());

        setting_file.close();
    }

}

bool PersistenceSystem::get_is_accessible(int levelNum) {
    LEVEL_STATE state = data[levelNum];
    if (state != LEVEL_STATE::LOCKED) {
        return true;
    }
    return false;
}
bool PersistenceSystem::get_is_beaten(int levelNum) {
    LEVEL_STATE state = data[levelNum];
    if (state == LEVEL_STATE::BEATEN) {
        return true;
    }
    return false;
}
bool PersistenceSystem::get_is_locked(int levelNum) {
    LEVEL_STATE state = data[levelNum];
    if (state == LEVEL_STATE::LOCKED) {
        return true;
    }
    return false;
}
void PersistenceSystem::set_beaten(int levelNum) {
    data[levelNum] = LEVEL_STATE::BEATEN;
}
void PersistenceSystem::set_accessible(int levelNum) {
    if (data[levelNum] != LEVEL_STATE::BEATEN) { // Can only go locked -> accessible. Not beaten -> accessible
        data[levelNum] = LEVEL_STATE::ACCESSIBLE;
    }
}
void PersistenceSystem::clear_data() {
    for (auto &it : data) {
        it.second = LEVEL_STATE::LOCKED;
    }
    data[1] = LEVEL_STATE::ACCESSIBLE;
    LOG_INFO("Cleared data");
    try_write_save();
}

bool PersistenceSystem::try_write_save() {
    json j = data;

    std::ofstream save_file("save");
    save_file << j;
    save_file.close();

    LOG_INFO("Saving data: {}", j.dump());

    j = current_settings;

    std::ofstream setting_file("settings");
    setting_file << j;
    setting_file.close();

    LOG_INFO("Saving settings: {}", j.dump());

    return true;
}

#ifdef ALLOW_DEBUG_FUNCTIONS
void PersistenceSystem::debug_set_all_accessible(int levelCount) {

    LOG_INFO("DEBUG: Setting all levels accessible");

    for (int i = 1; i <= levelCount; ++i) {
        set_accessible(i);
    }
}
#endif


float PersistenceSystem::get_settings_music_volume() {
    return current_settings.musicVolume;
}

float PersistenceSystem::get_settings_sfx_volume() {
    return current_settings.sfxVolume;
}

bool PersistenceSystem::get_settings_hard_mode() {
    return current_settings.hardMode;
}

void PersistenceSystem::set_settings_music_volume(float volume) {
    current_settings.musicVolume = volume;
}

void PersistenceSystem::set_settings_sfx_volume(float volume) {
    current_settings.sfxVolume = volume;
}

void PersistenceSystem::set_settings_hard_mode(bool hard) {
    current_settings.hardMode = hard;
}
