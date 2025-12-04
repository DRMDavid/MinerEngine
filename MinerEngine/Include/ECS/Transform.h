#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

/**
 * @class Transform
 * @brief Gestor de datos espaciales y jerarquía matricial.
 *
 * Este componente almacena la información topológica (posición, rotación, escala)
 * de una entidad y es responsable de construir la Matriz de Mundo (World Matrix)
 * necesaria para ubicar el objeto dentro del espacio 3D durante el renderizado.
 */
class Transform : public Component {
public:
  /**
   * @brief Constructor de inicialización.
   * * Establece la entidad en el origen del mundo (0,0,0) con rotación neutral
   * y escala unitaria. Asigna el tipo de componente como TRANSFORM.
   */
  Transform() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), Component(ComponentType::TRANSFORM) {
    matrix = XMMatrixIdentity();
  }

  /**
   * @brief Restablecimiento de estado.
   * * Reinicia la matriz de transformación a la matriz identidad.
   */
  void init() override {
    matrix = XMMatrixIdentity();
  }

  /**
   * @brief Recálculo de transformaciones geométricas.
   *
   * Reconstruye la matriz final combinando las transformaciones individuales
   * en el orden estándar SRT (Escala * Rotación * Traslación).
   *
   * @param deltaTime Tiempo delta (no utilizado directamente en el cálculo matricial estático).
   */
  void update(float deltaTime) override {
    // Generación de la matriz de escalado (S)
    XMMATRIX mScale = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Generación de la matriz de rotación (R) basada en ángulos de Euler (Pitch, Yaw, Roll)
    XMMATRIX mRot = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

    // Generación de la matriz de traslación (T)
    XMMATRIX mTrans = XMMatrixTranslation(position.x, position.y, position.z);

    // Composición final: W = S * R * T
    matrix = mScale * mRot * mTrans;
  }

  /**
   * @brief Implementación nula de renderizado.
   * * Este componente es puramente lógico-matemático y no emite comandos de dibujo.
   */
  void render(DeviceContext& deviceContext) override {}

  /**
   * @brief Implementación nula de destrucción.
   * * No gestiona memoria dinámica compleja que requiera liberación manual.
   */
  void destroy() override {}

  // --- Accesores de Propiedades Espaciales ---

  /**
   * @brief Obtiene las coordenadas actuales en espacio local/global.
   */
  const EU::Vector3& getPosition() const { return position; }

  /**
   * @brief Modifica la ubicación de la entidad.
   * @param p Nuevo vector de posición.
   */
  void setPosition(const EU::Vector3& p) { position = p; }

  /**
   * @brief Obtiene la orientación actual en ángulos de Euler.
   */
  const EU::Vector3& getRotation() const { return rotation; }

  /**
   * @brief Modifica la orientación de la entidad.
   * @param r Nuevo vector de rotación (en radianes o grados según convención del motor).
   */
  void setRotation(const EU::Vector3& r) { rotation = r; }

  /**
   * @brief Obtiene los factores de dimensión actuales.
   */
  const EU::Vector3& getScale() const { return scale; }

  /**
   * @brief Modifica el tamaño relativo de la entidad.
   * @param s Nuevo vector de escala.
   */
  void setScale(const EU::Vector3& s) { scale = s; }

public:
  /**
   * @brief Matriz de Transformación Mundial (World Matrix).
   * * Almacena el resultado de la composición $S \cdot R \cdot T$.
   * Es accedida directamente por el pipeline gráfico para transformar vértices.
   */
  XMMATRIX matrix;

private:
  EU::Vector3 position; ///< Coordenadas de traslación (X, Y, Z).
  EU::Vector3 rotation; ///< Ángulos de orientación.
  EU::Vector3 scale;    ///< Factores de escalado por eje.
};