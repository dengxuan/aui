﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AVerticalLayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/View/AViewContainer.h"

#undef max
#undef min

#include "HVLayout.h"

using HVLayout = aui::HVLayout<ALayoutDirection::VERTICAL>;

void AVerticalLayout::onResize(int x, int y, int width, int height) {
    HVLayout::onResize({ x, y }, { width, height }, mViews, mSpacing);
}

int AVerticalLayout::getMinimumWidth() {
    return HVLayout::getMinimumWidth(mViews, mSpacing);
}

int AVerticalLayout::getMinimumHeight() {
    return HVLayout::getMinimumHeight(mViews, mSpacing);
}

void AVerticalLayout::setSpacing(int spacing) {
    if (mSpacing == spacing) [[unlikely]] {
        return;
    }
    mSpacing = spacing;
    if (mViews.empty()) {
        return;
    }
    AUI_NULLSAFE(mViews.first()->getParent())->markMinContentSizeInvalid();
}

ALayoutDirection AVerticalLayout::getLayoutDirection() {
    return ALayoutDirection::VERTICAL;
}
