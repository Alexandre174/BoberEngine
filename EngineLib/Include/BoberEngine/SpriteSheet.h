#pragma once

#include "Texture.h"
#include "RenderTypes.h"
struct SDL_Renderer;

#include <string>


class SpriteSheet
{
public:
    SpriteSheet() = default;

    // frameW/frameH are the pixel size of each frame, if 0 then it uses whole image
    bool load(SDL_Renderer* renderer, const std::string& filePath, int frameW, int frameH, bool enableColorKey = true);


    void renderFrame(SDL_Renderer* renderer, int frameIndex, float x, float y, float scale = 1.0f) const;
    void renderFrame(SDL_Renderer* renderer, int frameIndex, float x, float y, float scaleX, float scaleY) const; // for rending non square frames

    void renderFrameEx(SDL_Renderer* renderer, int frameIndex, float x, float y,
                       float scaleX, float scaleY,
                       double angleDeg,
                       FlipMode flip) const;

    int getFrameWidth() const { return mFrameW; }
    int getFrameHeight() const { return mFrameH; }
    int getColumns() const { return mColumns; }
    int getRows() const { return mRows; }

    const Texture& getTexture() const { return mTexture; }
    bool isValid() const { return mTexture.isValid(); }

private:
    Texture mTexture;
    int mFrameW = 0;
    int mFrameH = 0;
    int mColumns = 0;
    int mRows = 0;
};
