#include "utils/math.hpp"

namespace raycast {
namespace math {

float snap(float value, float interval) {
    float modulo = fmod(value, interval);
    if (essentiallyEqual(modulo, 0.0f)) {
        return value;
    }

    if (modulo < 0.0f) {
        if (fabs(modulo * 2.f) > interval) {
            return value - interval - modulo;
        } else {
            return value - modulo;
        }
    } else {
        if (modulo * 2.f > interval) {
            return value + interval - modulo;
        } else {
            return value - modulo;
        }
    }
}

}
} // namespace raycast
