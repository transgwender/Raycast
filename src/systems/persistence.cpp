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

    return true;
}
