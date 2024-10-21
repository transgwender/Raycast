#pragma once

#ifndef SOUNDS_H
#define SOUNDS_H

#include <SDL_mixer.h>
#include <unordered_map>
#include <string>

constexpr float BGM_VOLUME_MULTIPLIER = 0.5;
const std::string BGM_FILENAME = "8BitCave.wav";

class SoundSystem {
    public:
        /**
         * Initialize SDL Mixer
         */
        static void init();

        /**
         * Load BGM and SFX and play BGM
         */
        void load_all_sounds(const std::string& bgm_filename = BGM_FILENAME);

        /**
         * Free all sounds
         */
        void free_sounds();

        /**
         * @param filename          - use filename with extension. For example: "my_sound.wav".
         * @param volume_multiplier - float value from 0 to 1. 0 being muted, 1 being max volume. Defaults to 1.
         */
        void play_sound(const std::string& filename, float volume_multiplier = 1);

    private:
        Mix_Music* background_music = nullptr;
        std::unordered_map<std::string, Mix_Chunk*> sfxs;

        void load_chunks();
};

#endif //SOUNDS_H
