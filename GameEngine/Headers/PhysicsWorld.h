#pragma once

#include <box2d/box2d.h>

#include "Actor.h"

// box2d phisics
class PhysicsWorld
{
public:
    ~PhysicsWorld()
    {
        if (b2World_IsValid(worldId))
            b2DestroyWorld(worldId);
    }

    bool init(float gx, float gy)
    {
        if (b2World_IsValid(worldId))
            b2DestroyWorld(worldId);

        b2WorldDef def = b2DefaultWorldDef();
        def.gravity = b2Vec2{ gx, gy };
        worldId = b2CreateWorld(&def);
        return b2World_IsValid(worldId);
    }

    bool isValid() const { return b2World_IsValid(worldId); }
    b2WorldId getId() const { return worldId; }

    void step(float dt, int subSteps = 4)
    {
        if (!isValid()) return;

        b2World_Step(worldId, dt, subSteps);

        const b2ContactEvents ev = b2World_GetContactEvents(worldId);
        auto getActor = [](b2ShapeId sid) -> Actor*
        {
            return b2Shape_IsValid(sid) ? static_cast<Actor*>(b2Shape_GetUserData(sid)) : nullptr;
        };

        for (int i = 0; i < ev.beginCount; ++i)
        {
            const b2ContactBeginTouchEvent& e = ev.beginEvents[i];
            Actor* a = getActor(e.shapeIdA);
            Actor* b = getActor(e.shapeIdB);
            if (a) a->onCollisionBegin(b);
            if (b) b->onCollisionBegin(a);
        }
        for (int i = 0; i < ev.endCount; ++i)
        {
            const b2ContactEndTouchEvent& e = ev.endEvents[i];
            Actor* a = getActor(e.shapeIdA);
            Actor* b = getActor(e.shapeIdB);
            if (a) a->onCollisionEnd(b);
            if (b) b->onCollisionEnd(a);
        }
    }

private:
    b2WorldId worldId{};
};
