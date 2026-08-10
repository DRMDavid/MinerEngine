#pragma once

#include "Prerequisites.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h"
#include "ECS/RigidbodyComponent.h"
#include "ECS/BoxColliderComponent.h"

/**
 * @class PhysicsSystem
 * @brief Sistema encargado de la simulación de físicas y detección de colisiones.
 * @details Examina los actores de la escena, actualiza las posiciones según sus componentes de física (RigidbodyComponent) y evalúa intersecciones entre colisionadores (BoxColliderComponent).
 */
class PhysicsSystem
{
public:
    /**
     * @brief Actualiza la lógica de físicas y resuelve colisiones para todos los actores dados.
     * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
     * @param actors Vector de punteros compartidos a los actores de la escena a procesar.
     */
    void update(
        float deltaTime,
        std::vector<EU::TSharedPointer<Actor>>& actors
    );

private:
    /**
     * @brief Realiza una prueba de colisión entre dos cajas alineadas con los ejes (AABB).
     * @param positionA Centro o posición global de la primera caja.
     * @param sizeA Dimensiones (ancho, alto, profundidad) de la primera caja.
     * @param positionB Centro o posición global de la segunda caja.
     * @param sizeB Dimensiones de la segunda caja.
     * @return True si existe superposición entre las dos cajas, false en caso contrario.
     */
    bool checkAABB(
        const EU::Vector3& positionA,
        const EU::Vector3& sizeA,
        const EU::Vector3& positionB,
        const EU::Vector3& sizeB
    ) const;
};