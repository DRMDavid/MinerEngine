#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;

/**
 * @class BoxColliderComponent
 * @brief Componente que representa un colisionador con forma de caja (AABB/OBB) dentro del sistema ECS.
 * @details Hereda de Component y permite gestionar la detección de colisiones mediante dimensiones de caja y estado de disparador (trigger).
 */
class BoxColliderComponent : public Component
{
public:

    /**
     * @brief Constructor por defecto.
     * @details Inicializa el componente asignando el tipo de componente como COLLIDER.
     */
    BoxColliderComponent()
        : Component(ComponentType::COLLIDER)
    {
    }

    /**
     * @brief Inicializa los recursos o el estado del componente.
     */
    void init() override {}

    /**
     * @brief Actualiza la lógica del componente en cada frame.
     * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
     */
    void update(float deltaTime) override {}

    /**
     * @brief Renderiza la representación visual o de depuración del colisionador.
     * @param deviceContext Referencia al contexto del dispositivo para la renderización.
     */
    void render(DeviceContext& deviceContext) override {}

    /**
     * @brief Libera los recursos asociados al componente antes de su destrucción.
     */
    void destroy() override {}

public:

    /**
     * @brief Centro relativo del colisionador en el espacio local del objeto.
     */
    EU::Vector3 center = EU::Vector3(0.0f, 0.0f, 0.0f);

    /**
     * @brief Dimensiones (ancho, alto, profundidad) de la caja colisionadora.
     */
    EU::Vector3 size = EU::Vector3(1.0f, 1.0f, 1.0f);

    /**
     * @brief Indica si el colisionador actúa como un disparador (trigger) que detecta superposiciones sin respuesta física.
     */
    bool isTrigger = false;
};