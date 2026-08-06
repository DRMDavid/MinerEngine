#include "BehaviorSystem.h"

/**
 * @brief Actualiza los comportamientos configurados en los actores.
 *
 * Recorre la escena, busca actores con Transform y
 * RotateBehaviorComponent, y aplica la rotación configurada.
 *
 * @param deltaTime Tiempo transcurrido desde el último frame.
 * @param actors Actores registrados en la escena.
 */
void
BehaviorSystem::update(
	float deltaTime,
	std::vector<EU::TSharedPointer<Actor>>& actors
)
{
	for (auto& actor : actors)
	{
		// Ignorar actores inválidos.
		if (actor.isNull())
		{
			continue;
		}

		// Obtener el Transform del actor.
		auto transform =
			actor->getComponent<Transform>();

		// Obtener el comportamiento de rotación.
		auto rotateBehavior =
			actor->getComponent<RotateBehaviorComponent>();

		// El actor necesita ambos componentes.
		if (!transform || !rotateBehavior)
		{
			continue;
		}

		// Ignorar comportamientos desactivados.
		if (!rotateBehavior->isEnabled())
		{
			continue;
		}

		// Leer la rotación actual.
		EU::Vector3 rotation =
			transform->getRotation();

		/**
		 * Calcular el desplazamiento angular del frame.
		 *
		 * direction puede ser:
		 *  1.0f  para sentido positivo.
		 * -1.0f  para sentido contrario.
		 */
		const float rotationAmount =
			rotateBehavior->speed *
			rotateBehavior->direction *
			deltaTime;

		// Aplicar la rotación en los ejes configurados.
		rotation.x +=
			rotateBehavior->axis.x *
			rotationAmount;

		rotation.y +=
			rotateBehavior->axis.y *
			rotationAmount;

		rotation.z +=
			rotateBehavior->axis.z *
			rotationAmount;

		// Guardar la nueva rotación.
		transform->setRotation(rotation);
	}
}