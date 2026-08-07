#pragma once

#include "Prerequisites.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h"
#include "ECS/RotateBehaviorComponent.h"
#include <xnamath.h>

/**
 * @class BehaviorSystem
 * @brief Actualiza los componentes de comportamiento durante el modo Play.
 */
class BehaviorSystem
{
public:

	/**
	 * @brief Ejecuta los comportamientos activos de los actores.
	 *
	 * @param deltaTime Tiempo transcurrido desde el último frame.
	 * @param actors Actores registrados en la escena.
	 */
	void update(
		float deltaTime,
		std::vector<EU::TSharedPointer<Actor>>& actors
	);
};