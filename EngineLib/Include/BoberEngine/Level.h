#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <unordered_map>

#include "Actor.h"

#include "PhysicsWorld.h"

class Texture;

class Level
{
public:
    Level() = default;
    ~Level();

    void beginPlay();
    void handleEvent(const SDL_Event& e);
    void update(float deltaTime);
    void render();

    void setRenderer(SDL_Renderer* inRenderer);

    SDL_Renderer* getRenderer() const { return renderer; }

    void createWorldBounds(float thicknessPx, uint64_t categoryBits, uint64_t maskBits);

    Texture* loadTexture(const std::string& bmpPath, bool colorKey = true);

    void setBackground(const std::string& bmpPath, bool colorKey = false);

    // Add an actor to this level
    void addActor(Actor* actor);

    template <typename T, typename... Args>
    T* spawnActor(Args&&... args)
    {
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = obj.get();

        addActor(raw);

        if (isIteratingActors)
            pendingOwnedActors.push_back(std::move(obj));
        else
            ownedActors.push_back(std::move(obj));

        return raw;
    }

    void setViewportPixels(int width, int height);

    int getViewportWidth() const { return viewportW; }
    int getViewportHeight() const { return viewportH; }

    PhysicsWorld* getPhysicsWorld() const { return physics.get(); }

protected:

    virtual void onBeginPlay() {}
    virtual void onHandleEvent(const SDL_Event&) {}
    virtual void onUpdate(float) {}
    virtual void onRender() {}

    //Box2D world
    std::unique_ptr<PhysicsWorld> physics;
    float physicsAccumulator = 0.0f;
    float fixedDelta = 1.0f / 60.0f;

    int viewportW = 1280;
    int viewportH = 720;

    bool hasBegunPlay = false;

private:
    SDL_Renderer* renderer = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;

    Texture* backgroundTex = nullptr;

    // main list of active actors 
    std::vector<Actor*> actors;

    // actors spawned from spawnActor()
    std::vector<std::unique_ptr<Actor>> ownedActors;


    bool isIteratingActors = false;
    std::vector<Actor*> pendingActors;
    std::vector<std::unique_ptr<Actor>> pendingOwnedActors;

    void commitPendingActors();
    void removeDestroyedActors();
};
