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

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include "AUI/Font/FreeType.h"
#include "AUI/Platform/AFontManager.h"
#include <fstream>
#include <string>
#include <algorithm>
#include <climits>
#include <cstdint>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"


AFont::AFont(AFontManager* fm, const AString& path) :
        ft(fm->mFreeType) {
    if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &mFace)) {
        throw AException("Could not load font: " + path);
    }
}

AFont::AFont(AFontManager* fm, const AUrl& url) :
        ft(fm->mFreeType) {
    if (url.schema() == "file") {
        if (FT_New_Face(fm->mFreeType->getFt(), url.path().toStdString().c_str(), 0, &mFace)) {
            throw AException("Could not load font: " + url.full());
        }
        return;
    }
    mFontDataBuffer = AByteBuffer::fromStream(url.open());

    if (FT_New_Memory_Face(fm->mFreeType->getFt(), (const FT_Byte*) mFontDataBuffer.data(), mFontDataBuffer.getSize(),
                           0, &mFace)) {
        throw AException("Could not load font: " + url.full());
    }
}

AString AFont::getFontFamilyName() const {
    FT_SfntName name;
    FT_Get_Sfnt_Name(mFace, 0, &name);
    return std::string(name.string, name.string + name.string_len);
}

AFontFamily::Weight AFont::getFontWeight() const {
    return AFontFamily::NORMAL;
}

bool AFont::isItalic() const {
    return mFace->style_flags & FT_STYLE_FLAG_ITALIC;
}


glm::vec2 AFont::getKerning(wchar_t left, wchar_t right) {
    FT_Vector vec2;
    FT_Get_Kerning(mFace, left, right, FT_KERNING_DEFAULT, &vec2);

    return {vec2.x >> 6, vec2.y >> 6};
}

