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

/// @brief returns the value snapped to the nearest whole number multiple of the interval
/// @param value: the value to round
/// @param interval: the number to round to
float snap(float value, float interval);

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

 inline float clamp(float start, float end, float t) {
  if (definitelyLessThan(t, start)) return start;
  if (definitelyGreaterThan(t, end)) return end;
  return t;
}

/// @brief returns the angle between the vector `vec` and the positive x-axis in radians
inline float heading(vec2 vec) {
    return atan2(vec.y, vec.x);
}

/// @brief returns a normalized vector pointing the direction of theta (reference from +x-axis)
inline vec2 from_angle(float theta) {
    return vec2(::cos(theta), ::sin(theta));
}

/// @brief returns a vector pointing in the same direction as vec but with magnitude set to mag
inline vec2 set_mag(vec2 vec, float mag) {
    return glm::normalize(vec) * mag;
}

/// @brief project vector a onto vector b
inline vec2 project(vec2 a, vec2 b) {
    return (glm::dot(a,b) / glm::dot(b,b)) * b;
}

/// @brief project points on the line segment between start and end and clamps to be strictly on this line
inline vec2 clampToLineSegment(vec2 start, vec2 end, vec2 point) {
    vec2 dir = end - start;
    float t = glm::dot(point - start, dir) / glm::dot(dir, dir);
    t = glm::clamp(t, 0.0f, 1.0f);
    return start + t * dir;
}

} // namespace math
} // namespace raycast
