#pragma once

union SDL_Event;
struct SDL_Renderer;
#include "RenderTypes.h"

#include <string>
#include <cstdint>
#include <vector>

#include <box2d/box2d.h>

#include "Animation.h"
#include "SpriteSheet.h"

class Texture;

class Level;

class Actor
{
public:
    virtual ~Actor();

    virtual void beginPlay();
    virtual void handleEvent(const SDL_Event& e);
    virtual void update(float deltaTime);

    // called after physics has stepped
    virtual void prePhysicsUpdate();
    // called before physics has stepped
    virtual void postPhysicsUpdate();

    virtual void render();

    // Collision
    virtual void onCollisionBegin(Actor* other);
    virtual void onCollisionEnd(Actor* other);

    // tags for collisions
    enum class Tag
    {
        None = 0,
        Player,
        Enemy,
        PlayerProjectile,
        EnemyProjectile,
        Effect,
        World,
    };

    void setTag(Tag t) { tag = t; }
    Tag getTag() const { return tag; }

    // Lifetime
    void destroy() { pendingDestroy = true; }
    bool isPendingDestroy() const { return pendingDestroy; }

    // Renderer 
    virtual void setRenderer(SDL_Renderer* inRenderer);
    SDL_Renderer* getRenderer() const { return renderer; }

    // Level ownership
    void setLevel(Level* inLevel) { level = inLevel; }
    Level* getLevel() const { return level; }

    // transform 
    void setPosition(float inX, float inY) { x = inX; y = inY; }
    float getX() const { return x; }
    float getY() const { return y; }

    void setRenderLayer(int layer) { renderLayer = layer; }
    int getRenderLayer() const { return renderLayer; }

    // sprite sheet + animation
    bool loadSpriteSheet(const std::string& bmpPath, int frameW, int frameH, bool enableColorKey = true);
    void addRowClip(const std::string& name, int row, int startCol, int count, float fps, bool loop);
    void addClipFrames(const std::string& name, const std::vector<int>& frames, float fps, bool loop);
    bool playClip(const std::string& clipName, bool restart = true);

    bool isAnimationPlaying() const { return animator.isPlaying(); }
    bool isAnimationFinished() const { return animator.isFinished(); }
    void setSpriteScale(float s) { spriteScaleX = s; spriteScaleY = s; }
    void setSpriteScale(float sx, float sy) { spriteScaleX = sx; spriteScaleY = sy; }
    void setStaticFrame(int frame) { staticFrame = frame; }

    // Per-actor render rotation
    void setRenderRotationDegrees(float deg) { renderRotationDeg = deg; }
    float getRenderRotationDegrees() const { return renderRotationDeg; }
    //smaller name function
    void setRotationDegrees(float deg) { setRenderRotationDegrees(deg); }

    //  sprite 
    void setSimpleSprite(Texture* tex, int srcX, int srcY, int srcW, int srcH, bool flipH = false);
    void setSimpleSpriteEx(Texture* tex, int srcX, int srcY, int srcW, int srcH, FlipMode flip);
    void clearSimpleSprite();

    void startBlink(float seconds, float intervalSeconds = 0.08f);
    void setDrawSize(float w, float h) { drawW = w; drawH = h; }

    //Box2D helpers
    bool hasBody() const { return b2Body_IsValid(bodyId); }
    b2BodyId getBodyId() const { return bodyId; }
    void destroyBody();

    // Creates a body at the current actor position
    bool createBody(b2BodyType type, bool fixedRotation = true);
    bool createDynamicBody(bool fixedRotation = true) { return createBody(b2_dynamicBody, fixedRotation); }
    bool createStaticBody(bool fixedRotation = true) { return createBody(b2_staticBody, fixedRotation); }
    // Creates a box collider centered on the body.
    b2ShapeId addBoxCollider(
        float halfWpx,
        float halfHpx,
        bool isSensor = false,
        float density = 1.0f,
        uint64_t categoryBits = 0xFFFF,
        uint64_t maskBits = 0xFFFF,
        bool enableContactEvents = true);
    // for movement
    void setLinearVelocityPixels(float vx, float vy);

    // Gets the body linear velocity
    // If the actor has no body returns 0,0
    void getLinearVelocityPixels(float& outVx, float& outVy) const;

    void teleportPixels(float px, float py, bool zeroVelocity = true);

protected:
    SDL_Renderer* renderer = nullptr;
    Level* level = nullptr;

    float x = 0.0f;
    float y = 0.0f;

    // Sprite/animation state
    SpriteSheet spriteSheet;
    Animator animator;
    float spriteScaleX = 1.0f;
    float spriteScaleY = 1.0f;
    bool hasSprite = false;
    int staticFrame = -1;

    float renderRotationDeg = 0.0f;

    float blinkTime = 0.0f;
    float blinkInterval = 0.08f;
    float blinkAcc = 0.0f;
    bool blinkVisible = true;

    // Physics state
    b2BodyId bodyId{};

private:
    Tag tag = Tag::None;
    bool pendingDestroy = false;

    int renderLayer = 0;

    Texture* simpleTex = nullptr;
    int simpleSrcX = 0;
    int simpleSrcY = 0;
    int simpleSrcW = 0;
    int simpleSrcH = 0;
    FlipMode simpleFlip = FlipMode::None;
    float drawW = 0.0f;
    float drawH = 0.0f;
};
