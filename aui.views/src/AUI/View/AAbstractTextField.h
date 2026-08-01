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

#include <AUI/View/AAbstractTypeableView.h>
#include "AUI/Enum/ATextInputType.h"
#include <functional>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Common/IStringable.h>
#include <AUI/Render/IRenderer.h>

/**
 * @brief Text field implementation
 * @details ATextField is separated into the different class in order to simplify styling.
 */
class API_AUI_VIEWS AAbstractTextField : public AAbstractTypeableView<AView>, public IStringable {
public:
    AAbstractTextField();

    ~AAbstractTextField() override;

    int getContentMinimumHeight() override;

    void setText(const AString& t) override;

    void setSuffix(const AString& s);

    void render(ARenderContext ctx) override;

    AString toString() const override;

    void setTextInputType(ATextInputType textInputType) noexcept {
        mTextInputType = textInputType;
    }

    [[nodiscard]]
    ATextInputType textInputType() const noexcept override {
        return mTextInputType;
    }

    void setEditable(bool isEditable) {
        mIsEditable = isEditable;
    }

    void setPasswordMode(bool isPasswordField) {
        mIsPasswordTextField = isPasswordField;
        setCopyable(!isPasswordField);
    }

    [[nodiscard]]
    bool isPasswordField() const noexcept override {
        return mIsPasswordTextField;
    }

    bool handlesNonMouseNavigation() override;

    AString getText() const override;

    void onCharEntered(AChar c) override;

    void setSize(glm::ivec2 size) override;

    glm::ivec2 getCursorPosition() override;

    /**
     * @brief 内联绘制钩子：文本字形画完后回调，让上层在字符位置上叠画（如 emoji 精灵图）。
     * @details 参数：renderer、当前文本(utf32)、xByIndex(字符下标→该字符左边 x 像素，含
     *          padding/scroll 的**内容坐标**)、centerY(文本可视区竖直中心，供图片按中心居中)、
     *          字号。文本仍以码点存储（光标/IME 原生工作），上层据此把某些码点段叠画成图片。
     *          Telegram 输入框 emoji 内联同思路（Qt 用 QTextObjectInterface，这里用绘制钩子）。
     */
    using InlineDrawer = std::function<void(IRenderer& render, const std::u32string& text,
                                            const std::function<int(size_t)>& xByIndex,
                                            int baselineY, int fontSize,
                                            size_t selBegin, size_t selEnd)>;
    void setInlineDrawer(InlineDrawer drawer) { mInlineDrawer = std::move(drawer); invalidateFont(); }

protected:
    _<IRenderer::IPrerenderedString> mPrerenderedString;
    std::u32string mContents;
    AString mSuffix;
    InlineDrawer mInlineDrawer;

    virtual bool isValidText(std::u32string_view text);

    void prerenderStringIfNeeded(IRenderer& render);

    void typeableErase(size_t begin, size_t end) override;

    bool typeableInsert(size_t at, const AString& toInsert) override;

    size_t typeableFind(AChar c, size_t startPos) override;

    size_t typeableReverseFind(AChar c, size_t startPos) override;

    size_t length() const override;

    bool typeableInsert(size_t at, AChar toInsert) override;

    std::u32string getDisplayText() override;

    void cursorSelectableRedraw() override;

    unsigned cursorIndexByPos(glm::ivec2 pos) override;
    glm::ivec2 getPosByIndex(size_t index) override;

    void doDrawString(IRenderer& render);

    void onCursorIndexChanged() override;
    void commitStyle() override;

private:
    ATextInputType mTextInputType = ATextInputType::DEFAULT;
    bool mIsPasswordTextField = false;
    bool mIsEditable = true;
    int mTextAlignOffset = 0;
    int mHorizontalScroll = 0; // positive only
    unsigned mAbsoluteCursorPos = 0;
    ATextLayoutHelper mTextLayoutHelper;

    void invalidateFont() override;

    void updateTextAlignOffset();

    int getPosByIndexAbsolute(size_t index);

    int getVerticalAlignmentOffset() noexcept;
};
