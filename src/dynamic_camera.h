#ifndef DYNAMIC_CAMERA_H
#define DYNAMIC_CAMERA_H

#include <iostream>
#include <limits>
#include "camera.h"
#include <algorithm>

constexpr float INF = std::numeric_limits<float>::infinity();

struct Interval {
    float min;
    float max;
    constexpr bool contains(const Interval& other) const {
        return (min <= other.min) && (max >= other.max);
    }

    constexpr bool contains(float x) const { return (min <= x) && (x <= max); }

    // signedDistance returns a positive smallest distance of x from the interval boundary
    // if it is not contained, else it returns a smallest (in magnitude) negative distance
    // from the interval boundary.
    constexpr float signedDistance(float x) const {
        if (contains(x)) {
            // 0 or negative
            return std::max(x - max, min - x);
        }
        // positive
        return std::max(x - max, min - x);
    }
};

struct DynamicCamera {
    void update(Camera& camera,                           // the camera to update
                const sf::Vector2f& trackedPosWorld,      // world coords of player to track
                const sf::Vector2f& trackedVelocityWorld  // world velocity of player to track
    ) {
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

    float alpha;  // alpha to use for damping of camera position, *as well as* for smoothing of
                  // lookahead offset.
    Interval deadZoneX;  // percentage interval along x-axis to use for dead zone in [0, 1]
    // percentage interval along x-axis to use for soft rejection zone in [0, 1]
    Interval softZoneX = {-INF, INF};
    // (everything beyond softZone is considered hard zone).
    float lookAhead;  // screen-percentage lookahead
    mutable sf::Vector2f
        cameraUnadjustedCenter;      // the camera world center from last frame, ignoring lookahead
    mutable float lookaheadState{}; 

private:
    float cameraWindowWidth() const { return m_cachedWindowWidth; }

public:
    float m_cachedWindowWidth{1280.f};
};

#endif  // DYNAMIC_CAMERA_H
