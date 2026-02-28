#pragma once

#include <SDL3/SDL.h>

#include <string>




class Texture
{
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // loads .bmp file, if enableColorKey is true then it uses (255,0,255) as transparent
    bool loadFromBMP(SDL_Renderer* renderer, const std::string& filePath, bool enableColorKey = true);

    void destroy();

    // wrapper for SDL_RenderTexture
    bool render(SDL_Renderer* renderer, const SDL_FRect* srcRect, const SDL_FRect* dstRect) const;

    // wrapper for SDL_RenderTextureRotated (use angle=0 to only flip)
    bool renderEx(SDL_Renderer* renderer,
                  const SDL_FRect* srcRect,
                  const SDL_FRect* dstRect,
                  double angle = 0.0,
                  const SDL_FPoint* center = nullptr,
                  SDL_FlipMode flip = SDL_FLIP_NONE) const;

    unsigned int getId() const { return mTextureId; }
    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }
    bool isValid() const { return mTextureId != 0; }

private:
    unsigned int mTextureId = 0;
    int mWidth = 0;
    int mHeight = 0;
};
