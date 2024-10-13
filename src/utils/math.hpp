#pragma once

#include "common.hpp"
#include <cmath>

using vec2 = glm::vec2;

namespace raycast {
namespace math {
inline float lerp(float start, float end, float t) {
    return start * (1 - t) + (end * t);
}
inline vec2 lerp(vec2 start, vec2 end, float t) {
    return vec2(lerp(start.x, end.x, t), lerp(start.y, end.y, t));
}

///////////////////////////////////////////////////////////////////////////////////
///
/// The methods below help in making floating point comparisons more
/// confidently. They are taken from "The Art of Computer Programming" by Knuth.
///
///////////////////////////////////////////////////////////////////////////////////

inline bool approximatelyEqual(float a, float b, float epsilon) {
    return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}
inline bool essentiallyEqual(float a, float b, float epsilon) {
    return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}
inline bool definitelyGreaterThan(float a, float b, float epsilon) {
    return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}
inline bool definitelyLessThan(float a, float b, float epsilon) {
    return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}
inline bool approximatelyEqual(float a, float b) {
    return approximatelyEqual(a, b, std::numeric_limits<double>::epsilon());
}
inline bool essentiallyEqual(float a, float b) {
    return essentiallyEqual(a, b, std::numeric_limits<double>::epsilon());
}
inline bool definitelyGreaterThan(float a, float b) {
    return definitelyGreaterThan(a, b, std::numeric_limits<double>::epsilon());
}
inline bool definitelyLessThan(float a, float b) {
    return definitelyLessThan(a, b, std::numeric_limits<double>::epsilon());
}
} // namespace math
} // namespace raycast
