#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;

class RigidbodyComponent : public Component
{
public:
    RigidbodyComponent()
        : Component(ComponentType::RIGIDBODY)
    {
    }

    void init() override {}

    void update(float deltaTime) override {}

    void render(DeviceContext& deviceContext) override {}

    void destroy() override {}

public:
    float mass = 1.0f;

    EU::Vector3 velocity = EU::Vector3(0.0f, 0.0f, 0.0f);

    bool useGravity = true;

    bool isKinematic = false;

    bool isGrounded = false;
};