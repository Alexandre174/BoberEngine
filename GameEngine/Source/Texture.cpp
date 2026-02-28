#include "Texture.h"

#include <SDL3/SDL_filesystem.h>

#include <glad/glad.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
    bool FileExists(const std::string& path)
    {
        std::ifstream f(path.c_str(), std::ios::binary);
        return f.good();
    }

    std::string JoinPath(const std::string& a, const std::string& b)
    {
        if (a.empty()) return b;
        if (b.empty()) return a;

        const char last = a.back();
        if (last == '/' || last == '\\')
            return a + b;

        return a + "/" + b;
    }

    std::string ResolveAssetPath(const std::string& filePath)
    {
        if (FileExists(filePath))
            return filePath;

        const char* base = SDL_GetBasePath();
        if (!base)
            return filePath;

        const std::string basePath = base;

        const std::vector<std::string> prefixes = {
            basePath,
            JoinPath(basePath, "../"),
            JoinPath(basePath, "../../"),
            JoinPath(basePath, "../../../"),
            JoinPath(basePath, "../../../../"),
        };

        for (const auto& p : prefixes)
        {
            const auto candidate = JoinPath(p, filePath);
            if (FileExists(candidate))
                return candidate;
        }

        return filePath;
    }

    void ApplyMagentaTransparency(SDL_Surface* surface, int tolerance = 20) // aplyes transparent background
    {
        if (!surface || !surface->pixels)
            return;

        const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surface->format);
        if (!fmt)
            return;

        if (!SDL_LockSurface(surface))
            return;

        auto* base = static_cast<Uint8*>(surface->pixels);
        for (int y = 0; y < surface->h; ++y)
        {
            auto* row = reinterpret_cast<Uint32*>(base + y * surface->pitch);
            for (int x = 0; x < surface->w; ++x)
            {
                Uint8 r = 0, g = 0, b = 0, a = 255;
                SDL_GetRGBA(row[x], fmt, nullptr, &r, &g, &b, &a);

                const bool magentaish =
                    (r >= 255 - tolerance) &&
                    (b >= 255 - tolerance) &&
                    (g <= tolerance);

                if (magentaish)
                {
                    row[x] = SDL_MapRGBA(fmt, nullptr, 0, 0, 0, 0);
                }
            }
        }

        SDL_UnlockSurface(surface);
    }

    struct GLState
    {
        GLuint program = 0;
        GLuint vao = 0;
        GLuint vbo = 0;
        GLint uProjLoc = -1;
        GLint uTexLoc = -1;
        bool ready = false;
    };

    GLState& GetGL()
    {
        static GLState s;
        return s;
    }

    GLuint Compile(GLenum type, const char* src)
    {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        GLint ok = 0;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[2048] = {};
            glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
            std::cerr << "Shader compile failed: " << log << "\n";
            glDeleteShader(sh);
            return 0;
        }
        return sh;
    }

    bool EnsureGLReady()
    {
        GLState& s = GetGL();
        if (s.ready)
            return true;

        // open gl shaders
        const char* vs = R"( 
            #version 330 core
            layout(location=0) in vec2 aPos;
            layout(location=1) in vec2 aUV;
            uniform mat4 uProj;
            out vec2 vUV;
            void main(){
                vUV = aUV;
                gl_Position = uProj * vec4(aPos.xy, 0.0, 1.0);
            }
        )";

        const char* fs = R"(
            #version 330 core
            in vec2 vUV;
            uniform sampler2D uTex;
            out vec4 FragColor;
            void main(){
	                FragColor = texture(uTex, vec2(vUV.x, 1.0 - vUV.y));
            }
        )";

        GLuint v = Compile(GL_VERTEX_SHADER, vs);
        GLuint f = Compile(GL_FRAGMENT_SHADER, fs);
        if (!v || !f)
            return false;

        s.program = glCreateProgram();
        glAttachShader(s.program, v);
        glAttachShader(s.program, f);
        glLinkProgram(s.program);
        glDeleteShader(v);
        glDeleteShader(f);

        GLint ok = 0;
        glGetProgramiv(s.program, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            char log[2048] = {};
            glGetProgramInfoLog(s.program, sizeof(log), nullptr, log);
            std::cerr << "Program link failed: " << log << "\n";
            glDeleteProgram(s.program);
            s.program = 0;
            return false;
        }

        s.uProjLoc = glGetUniformLocation(s.program, "uProj");
        s.uTexLoc = glGetUniformLocation(s.program, "uTex");

        glGenVertexArrays(1, &s.vao);
        glGenBuffers(1, &s.vbo);
        glBindVertexArray(s.vao);
        glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        s.ready = true;
        return true;
    }

    void SetProjectionUniform(int viewportW, int viewportH)
    {
        GLState& s = GetGL();

        // Top-left origin, y goes down.
        const float sx = 2.0f / static_cast<float>(viewportW);
        const float sy = -2.0f / static_cast<float>(viewportH);

        const float proj[16] = {
            sx,  0,  0,  0,
            0,  sy,  0,  0,
            0,   0,  1,  0,
           -1,   1,  0,  1
        };

        glUniformMatrix4fv(s.uProjLoc, 1, GL_FALSE, proj);
    }
}

