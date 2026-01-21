#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

class
    Transform : public Component {
public:
    Transform() : position(),
        rotation(),
        scale(),
        matrix(),
        Component(ComponentType::TRANSFORM) {
    }

    void
        init() {
        scale.one();
        matrix = XMMatrixIdentity();
    }

    // -------------------------------------------------------------
    // CORRECCIÓN DE ORDEN DE ROTACIÓN
    // -------------------------------------------------------------
    void
        update(float deltaTime) override {
        // 1. Escala
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

        // 2. Rotación (Manual por ejes para compatibilidad con ImGuizmo)
        // Usamos X * Y * Z (o el orden que prefieras, pero separado) para evitar
        // el comportamiento predeterminado de RollPitchYaw que puede causar gimbal lock visual.
        XMMATRIX rotX = XMMatrixRotationX(rotation.x);
        XMMATRIX rotY = XMMatrixRotationY(rotation.y);
        XMMATRIX rotZ = XMMatrixRotationZ(rotation.z);
        XMMATRIX rotationMatrix = rotX * rotY * rotZ;

        // 3. Traslación
        XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

        // Componer: Scale -> Rotation -> Translation
        matrix = scaleMatrix * rotationMatrix * translationMatrix;
    }

    void
        render(DeviceContext& deviceContext) override {}

    void
        destroy() {}

    const EU::Vector3&
        getPosition() const { return position; }

    void
        setPosition(const EU::Vector3& newPos) { position = newPos; }

    const EU::Vector3&
        getRotation() const { return rotation; }

    void
        setRotation(const EU::Vector3& newRot) { rotation = newRot; }

    const EU::Vector3&
        getScale() const { return scale; }

    void
        setScale(const EU::Vector3& newScale) { scale = newScale; }

    void
        setTransform(const EU::Vector3& newPos,
            const EU::Vector3& newRot,
            const EU::Vector3& newSca) {
        position = newPos;
        rotation = newRot;
        scale = newSca;
    }

    void
        translate(const EU::Vector3& translation);

private:
    EU::Vector3 position;
    EU::Vector3 rotation; // Radianes
    EU::Vector3 scale;

public:
    XMMATRIX matrix;
};