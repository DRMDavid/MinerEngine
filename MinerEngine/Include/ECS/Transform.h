#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class Transform : public Component {
public:
  // Constructor
  Transform() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), Component(ComponentType::TRANSFORM) {
    matrix = XMMatrixIdentity();
  }

  // Inicialización
  void init() override {
    matrix = XMMatrixIdentity();
  }

  // Actualización: Calcula la matriz matemática
  void update(float deltaTime) override {
    // 1. Matriz de Escala
    XMMATRIX mScale = XMMatrixScaling(scale.x, scale.y, scale.z);
    // 2. Matriz de Rotación
    XMMATRIX mRot = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    // 3. Matriz de Traslación
    XMMATRIX mTrans = XMMatrixTranslation(position.x, position.y, position.z);

    // Orden: Escala -> Rotación -> Traslación
    matrix = mScale * mRot * mTrans;
  }

  // Render (No dibuja nada por sí mismo)
  void render(DeviceContext& deviceContext) override {}
  void destroy() override {}

  // --- Getters y Setters para ImGui ---
  const EU::Vector3& getPosition() const { return position; }
  void setPosition(const EU::Vector3& p) { position = p; }

  const EU::Vector3& getRotation() const { return rotation; }
  void setRotation(const EU::Vector3& r) { rotation = r; }

  const EU::Vector3& getScale() const { return scale; }
  void setScale(const EU::Vector3& s) { scale = s; }

public:
  // Variable pública para que Actor.cpp la lea
  XMMATRIX matrix;

private:
  EU::Vector3 position;
  EU::Vector3 rotation;
  EU::Vector3 scale;
};