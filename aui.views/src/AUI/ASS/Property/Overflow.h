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

#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Enum/AOverflow.h>   // defined here
#include "IProperty.h"

namespace ass {
namespace prop {
template <>
struct API_AUI_VIEWS Property<AOverflow> : IPropertyBase {
private:
    AOverflow mInfo;

public:
    Property(const AOverflow& info) : mInfo(info) {}

    void applyFor(AView* view) override;

    [[nodiscard]]
    const auto& value() const noexcept {
        return mInfo;
    }
};

template <>
struct API_AUI_VIEWS Property<AOverflowMask> : IPropertyBase {
private:
    AOverflowMask mInfo;

public:
    Property(const AOverflowMask& info) : mInfo(info) {}

    void applyFor(AView* view) override;

    [[nodiscard]]
    const auto& value() const noexcept {
        return mInfo;
    }
};
}   // namespace prop
}   // namespace ass