AFont::Character AFont::renderGlyph(const FontEntry& fs, AChar glyph) {
    int size = fs.first.size;
    FontRendering fr = fs.first.fr;

    // 彩色 emoji 字体（COLR/CBDT/sbix）是固定位图 strike——FT_Set_Pixel_Sizes 常失败，
    // 需先按 FT_FACE_FLAG_COLOR 探测：有彩色字形时选最近 strike，渲染出 BGRA 位图后缩放到 size。
    const bool fontHasColor = (mFace->face_flags & FT_FACE_FLAG_COLOR) != 0;

    if (fontHasColor && (mFace->face_flags & FT_FACE_FLAG_SCALABLE) == 0) {
        // 纯位图彩色字体：选像素高度最接近 size 的 strike。
        int best = 0; int bestDiff = INT_MAX;
        for (int i = 0; i < mFace->num_fixed_sizes; ++i) {
            int h = mFace->available_sizes[i].height;
            int d = h > size ? h - size : size - h;
            if (d < bestDiff) { bestDiff = d; best = i; }
        }
        if (mFace->num_fixed_sizes > 0) FT_Select_Size(mFace, best);
    } else {
        FT_Set_Pixel_Sizes(mFace, 0, size);
    }

    FT_Int32 flags = FT_LOAD_RENDER;
    if (fontHasColor) flags |= FT_LOAD_COLOR;    // 让 FreeType 输出 BGRA 彩色位图

    if (fr == FontRendering::SUBPIXEL && !fontHasColor)
        flags |= FT_LOAD_TARGET_LCD;
    if (fr == FontRendering::NEAREST && !fontHasColor)
        flags |= FT_LOAD_TARGET_MONO;

    FT_Error e = FT_Load_Char(mFace, glyph.codepoint(), flags);
    if (e) {
        throw std::runtime_error(("Cannot load char: error code" + AString::number(e)).toStdString());
    }
    FT_GlyphSlot g = mFace->glyph;

    // ---- 彩色 BGRA 位图字形分支：转 RGBA + 缩放到目标行高，metrics 按缩放比换算 ----
    if (g->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && g->bitmap.width && g->bitmap.rows) {
        const unsigned srcW = g->bitmap.width, srcH = g->bitmap.rows;
        // 目标缩放：strike 高度可能 != size，等比缩放到 size 像素行高。
        const float scale = srcH > 0 ? float(size) / float(srcH) : 1.f;
        // 防御性上限：dst 不超过 size 的 2 倍（彩色字形不该比行高大太多，防图集爆显存）。
        const unsigned cap = unsigned(size) * 2 + 2;
        const unsigned dstW = std::min(cap, std::max(1u, unsigned(srcW * scale + 0.5f)));
        const unsigned dstH = std::min(cap, std::max(1u, unsigned(srcH * scale + 0.5f)));

        AByteBuffer data;
        data.resize(size_t(dstW) * dstH * 4);
        auto* out = reinterpret_cast<uint8_t*>(data.data());
        // 最近邻缩放 + BGRA→RGBA（premultiplied alpha 保留，直接搬通道）。
        for (unsigned y = 0; y < dstH; ++y) {
            unsigned sy = std::min(srcH - 1, unsigned(y / scale));
            const uint8_t* srcRow = g->bitmap.buffer + size_t(sy) * g->bitmap.pitch;
            for (unsigned x = 0; x < dstW; ++x) {
                unsigned sx = std::min(srcW - 1, unsigned(x / scale));
                const uint8_t* p = srcRow + size_t(sx) * 4;   // BGRA
                uint8_t* q = out + (size_t(y) * dstW + x) * 4;
                q[0] = p[2]; q[1] = p[1]; q[2] = p[0]; q[3] = p[3];   // RGBA
            }
        }
        const float div = 1.f / 64.f;
        // bearing/advance 用 bitmap_left/top（像素）× scale，advance 用 metrics.horiAdvance × scale。
        return Character{
            .image = _new<AImage>(data, glm::uvec2(dstW, dstH),
                                  APixelFormat::BYTE | APixelFormat::RGBA),
            .size = { float(dstW), float(dstH) },
            .horizontal = {
              .bearing = { g->bitmap_left * scale, g->bitmap_top * scale },
              .advance = div * g->metrics.horiAdvance * scale,
            },
            .vertical = {
              .bearing = { div * g->metrics.vertBearingX * scale, div * g->metrics.vertBearingY * scale },
              .advance = div * g->metrics.vertAdvance * scale,
            },
            .colored = true,
        };
    }

    if (g->bitmap.width && g->bitmap.rows) {
        const float div = 1.f / 64.f;
        int width = g->bitmap.width;

        if (fr == FontRendering::SUBPIXEL)
            width /= 3;

        int height = g->bitmap.rows;

        AByteBuffer data;

        if (fr == FontRendering::NEAREST) {
            // when nearest, freetype renders glyphs into the 1bit-depth image but OpenGL required at least8bit-depth,
            // so we will convert it here
            data.resize(g->bitmap.rows * g->bitmap.width);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                for (unsigned c = 0; c < g->bitmap.width; ++c) {
                    data.at<std::uint8_t>(c + r * g->bitmap.width) = (bufPtr[c / 8] & (0b10000000 >> (c % 8))) ? 255
                                                                                                               : 0;
                }
            }
        } else {
            data.reserve(g->bitmap.rows * g->bitmap.pitch);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                data.write(reinterpret_cast<const char*>(bufPtr), g->bitmap.width);
            }
        }

        int imageFormat = APixelFormat::BYTE;
        if (fr == FontRendering::SUBPIXEL)
            imageFormat |= APixelFormat::RGB;
        else
            imageFormat |= APixelFormat::R;

        return Character{
            .image = _new<AImage>(data, glm::uvec2(width, height), imageFormat),
            .size = { div * g->metrics.width, div * g->metrics.height },
            .horizontal = {
              .bearing = { g->bitmap_left, g->bitmap_top },
              .advance = div * g->metrics.horiAdvance,
            },
            .vertical = {
              .bearing = { div * g->metrics.vertBearingX, div * g->metrics.vertBearingY },
              .advance = div * g->metrics.vertAdvance,
            },
        };
    }
    return Character{};
}

AFont::Character& AFont::getCharacter(const FontEntry& charset, AChar glyph) {
    auto& chars = charset.second.characters;
    if (chars.size() > glyph && chars[glyph.codepoint()]) {
        return *chars[glyph.codepoint()];
    } else {
        if (chars.size() <= glyph) {
            chars.resize(glyph + 1, std::nullopt);
        }
        chars[glyph.codepoint()] = std::move(renderGlyph(charset, glyph));

        return *chars[glyph.codepoint()];
    }
}

int AFont::length(const FontEntry& charset, AStringView text) {
    return length(charset, text.utf8().begin(), text.utf8().end());
}

int AFont::length(const FontEntry& charset, std::u32string_view text) {
    return length(charset, text.begin(), text.end());
}

bool AFont::isHasKerning() {
    return FT_HAS_KERNING(mFace);
}

int AFont::getAscenderHeight(unsigned size) const {
    return int(mFace->ascender) * size / mFace->height;
}


int AFont::getDescenderHeight(unsigned size) const {
    return -int(mFace->descender) * size / mFace->height;
}
