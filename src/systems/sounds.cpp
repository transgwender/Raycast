#include "sounds.hpp"

#include <SDL.h>
#include <filesystem>
#include "common.hpp"

namespace fs = std::filesystem;

void SoundSystem::init() {
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        LOG_ERROR("Failed to initialize SDL Audio");
        return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        LOG_ERROR("Failed to open audio device");
    }
}

void SoundSystem::load_all_sounds() {
    // Load background music
    background_music = Mix_LoadMUS(music_path(BGM_FILENAME).c_str());

    if (background_music == nullptr) {
        LOG_ERROR("Failed to load music: {} make sure the file path exists", BGM_FILENAME);
        throw std::runtime_error("Failed to load music: " + BGM_FILENAME);
    }

    active_music = NONE;

    forest_sounds = Mix_LoadMUS(music_path(FS_FILENAME).c_str());

    if (forest_sounds == nullptr) {
        LOG_ERROR("Failed to load music: {} make sure the file path exists", FS_FILENAME);
        throw std::runtime_error("Failed to load music: " + FS_FILENAME);
    }

    play_background();

    // Load all sound effects
    load_chunks();
}

void SoundSystem::free_sounds() {
    // Free background music
    if (background_music != nullptr)
        Mix_FreeMusic(background_music);

    if (forest_sounds != nullptr)
        Mix_FreeMusic(forest_sounds);

    // Free all chunks
    for (const auto& [filename, chunk] : sfxs) {
        if (chunk != nullptr)
            Mix_FreeChunk(chunk);
    }

    sfxs.clear();

    Mix_CloseAudio();
    Mix_Quit();
}

void SoundSystem::load_chunks() {
    const fs::path sfx_dir = sfx_dir_path();

    for (const auto& entry : fs::directory_iterator(sfx_dir)) {
        const auto filename = entry.path().filename().string();

        Mix_Chunk* chunk = Mix_LoadWAV(sfx_path(filename).c_str());

        if (chunk == nullptr) {
            LOG_ERROR("Failed to load sound effect: {} make sure the file path exists", filename);
            throw std::runtime_error("Failed to load sound effect: " + filename);
        }

        sfxs.emplace(filename, chunk);
    }
}

void SoundSystem::play_sound(const std::string& filename, const float volume_multiplier) {
    const auto it = sfxs.find(filename);
    if (it == sfxs.end()) {
        LOG_ERROR("Sound effect not found: {}", filename);
        return;
    }

    Mix_Chunk* sfx_to_play = it->second;

    Mix_VolumeChunk(sfx_to_play, static_cast<int>(MIX_MAX_VOLUME * volume_multiplier * sfx_volume));
    Mix_PlayChannel(-1, sfx_to_play, 0);
}

void SoundSystem::play_background() {
    if (active_music == MAIN) return;

    Mix_VolumeMusic(MIX_MAX_VOLUME * BGM_VOLUME_MULTIPLIER * music_volume);
    Mix_PlayMusic(background_music, -1);
    active_music = MAIN;
}

void SoundSystem::play_forest() {
    Mix_VolumeMusic(MIX_MAX_VOLUME * FS_VOLUME_MULTIPLIER * music_volume);
    Mix_FadeInMusic(forest_sounds, -1, 200);
    active_music = FOREST;
}

void SoundSystem::stop_music() {
    active_music = NONE;
    Mix_FadeOutMusic(2000);
}

void SoundSystem::change_volume_music(float volume) {
    music_volume = volume;
    if (active_music == MAIN) {
        Mix_VolumeMusic(MIX_MAX_VOLUME * BGM_VOLUME_MULTIPLIER * music_volume);
    } else if (active_music == FOREST) {
        Mix_VolumeMusic(MIX_MAX_VOLUME * FS_VOLUME_MULTIPLIER * music_volume);
    }
}

void SoundSystem::change_volume_sfx(float volume) {
    sfx_volume = volume;
}
