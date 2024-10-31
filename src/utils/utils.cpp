#include "utils.h"

// converts frame time in ms to FPS
int Utils::fps(float ms_per_frame) {
    return (int)((1.f / ms_per_frame) * 1000);
}
