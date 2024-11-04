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

void SoundSystem::load_all_sounds(const std::string& bgm_filename) {
    // Load background music
    background_music = Mix_LoadMUS(music_path(bgm_filename).c_str());

    if (background_music == nullptr) {
        LOG_ERROR("Failed to load music: {} make sure the file path exists", bgm_filename);
        throw std::runtime_error("Failed to load music: " + bgm_filename);
    }

    Mix_VolumeMusic(MIX_MAX_VOLUME * BGM_VOLUME_MULTIPLIER);
    Mix_PlayMusic(background_music, -1);

    // Load all sound effects
    load_chunks();
}

void SoundSystem::free_sounds() {
    // Free background music
    if (background_music != nullptr)
        Mix_FreeMusic(background_music);

    // Free all chunks
    for (const auto& [filename, chunk] : sfxs) {
        if (chunk != nullptr)
            Mix_FreeChunk(chunk);
    }

    Mix_CloseAudio();
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
    Mix_Chunk* sfx_to_play = sfxs[filename];

    sfx_to_play->volume = MIX_MAX_VOLUME * volume_multiplier;
    Mix_PlayChannel(-1, sfx_to_play, 0);
}
