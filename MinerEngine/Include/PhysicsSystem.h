#pragma once

#include "Prerequisites.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h"
#include "ECS/RigidbodyComponent.h"
#include "ECS/BoxColliderComponent.h"

class PhysicsSystem
{
public:
    void update(
        float deltaTime,
        std::vector<EU::TSharedPointer<Actor>>& actors
    );

private:
    bool checkAABB(
        const EU::Vector3& positionA,
        const EU::Vector3& sizeA,
        const EU::Vector3& positionB,
        const EU::Vector3& sizeB
    ) const;
};