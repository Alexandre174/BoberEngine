#include "SpriteSheet.h"

#include <algorithm>
#include <SDL3/SDL.h>

bool SpriteSheet::load(SDL_Renderer* renderer, const std::string& filePath, int frameW, int frameH, bool enableColorKey) //load the sprisheet from .bmp
{
    if (!mTexture.loadFromBMP(renderer, filePath, enableColorKey))
        return false;

    mFrameW = (frameW > 0) ? frameW : mTexture.getWidth();
    mFrameH = (frameH > 0) ? frameH : mTexture.getHeight();

    mColumns = 0;
    mRows = 0;

    if (mFrameW <= 0 || mFrameH <= 0)
        return false;

    mColumns = mTexture.getWidth() / mFrameW;
    mRows = mTexture.getHeight() / mFrameH;

    return mColumns > 0 && mRows > 0;
}

void SpriteSheet::renderFrame(SDL_Renderer* renderer, int frameIndex, float x, float y, float scale) const
{
    renderFrameEx(renderer, frameIndex, x, y, scale, scale, 0.0, FlipMode::None);
}

void SpriteSheet::renderFrame(SDL_Renderer* renderer, int frameIndex, float x, float y, float scaleX, float scaleY) const //draws specific frame
{
    renderFrameEx(renderer, frameIndex, x, y, scaleX, scaleY, 0.0, FlipMode::None);
}

void SpriteSheet::renderFrameEx(SDL_Renderer* renderer, int frameIndex, float x, float y,
                                float scaleX, float scaleY,
                                double angleDeg,
                                FlipMode flip) const
{
    if (!mTexture.isValid() || mColumns <= 0 || mRows <= 0)
        return;

    const int maxFrames = mColumns * mRows;
    if (maxFrames <= 0)
        return;

    // Wrap into valid range.
    frameIndex %= maxFrames;
    if (frameIndex < 0)
        frameIndex += maxFrames;

    const int col = frameIndex % mColumns;
    const int row = frameIndex / mColumns;

    SDL_FRect src{};
    src.x = static_cast<float>(col * mFrameW);
    src.y = static_cast<float>(row * mFrameH);
    src.w = static_cast<float>(mFrameW);
    src.h = static_cast<float>(mFrameH);

    SDL_FRect dst{};
    dst.x = x;
    dst.y = y;
    dst.w = static_cast<float>(mFrameW) * scaleX;
    dst.h = static_cast<float>(mFrameH) * scaleY;

    SDL_FlipMode sdlFlip = SDL_FLIP_NONE;
    switch (flip)
    {
    case FlipMode::Horizontal: sdlFlip = SDL_FLIP_HORIZONTAL; break;
    case FlipMode::Vertical: sdlFlip = SDL_FLIP_VERTICAL; break;
    case FlipMode::Both: sdlFlip = static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL); break;
    default: sdlFlip = SDL_FLIP_NONE; break;
    }

    mTexture.renderEx(nullptr, &src, &dst, angleDeg, nullptr, sdlFlip);
}
