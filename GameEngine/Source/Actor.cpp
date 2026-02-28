#include "Actor.h"

#include <vector>

#include "Level.h"
#include "Texture.h"
#include "PhysicsHelpers.h"
#include "PhysicsWorld.h"

#include <SDL3/SDL.h>

Actor::~Actor()
{
    destroyBody();
}

void Actor::beginPlay() {}
void Actor::handleEvent(const SDL_Event& /*e*/) {}
void Actor::prePhysicsUpdate() {}

void Actor::update(float deltaTime)
{
    if (hasSprite)
        animator.update(deltaTime);

    if (blinkTime > 0.0f)
    {
        blinkTime -= deltaTime;
        blinkAcc += deltaTime;
        while (blinkAcc >= blinkInterval)
        {
            blinkAcc -= blinkInterval;
            blinkVisible = !blinkVisible;
        }
        if (blinkTime <= 0.0f)
        {
            blinkTime = 0.0f;
            blinkVisible = true;
            blinkAcc = 0.0f;
        }
    }
}

void Actor::postPhysicsUpdate() // gets actor position from box 2d and updates the actor to keep everything aligned
{
    if (b2Body_IsValid(bodyId))
    {
        const b2Vec2 p = b2Body_GetPosition(bodyId);
        float cx = 0.0f, cy = 0.0f;
        GEPhysics::metersToPixels(p, cx, cy);

        if (hasSprite)
        {
            const float w = spriteSheet.getFrameWidth() * spriteScaleX;
            const float h = spriteSheet.getFrameHeight() * spriteScaleY;
            x = cx - w * 0.5f;
            y = cy - h * 0.5f;
        }
        else
        {
            x = cx;
            y = cy;
        }
    }
}

void Actor::render()
{
    if (blinkTime > 0.0f && !blinkVisible)
        return;

    const double angle = static_cast<double>(renderRotationDeg);

    if (simpleTex && simpleTex->isValid() && simpleSrcW > 0 && simpleSrcH > 0)
    {
        SDL_FRect src{ static_cast<float>(simpleSrcX), static_cast<float>(simpleSrcY), static_cast<float>(simpleSrcW), static_cast<float>(simpleSrcH) };
        SDL_FRect dst{ x, y, (drawW > 0.0f) ? drawW : (static_cast<float>(simpleSrcW) * spriteScaleX), (drawH > 0.0f) ? drawH : (static_cast<float>(simpleSrcH) * spriteScaleY) };

		SDL_FlipMode sdlFlip = SDL_FLIP_NONE;
		switch (simpleFlip)
		{
		case FlipMode::Horizontal: sdlFlip = SDL_FLIP_HORIZONTAL; break;
		case FlipMode::Vertical: sdlFlip = SDL_FLIP_VERTICAL; break;
		case FlipMode::Both: sdlFlip = static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL); break;
		default: sdlFlip = SDL_FLIP_NONE; break;
		}

		simpleTex->renderEx(nullptr, &src, &dst, angle, nullptr, sdlFlip);
        return;
    }

    if (!hasSprite || !spriteSheet.isValid())
        return;

    const int frame = (staticFrame >= 0) ? staticFrame : animator.getFrame();
	    spriteSheet.renderFrameEx(nullptr, frame, x, y, spriteScaleX, spriteScaleY, angle, FlipMode::None);
}

void Actor::startBlink(float seconds, float intervalSeconds) //starts the blinking effect
{
    blinkTime = seconds;
    blinkInterval = (intervalSeconds > 0.0f) ? intervalSeconds : 0.08f;
    blinkAcc = 0.0f;
    blinkVisible = true;
}

void Actor::onCollisionBegin(Actor* /*other*/) {} //callbacks from colisions to be used in actor code 
void Actor::onCollisionEnd(Actor* /*other*/) {}

void Actor::setRenderer(SDL_Renderer* inRenderer)
{
    renderer = inRenderer;
}

bool Actor::loadSpriteSheet(const std::string& bmpPath, int frameW, int frameH, bool enableColorKey)
{
    hasSprite = spriteSheet.load(nullptr, bmpPath, frameW, frameH, enableColorKey);
    return hasSprite;
}

void Actor::setSimpleSprite(Texture* tex, int srcX, int srcY, int srcW, int srcH, bool flipH) //draws a simple sprite without animations
{
	setSimpleSpriteEx(tex, srcX, srcY, srcW, srcH, flipH ? FlipMode::Horizontal : FlipMode::None);
}

void Actor::setSimpleSpriteEx(Texture* tex, int srcX, int srcY, int srcW, int srcH, FlipMode flip)
{
    simpleTex = tex;
    simpleSrcX = srcX;
    simpleSrcY = srcY;
    simpleSrcW = srcW;
    simpleSrcH = srcH;
    simpleFlip = flip;
}

