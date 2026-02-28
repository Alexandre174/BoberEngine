#include "Level.h"

#include "Input.h"
#include "PhysicsWorld.h"
#include "PhysicsHelpers.h"
#include "Texture.h"

#include <SDL3/SDL.h>
#include <algorithm>

Level::~Level() = default;

void Level::setRenderer(SDL_Renderer* inRenderer) // give actors access to the renderer
{
    renderer = inRenderer;

    const bool prev = isIteratingActors;
    isIteratingActors = true;

    for (Actor* actor : actors)
        if (actor) actor->setRenderer(inRenderer);

    isIteratingActors = prev;

    for (Actor* actor : pendingActors)
        if (actor) actor->setRenderer(inRenderer);
}

Texture* Level::loadTexture(const std::string& bmpPath, bool colorKey)
{
    if (!renderer)
        return nullptr;

    auto it = textures.find(bmpPath);
    if (it != textures.end())
        return it->second.get();

    auto tex = std::make_unique<Texture>();
    if (!tex->loadFromBMP(renderer, bmpPath, colorKey))
        return nullptr;

    Texture* raw = tex.get();
    textures.emplace(bmpPath, std::move(tex));
    return raw;
}

void Level::setBackground(const std::string& bmpPath, bool colorKey)
{
    backgroundTex = loadTexture(bmpPath, colorKey);
}



void Level::addActor(Actor* actor)
{
    if (!actor) return;

    actor->setLevel(this);

    if (renderer)
        actor->setRenderer(renderer);

    // If the level already started begin this actor immediately
    if (hasBegunPlay)
        actor->beginPlay();

    if (isIteratingActors)
        pendingActors.push_back(actor);
    else
        actors.push_back(actor);
}

void Level::commitPendingActors()
{
    if (!pendingActors.empty())
    {
        actors.insert(actors.end(), pendingActors.begin(), pendingActors.end());
        pendingActors.clear();
    }

    if (!pendingOwnedActors.empty())
    {
        for (auto& a : pendingOwnedActors)
            ownedActors.push_back(std::move(a));
        pendingOwnedActors.clear();
    }
}

void Level::setViewportPixels(int width, int height)
{
    viewportW = width > 0 ? width : viewportW;
    viewportH = height > 0 ? height : viewportH;
}

void Level::beginPlay()
{
    //create physics world
    if (!physics)
    {
        physics = std::make_unique<PhysicsWorld>();
        physics->init(0.0f, 0.0f); // start with no gravity
    }

    hasBegunPlay = true;

    
    onBeginPlay();

    const bool prev = isIteratingActors;
    isIteratingActors = true;

    for (Actor* actor : actors)
        if (actor) actor->beginPlay();

    isIteratingActors = prev;

    // Commit any actors spawned during beginPlay
    commitPendingActors();
    removeDestroyedActors();
}

void Level::handleEvent(const SDL_Event& e)
{
   
    onHandleEvent(e);

    const bool prev = isIteratingActors;
    isIteratingActors = true;

    for (Actor* actor : actors)
        if (actor) actor->handleEvent(e);

    isIteratingActors = prev;

    
    commitPendingActors();
}

void Level::update(float deltaTime)
{
    // check if begin play ran before the first update
    if (!hasBegunPlay)
        beginPlay();

    onUpdate(deltaTime);


    {
        const bool prev = isIteratingActors;
        isIteratingActors = true;

        for (Actor* actor : actors)
            if (actor) actor->update(deltaTime);

        isIteratingActors = prev;
    }

    commitPendingActors();

    
    if (physics && physics->isValid())//physics
    {
        physicsAccumulator += deltaTime;

        int steps = 0;
        const int maxSteps = 5;

        while (physicsAccumulator >= fixedDelta && steps < maxSteps)
        {
            {
                const bool prev = isIteratingActors;
                isIteratingActors = true;

                for (Actor* actor : actors)
                    if (actor) actor->prePhysicsUpdate();

                isIteratingActors = prev;
            }

            physics->step(fixedDelta, 4);
            physicsAccumulator -= fixedDelta;
            steps++;

            commitPendingActors();
        }

        // Sync actor transforms from physics
        {
            const bool prev = isIteratingActors;
            isIteratingActors = true;

            for (Actor* actor : actors)
                if (actor) actor->postPhysicsUpdate();

            isIteratingActors = prev;
        }
    }

    // remove any actors that requested destruction during this frame
    removeDestroyedActors();

    // Commit pending actors 
    commitPendingActors();
}

void Level::removeDestroyedActors()
{

    pendingActors.erase(
        std::remove_if(pendingActors.begin(), pendingActors.end(), [](Actor* a)
            {
                return a == nullptr || a->isPendingDestroy();
            }),
        pendingActors.end());

    pendingOwnedActors.erase(
        std::remove_if(pendingOwnedActors.begin(), pendingOwnedActors.end(), [](const std::unique_ptr<Actor>& a)
            {
                return !a || a->isPendingDestroy();
            }),
        pendingOwnedActors.end());

    // Remove raw pointers
    actors.erase(
        std::remove_if(actors.begin(), actors.end(), [](Actor* a)
            {
                return a == nullptr || a->isPendingDestroy();
            }),
        actors.end());

    // Delete owned actors that are pending destroy.
    ownedActors.erase(
        std::remove_if(ownedActors.begin(), ownedActors.end(), [](const std::unique_ptr<Actor>& a)
            {
                return !a || a->isPendingDestroy();
            }),
        ownedActors.end());
}

void Level::render()
{
    // Background
    if (renderer && backgroundTex && backgroundTex->isValid())
    {
        SDL_FRect dst{ 0.0f, 0.0f, static_cast<float>(viewportW), static_cast<float>(viewportH) };
        backgroundTex->render(renderer, nullptr, &dst);
    }

    onRender();

    // Draw in layer order
    std::stable_sort(actors.begin(), actors.end(), [](const Actor* a, const Actor* b)
    {
        return a->getRenderLayer() < b->getRenderLayer();
    });

    {
        const bool prev = isIteratingActors;
        isIteratingActors = true;

        for (Actor* actor : actors)// render actors
            if (actor) actor->render();

        isIteratingActors = prev;
    }

    commitPendingActors();
}

void Level::createWorldBounds(float thicknessPx, uint64_t categoryBits, uint64_t maskBits) //creates colisions arround the game window
{
    if (!physics || !physics->isValid())
        return;

    const float t = thicknessPx;
    const float vw = static_cast<float>(viewportW);
    const float vh = static_cast<float>(viewportH);

    struct Wall { float cx, cy, w, h; };
    const Wall walls[4] =
    {
        { -t * 0.5f,      vh * 0.5f, t, vh + 2.0f * t },
        { vw + t * 0.5f,  vh * 0.5f, t, vh + 2.0f * t },
        { vw * 0.5f,     -t * 0.5f, vw + 2.0f * t, t },
        { vw * 0.5f,  vh + t * 0.5f, vw + 2.0f * t, t },
    };

    for (const Wall& w : walls)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = GEPhysics::pixelsToMeters(w.cx, w.cy);

        b2BodyId bid = b2CreateBody(physics->getId(), &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 0.0f;
        shapeDef.material.friction = 0.0f;
        shapeDef.material.restitution = 0.0f;
        shapeDef.filter.categoryBits = categoryBits;
        shapeDef.filter.maskBits = maskBits;

        b2Polygon poly = b2MakeBox((w.w * 0.5f) / GEPhysics::PixelsPerMeter, (w.h * 0.5f) / GEPhysics::PixelsPerMeter);
        b2CreatePolygonShape(bid, &shapeDef, &poly);
    }
}
