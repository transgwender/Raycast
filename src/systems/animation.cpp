#include "animation.hpp"
#include "registry.hpp"

void AnimationSystem::step(float elapsed_ms) const {
    for (int i = 0; i < registry.spriteSheets.size(); i++) {
        const Entity& entity = registry.spriteSheets.entities[i];
        SpriteSheet& ss = registry.spriteSheets.components[i];
        ss.timeElapsed += elapsed_ms;

        if (registry.levers.has(entity)) { // IF animated sprite is a lever
            animateLever(entity, ss);
        } else { // If animated sprite is not a lever

            // Update animation frame
            if (ss.timeElapsed >= animation_speed) {
                ss.currFrame = (ss.currFrame + 1) % ss.animationFrames[ss.currState];
                ss.timeElapsed = 0.f;
            }
        }

        Material& material = registry.materials.get(entity);
        material.texture.h_offset = ss.cellHeight * static_cast<float>(ss.currFrame) / ss.sheetWidth;
        material.texture.v_offset = ss.cellWidth * static_cast<float>(ss.currState) / ss.sheetHeight;
        material.texture.cell_size = vec2(ss.cellWidth / ss.sheetWidth, ss.cellHeight / ss.sheetHeight);
    }
}

void AnimationSystem::animateLever(const Entity& entity, SpriteSheet& ss) const {
    Lever& lever = registry.levers.get(entity);

    // Update animation frame
    if (ss.timeElapsed >= animation_speed) {
        if (lever.movementState == LEVER_MOVEMENT_STATES::PUSHED_RIGHT &&
            ss.currFrame < ss.animationFrames[ss.currState] - 1) {
            ss.currFrame = (ss.currFrame + 1) % ss.animationFrames[ss.currState];
            ss.timeElapsed = 0.f;
            lever.movementState = LEVER_MOVEMENT_STATES::STILL;
        }

        if (lever.movementState == LEVER_MOVEMENT_STATES::PUSHED_LEFT && ss.currFrame > 0) {
            ss.currFrame = (ss.currFrame - 1) % ss.animationFrames[ss.currState];
            ss.timeElapsed = 0.f;
            lever.movementState = LEVER_MOVEMENT_STATES::STILL;
        }

        lever.state = (LEVER_STATES)ss.currFrame;
    }
}
