#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;

/**
 * @class RigidbodyComponent
 * @brief Componente que representa un cuerpo rígido para el sistema de físicas en la arquitectura ECS.
 * @details Hereda de Component y gestiona propiedades físicas fundamentales como masa, velocidad, aceleración por gravedad y comportamiento cinemático.
 */
class RigidbodyComponent : public Component
{
public:

    /**
     * @brief Constructor por defecto.
     * @details Inicializa el componente definiendo su tipo como RIGIDBODY.
     */
    RigidbodyComponent()
        : Component(ComponentType::RIGIDBODY)
    {
    }

    /**
     * @brief Inicializa los recursos o el estado del componente.
     */
    void init() override {}

    /**
     * @brief Actualiza la física y el movimiento del componente en cada frame.
     * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
     */
    void update(float deltaTime) override {}

    /**
     * @brief Renderiza elementos visuales de depuración relacionados con la física (p. ej., vectores de velocidad).
     * @param deviceContext Referencia al contexto del dispositivo para la renderización.
     */
    void render(DeviceContext& deviceContext) override {}

    /**
     * @brief Libera los recursos asociados al componente antes de su destrucción.
     */
    void destroy() override {}

public:

    /**
     * @brief Masa del objeto en kilogramos (kg).
     */
    float mass = 1.0f;

    /**
     * @brief Vector de velocidad lineal actual del cuerpo en el espacio tridimensional.
     */
    EU::Vector3 velocity = EU::Vector3(0.0f, 0.0f, 0.0f);

    /**
     * @brief Indica si el cuerpo debe verse afectado por la fuerza de gravedad.
     */
    bool useGravity = true;

    /**
     * @brief Indica si el cuerpo es cinemático.
     * @details Si es true, el objeto se moverá mediante transformaciones directas o código en lugar de responder a fuerzas físicas y colisiones.
     */
    bool isKinematic = false;

    /**
     * @brief Indica si el cuerpo se encuentra actualmente apoyado sobre una superficie rígida (suelo).
     */
    bool isGrounded = false;
};