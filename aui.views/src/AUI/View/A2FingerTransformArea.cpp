/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "A2FingerTransformArea.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AAngleRadians.h"

A2FingerTransformArea::A2FingerTransformArea() {
    setExpanding();
    setLayout(std::make_unique<AStackedLayout>());
}

void A2FingerTransformArea::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);
    mTrackedPoints << TrackedPoint { .pos = event.position, .pointerIndex = event.pointerIndex };
}

void A2FingerTransformArea::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, event);
    auto trackedPoint = mTrackedPoints.findIf(event.pointerIndex, &TrackedPoint::pointerIndex);
    if (!trackedPoint) {
        return;
    }

    auto delta = pos - trackedPoint->pos;

    switch (mTrackedPoints.size()) {
        case 1: {
            if (glm::length2(delta) < 0.01f) {
                return;
            }
            trackedPoint->pos = pos;
            emit transformed(A2DTransform{ .offset = delta });
            break;
        }
        case 2: {
            auto calculateAngle = [&] {
                auto direction = mTrackedPoints.first().pos - mTrackedPoints.last().pos;
                return AAngleRadians(glm::atan(direction.x, direction.y));
            };
            auto calculateLength = [&] {
                return glm::distance(mTrackedPoints.first().pos, mTrackedPoints.last().pos);
            };

            auto prevLength = calculateLength();
            auto prevAngle = calculateAngle();
            trackedPoint->pos = pos;
            auto newLength = calculateLength();
            auto newAngle = calculateAngle();
            auto scale = newLength / prevLength;
            if (scale != scale) { // nan
                scale = 1.f;
            } else {
                scale = glm::clamp(scale, 0.000001f, 1000000.f);
            }
            emit transformed(A2DTransform{ .offset = delta / 2.f, .rotation = prevAngle - newAngle, .scale = scale });
            break;   
        }
        default:
            break;
    }
}

void A2FingerTransformArea::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    mTrackedPoints.removeAll(event.pointerIndex, &TrackedPoint::pointerIndex);
}

bool A2FingerTransformArea::consumesClick(const glm::ivec2& pos) {
    return true;
}
