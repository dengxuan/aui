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

#include <AUI/Render/RenderHints.h>
#include "BorderTop.h"

void ass::prop::Property<ass::BorderTop>::renderFor(AView* view, const ARenderContext& ctx) {
    RenderHints::PushColor x(ctx.render);
    int w = mInfo.width;
    ctx.render.rectangle(ASolidBrush{mInfo.color},
                         {0, 0},
                         {view->getWidth(), w});
}

bool ass::prop::Property<ass::BorderTop>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::prop::PropertySlot ass::prop::Property<ass::BorderTop>::getPropertySlot() const {
    return ass::prop::PropertySlot::BORDER;
}