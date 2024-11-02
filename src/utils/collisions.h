#ifndef COLLISIONS_H
#define COLLISIONS_H
#include "ecs.hpp"

class Collisions {
public:
    static int collides(const Entity& e1, const Entity& e2, bool user_interaction = false);
};



#endif //COLLISIONS_H
