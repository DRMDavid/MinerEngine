#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"
#include <xnamath.h>

class DeviceContext;

/**
 * @class RotateBehaviorComponent
 * @brief Componente de comportamiento que rota automáticamente un actor durante Play.
 *
 * Permite configurar el eje, la velocidad y el sentido de rotación
 * desde el Inspector del editor.
 */
class RotateBehaviorComponent : public Component
{
public:

    /**
     * @brief Constructor por defecto.
     */
    RotateBehaviorComponent()
        : Component(ComponentType::ROTATE_BEHAVIOR)
    {
    }

    /**
     * @brief Inicializa el componente.
     */
    void init() override {}

    /**
     * @brief Actualización propia del componente.
     *
     * La rotación real se aplicará desde el sistema de comportamiento,
     * porque este componente no conoce directamente al Transform del actor.
     *
     * @param deltaTime Tiempo transcurrido desde el último frame.
     */
    void update(float deltaTime) override {}

    /**
     * @brief No necesita renderizado directo.
     */
    void render(DeviceContext& deviceContext) override {}

    /**
     * @brief No contiene recursos que liberar.
     */
    void destroy() override {}

public:

    /**
     * @brief Eje local sobre el que se aplicará la rotación.
     *
     * Ejemplos:
     * X = (1, 0, 0)
     * Y = (0, 1, 0)
     * Z = (0, 0, 1)
     */
    EU::Vector3 axis =
        EU::Vector3(0.0f, 1.0f, 0.0f);

    /**
     * @brief Velocidad de rotación.
     *
     * Usa las mismas unidades de rotación que tu Transform.
     */
    float speed = 45.0f;

    /**
     * @brief Sentido de rotación.
     *
     * 1.0f  = sentido positivo.
     * -1.0f = sentido contrario.
     */
    float direction = 1.0f;
};