void Actor::clearSimpleSprite()
{
    simpleTex = nullptr;
    simpleSrcX = simpleSrcY = 0;
    simpleSrcW = simpleSrcH = 0;
	simpleFlip = FlipMode::None;
}

void Actor::addRowClip(const std::string& name, int row, int startCol, int count, float fps, bool loop) //create clip
{
    if (!hasSprite)
        return;

    const int cols = spriteSheet.getColumns();
    std::vector<int> frames;
    frames.reserve(count);
    for (int i = 0; i < count; ++i)
        frames.push_back(row * cols + (startCol + i));

    AnimationClip clip;
    clip.frames = frames;
    clip.fps = fps;
    clip.loop = loop;

    animator.addClip(name, clip);
}

void Actor::addClipFrames(const std::string& name, const std::vector<int>& frames, float fps, bool loop) // creates clip but allows to set the frame order in a index array
{
    if (!hasSprite)
        return;

    AnimationClip clip;
    clip.frames = frames;
    clip.fps = fps;
    clip.loop = loop;

    animator.addClip(name, clip);
}


bool Actor::playClip(const std::string& clipName, bool restart) //play animation
{
    return animator.play(clipName, restart);
}

void Actor::destroyBody()
{
    if (b2Body_IsValid(bodyId))
    {
        b2DestroyBody(bodyId);
        bodyId = b2BodyId{};
    }
}

bool Actor::createBody(b2BodyType type, bool fixedRotation) // adds a body to the actor so it is affected by phisics
{
    if (level == nullptr)
        return false;

    PhysicsWorld* pw = level->getPhysicsWorld();
    if (pw == nullptr || !pw->isValid())
        return false;

    b2BodyDef def = b2DefaultBodyDef();
    def.type = type;
    float cx = x;
    float cy = y;
    if (hasSprite)
    {
        const float w = spriteSheet.getFrameWidth() * spriteScaleX;
        const float h = spriteSheet.getFrameHeight() * spriteScaleY;
        cx = x + w * 0.5f;
        cy = y + h * 0.5f;
    }
    def.position = GEPhysics::pixelsToMeters(cx, cy);
    // Lock rotation if requested (Box2D 3 uses motion locks)
    def.motionLocks.angularZ = fixedRotation;

    bodyId = b2CreateBody(pw->getId(), &def);
    if (!b2Body_IsValid(bodyId))
        return false;

    b2Body_SetUserData(bodyId, this);
    return true;
}

b2ShapeId Actor::addBoxCollider(float halfWpx, float halfHpx, bool isSensor, float density, uint64_t categoryBits, uint64_t maskBits, bool enableContactEvents) //adds a box collider to the actor
{
    if (!b2Body_IsValid(bodyId))
        return b2ShapeId{};

    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density = density;
    sd.isSensor = isSensor;
    sd.enableContactEvents = enableContactEvents;
    sd.filter.categoryBits = categoryBits;
    sd.filter.maskBits = maskBits;
    sd.userData = this;

    const float hw = halfWpx / GEPhysics::PixelsPerMeter;
    const float hh = halfHpx / GEPhysics::PixelsPerMeter;
    b2Polygon box = b2MakeBox(hw, hh);
    return b2CreatePolygonShape(bodyId, &sd, &box);
}

void Actor::setLinearVelocityPixels(float vx, float vy) //sets actor velocity
{
    if (!b2Body_IsValid(bodyId))
        return;

    b2Body_SetLinearVelocity(bodyId, GEPhysics::pixelsToMetersVec(vx, vy));

    // Make sure the body actually moves
    b2Body_SetAwake(bodyId, true);
}

void Actor::getLinearVelocityPixels(float& outVx, float& outVy) const
{
    outVx = 0.0f;
    outVy = 0.0f;

    if (!b2Body_IsValid(bodyId))
        return;

    const b2Vec2 v = b2Body_GetLinearVelocity(bodyId);
    GEPhysics::metersToPixels(v, outVx, outVy);
}

void Actor::teleportPixels(float px, float py, bool zeroVelocity)// sets the actor position
{
    setPosition(px, py);

    if (!b2Body_IsValid(bodyId))
        return;

    float cx = x;
    float cy = y;
    if (hasSprite)
    {
        const float w = spriteSheet.getFrameWidth() * spriteScaleX;
        const float h = spriteSheet.getFrameHeight() * spriteScaleY;
        cx += w * 0.5f;
        cy += h * 0.5f;
    }

    const b2Transform tr = b2Body_GetTransform(bodyId);
    b2Body_SetTransform(bodyId, GEPhysics::pixelsToMeters(cx, cy), tr.q);

    if (zeroVelocity)
        b2Body_SetLinearVelocity(bodyId, b2Vec2{ 0.0f, 0.0f });

    b2Body_SetAwake(bodyId, true);
}
