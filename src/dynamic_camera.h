#ifndef DYNAMIC_CAMERA_H
#define DYNAMIC_CAMERA_H

#include "camera.h"
#include <algorithm>

struct Interval {
    float min;
    float max;
};

struct DynamicCamera {
    void update(Camera& camera, const sf::Vector2f& trackedPosWorld,
                const sf::Vector2f& trackedVelocityWorld) {
        float targetLookahead = 0.f;
        if (trackedVelocityWorld.x > 0.f) {
            targetLookahead = camera.windowPercentToWorldDist({lookAhead, 0.f}).x;
        } else if (trackedVelocityWorld.x < 0.f) {
            targetLookahead = -camera.windowPercentToWorldDist({lookAhead, 0.f}).x;
        }
        lookaheadState = (1.f - alpha) * lookaheadState + alpha * targetLookahead;

        const sf::Vector2f playerWindow = camera.worldToWindow(trackedPosWorld);
        const float playerPctX = playerWindow.x / cameraWindowWidth();

        const float leftRed = softZoneX.min;
        const float rightRed = softZoneX.max;
        const float leftGreen = deadZoneX.min;
        const float rightGreen = deadZoneX.max;

        if (playerPctX < leftRed) {
            camera.setCamera(trackedPosWorld, {leftRed * cameraWindowWidth(), playerWindow.y});
            cameraUnadjustedCenter = camera.getCameraCenter();
        } else if (playerPctX > rightRed) {
            camera.setCamera(trackedPosWorld, {rightRed * cameraWindowWidth(), playerWindow.y});
            cameraUnadjustedCenter = camera.getCameraCenter();
        } else if (playerPctX < leftGreen) {
            camera.translateWindow({alpha * (playerPctX - leftGreen) * cameraWindowWidth(), 0.f});
            cameraUnadjustedCenter = camera.getCameraCenter();
        } else if (playerPctX > rightGreen) {
            camera.translateWindow({alpha * (playerPctX - rightGreen) * cameraWindowWidth(), 0.f});
            cameraUnadjustedCenter = camera.getCameraCenter();
        } else {
            cameraUnadjustedCenter = camera.getCameraCenter();
        }

        camera.setCameraCenter(cameraUnadjustedCenter + sf::Vector2f{lookaheadState, 0.f});
    }

    float alpha;
    Interval deadZoneX;
    Interval softZoneX;
    float lookAhead;
    mutable sf::Vector2f cameraUnadjustedCenter{};
    mutable float lookaheadState{};

private:
    float cameraWindowWidth() const { return m_cachedWindowWidth; }

public:
    float m_cachedWindowWidth{1280.f};
};

#endif  // DYNAMIC_CAMERA_H