Texture::~Texture()
{
    destroy();
}

Texture::Texture(Texture&& other) noexcept
{
    *this = std::move(other);
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        destroy();
        mTextureId = other.mTextureId;
        mWidth = other.mWidth;
        mHeight = other.mHeight;

        other.mTextureId = 0;
        other.mWidth = 0;
        other.mHeight = 0;
    }
    return *this;
}

bool Texture::loadFromBMP(SDL_Renderer* /*renderer*/, const std::string& filePath, bool enableColorKey)
{
    destroy();

    const std::string resolvedPath = ResolveAssetPath(filePath);
    SDL_Surface* loaded = SDL_LoadBMP(resolvedPath.c_str());
    if (!loaded)
    {
        std::cerr << "SDL_LoadBMP failed for '" << resolvedPath << "': " << SDL_GetError() << "\n";
        return false;
    }

    SDL_Surface* surface = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loaded);
    loaded = nullptr;
    if (!surface)
    {
        std::cerr << "SDL_ConvertSurface failed for '" << resolvedPath << "': " << SDL_GetError() << "\n";
        return false;
    }

    if (enableColorKey)
        ApplyMagentaTransparency(surface, 22);

    mWidth = surface->w;
    mHeight = surface->h;

    // Upload to OpenGL
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_DestroySurface(surface);
    return mTextureId != 0;
}

void Texture::destroy()
{
    if (mTextureId != 0)
    {
        glDeleteTextures(1, &mTextureId);
        mTextureId = 0;
    }
    mWidth = 0;
    mHeight = 0;
}

bool Texture::render(SDL_Renderer* renderer, const SDL_FRect* srcRect, const SDL_FRect* dstRect) const
{
    return renderEx(renderer, srcRect, dstRect, 0.0, nullptr, SDL_FLIP_NONE);
}

bool Texture::renderEx(SDL_Renderer* /*renderer*/, const SDL_FRect* srcRect, const SDL_FRect* dstRect, double angle, const SDL_FPoint* center, SDL_FlipMode flip) const
{
    if (mTextureId == 0 || !dstRect)
        return false;

    if (!EnsureGLReady())
        return false;

    // Viewport
    GLint vp[4] = { 0,0,0,0 };
    glGetIntegerv(GL_VIEWPORT, vp);
    const int vw = vp[2];
    const int vh = vp[3];

    GLState& s = GetGL();
    glUseProgram(s.program);
    SetProjectionUniform(vw, vh);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glUniform1i(s.uTexLoc, 0);

    // UVs
    float sx = 0.0f, sy = 0.0f, sw = static_cast<float>(mWidth), sh = static_cast<float>(mHeight);
    if (srcRect)
    {
        sx = srcRect->x;
        sy = srcRect->y;
        sw = srcRect->w;
        sh = srcRect->h;
    }

    float u0 = sx / static_cast<float>(mWidth);
    float u1 = (sx + sw) / static_cast<float>(mWidth);

    float vTop = 1.0f - (sy / static_cast<float>(mHeight));
    float vBot = 1.0f - ((sy + sh) / static_cast<float>(mHeight));

    float v0 = vTop;
    float v1 = vBot;

    const bool flipH = (flip & SDL_FLIP_HORIZONTAL) != 0;
    const bool flipV = (flip & SDL_FLIP_VERTICAL) != 0;
    if (flipH) std::swap(u0, u1);
    if (flipV) std::swap(v0, v1);

    const float dx = dstRect->x;
    const float dy = dstRect->y;
    const float dw = dstRect->w;
    const float dh = dstRect->h;

    const float pivotX = center ? center->x : (dw * 0.5f);
    const float pivotY = center ? center->y : (dh * 0.5f);
    const float worldPivotX = dx + pivotX;
    const float worldPivotY = dy + pivotY;

    const float rad = static_cast<float>(angle * (3.14159265358979323846 / 180.0));
    const float c = std::cos(rad);
    const float sA = std::sin(rad);

    auto rot = [&](float lx, float ly, float& outX, float& outY)
    {
        const float rx = lx * c - ly * sA;
        const float ry = lx * sA + ly * c;
        outX = worldPivotX + rx;
        outY = worldPivotY + ry;
    };

    float x0, y0, x1p, y1p, x2, y2, x3, y3;  //vert positions
    // local coords relative to pivot
    rot(0.0f - pivotX, 0.0f - pivotY, x0, y0);
    rot(dw   - pivotX, 0.0f - pivotY, x1p, y1p);
    rot(dw   - pivotX, dh   - pivotY, x2, y2);
    rot(0.0f - pivotX, dh   - pivotY, x3, y3);

    // 2 triangles
    const float verts[6 * 4] = {    //x,y spacial coordinates, u,v, texture coordinates
        x0,  y0,  u0, v0,
        x1p, y1p, u1, v0,
        x2,  y2,  u1, v1,

        x0,  y0,  u0, v0,
        x2,  y2,  u1, v1,
        x3,  y3,  u0, v1,
    };

    glBindVertexArray(s.vao);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    return true;
}
