#include "utils.h"


/**
 * Sample the given frame time and update and return the global frame rate
 */
int Utils::fps(float ms_per_frame) {
    static int frame_rate = 0;
    static float time_counter = 0.0f;
    static int frames_in_second = 0;

    time_counter += ms_per_frame;
    if (time_counter >= 1000.0f) {
        frame_rate = frames_in_second;
        time_counter -= 1000.0f;
        frames_in_second = 0;
    }
    frames_in_second++;

    return frame_rate;
}
