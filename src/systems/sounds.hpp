#pragma once

#ifndef SOUNDS_H
#define SOUNDS_H

#include <SDL_mixer.h>
#include <unordered_map>
#include <string>

constexpr float BGM_VOLUME_MULTIPLIER = 0.3;
const std::string BGM_FILENAME = "8BitCave.wav";
constexpr float FS_VOLUME_MULTIPLIER = 0.5;
const std::string FS_FILENAME = "ForestAmbient.wav";

class SoundSystem {
    public:
        Mix_Music* background_music = nullptr;
        Mix_Music* forest_sounds = nullptr;

        bool is_curr_background = false;

        /**
         * Initialize SDL Mixer
         */
        static void init();

        /**
         * Load BGM and SFX and play BGM
         */
        void load_all_sounds();

        /**
         * Free all sounds
         */
        void free_sounds();

        /**
         * @param filename          - use filename with extension. For example: "my_sound.wav".
         * @param volume_multiplier - float value from 0 to 1. 0 being muted, 1 being max volume. Defaults to 1.
         */
        void play_sound(const std::string& filename, float volume_multiplier = 1);

        void play_background();
        void play_forest();

        void stop_background();

      private:
        std::unordered_map<std::string, Mix_Chunk*> sfxs;

        void load_chunks();
};

#endif //SOUNDS_H
