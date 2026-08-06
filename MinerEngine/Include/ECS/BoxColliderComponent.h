#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;

class BoxColliderComponent : public Component
{
public:

    BoxColliderComponent()
        : Component(ComponentType::COLLIDER)
    {
    }

    void init() override {}

    void update(float deltaTime) override {}

    void render(DeviceContext& deviceContext) override {}

    void destroy() override {}

public:

    EU::Vector3 center = EU::Vector3(0.0f, 0.0f, 0.0f);

    EU::Vector3 size = EU::Vector3(1.0f, 1.0f, 1.0f);

    bool isTrigger = false;
};