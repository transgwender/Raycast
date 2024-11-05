#ifndef COLLISIONS_H
#define COLLISIONS_H
#include "ecs.hpp"
#include "math.hpp"

class Collisions {
public:
    static vec2 overlap(const Entity& e1, const Entity& e2, bool user_interaction = false);
};



#endif //COLLISIONS_H
