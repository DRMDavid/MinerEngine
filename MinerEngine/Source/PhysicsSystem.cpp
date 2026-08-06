#include "PhysicsSystem.h"

/**
 * @brief Comprueba si dos cajas alineadas con los ejes se superponen.
 *
 * @param positionA Centro mundial de la primera caja.
 * @param sizeA Tamaño total de la primera caja.
 * @param positionB Centro mundial de la segunda caja.
 * @param sizeB Tamaño total de la segunda caja.
 *
 * @return true si las cajas se superponen en X, Y y Z.
 */
bool
PhysicsSystem::checkAABB(
	const EU::Vector3& positionA,
	const EU::Vector3& sizeA,
	const EU::Vector3& positionB,
	const EU::Vector3& sizeB
) const
{
	const EU::Vector3 halfA(
		sizeA.x * 0.5f,
		sizeA.y * 0.5f,
		sizeA.z * 0.5f
	);

	const EU::Vector3 halfB(
		sizeB.x * 0.5f,
		sizeB.y * 0.5f,
		sizeB.z * 0.5f
	);

	const bool overlapX =
		positionA.x - halfA.x <= positionB.x + halfB.x &&
		positionA.x + halfA.x >= positionB.x - halfB.x;

	const bool overlapY =
		positionA.y - halfA.y <= positionB.y + halfB.y &&
		positionA.y + halfA.y >= positionB.y - halfB.y;

	const bool overlapZ =
		positionA.z - halfA.z <= positionB.z + halfB.z &&
		positionA.z + halfA.z >= positionB.z - halfB.z;

	return overlapX && overlapY && overlapZ;
}

/**
 * @brief Actualiza los cuerpos físicos de la escena.
 *
 * Aplica gravedad a los cuerpos dinámicos y resuelve contactos
 * verticales contra cuerpos estáticos o cinemáticos.
 *
 * @param deltaTime Tiempo transcurrido desde el último frame.
 * @param actors Actores actualmente registrados en la escena.
 */
void
PhysicsSystem::update(
	float deltaTime,
	std::vector<EU::TSharedPointer<Actor>>& actors
)
{
	const float gravity = -9.81f;
	const float maximumFallSpeed = -20.0f;

	// Evitar valores de tiempo inválidos o exagerados.
	if (deltaTime <= 0.0f)
	{
		return;
	}

	if (deltaTime > 0.1f)
	{
		deltaTime = 0.1f;
	}

	/**
	 * Diagnóstico temporal.
	 * Se muestra una vez por ejecución del programa.
	 */
	static bool physicsSystemLogged = false;

	if (!physicsSystemLogged)
	{
		MESSAGE(
			"PhysicsSystem",
			"update",
			"PhysicsSystem ejecutandose"
		);

		/**
		 * Mostrar todos los actores recibidos por el sistema.
		 */
		for (auto& debugActor : actors)
		{
			if (debugActor.isNull())
			{
				MESSAGE(
					"PhysicsSystem",
					"update",
					"Actor nulo encontrado"
				);

				continue;
			}

			MESSAGE(
				"PhysicsSystem",
				"update",
				debugActor->getName().c_str()
			);
		}

		physicsSystemLogged = true;
	}

	/**
	 * Recorrer los actores que podrían ser cuerpos dinámicos.
	 */
	for (auto& actor : actors)
	{
		if (actor.isNull())
		{
			continue;
		}

		auto transform =
			actor->getComponent<Transform>();

		auto rigidbody =
			actor->getComponent<RigidbodyComponent>();

		auto collider =
			actor->getComponent<BoxColliderComponent>();

		/**
		 * Para ser simulado, el actor necesita los tres componentes.
		 */
		if (!transform || !rigidbody || !collider)
		{
			continue;
		}

		/**
		 * Ignorar componentes desactivados desde el Inspector.
		 */
		if (!rigidbody->isEnabled() ||
			!collider->isEnabled())
		{
			continue;
		}

		/**
		 * Los cuerpos cinemáticos funcionan como superficies estáticas.
		 */
		if (rigidbody->isKinematic)
		{
			continue;
		}

		const bool wasGrounded =
			rigidbody->isGrounded;

		rigidbody->isGrounded = false;

		/**
		 * Aplicar aceleración gravitatoria.
		 */
		if (rigidbody->useGravity)
		{
			rigidbody->velocity.y +=
				gravity * deltaTime;

			if (rigidbody->velocity.y < maximumFallSpeed)
			{
				rigidbody->velocity.y =
					maximumFallSpeed;
			}
		}

		/**
		 * Guardar la posición previa para detectar el cruce
		 * de una superficie entre dos frames.
		 */
		const EU::Vector3 previousPosition =
			transform->getPosition();

		EU::Vector3 newPosition =
			previousPosition;

		newPosition.x +=
			rigidbody->velocity.x * deltaTime;

		newPosition.y +=
			rigidbody->velocity.y * deltaTime;

		newPosition.z +=
			rigidbody->velocity.z * deltaTime;

		transform->setPosition(newPosition);

		/**
		 * Buscar colliders estáticos contra los cuales resolver
		 * la caída del cuerpo dinámico.
		 */
		for (auto& otherActor : actors)
		{
			if (otherActor.isNull())
			{
				continue;
			}

			/**
			 * Comparar las direcciones reales de los actores.
			 * Esto evita depender del operador == del TSharedPointer.
			 */
			if (otherActor.get() == actor.get())
			{
				continue;
			}

			auto otherTransform =
				otherActor->getComponent<Transform>();

			auto otherCollider =
				otherActor->getComponent<BoxColliderComponent>();

			if (!otherTransform || !otherCollider)
			{
				continue;
			}

			if (!otherCollider->isEnabled())
			{
				continue;
			}

			auto otherRigidbody =
				otherActor->getComponent<RigidbodyComponent>();

			/**
			 * El otro actor es una superficie estática cuando:
			 *
			 * 1. No tiene Rigidbody.
			 * 2. Tiene un Rigidbody cinemático.
			 */
			const bool otherIsStatic =
				!otherRigidbody ||
				otherRigidbody->isKinematic;

			if (!otherIsStatic)
			{
				continue;
			}

			/**
			 * Si tiene Rigidbody, pero está desactivado,
			 * no participa en la simulación.
			 */
			if (otherRigidbody &&
				!otherRigidbody->isEnabled())
			{
				continue;
			}

			/**
			 * Diagnóstico específico del Ground.
			 */
			if (otherActor->getName() == "Ground")
			{
				static bool groundLogged = false;

				if (!groundLogged)
				{
					MESSAGE(
						"PhysicsSystem",
						"update",
						"Ground encontrado y listo para colisionar"
					);

					groundLogged = true;
				}
			}

			/**
			 * Calcular los centros mundiales de ambos colliders.
			 */
			const EU::Vector3 actorCenter =
				transform->getPosition() +
				collider->center;

			const EU::Vector3 otherCenter =
				otherTransform->getPosition() +
				otherCollider->center;

			/**
			 * Límites horizontales del cuerpo dinámico.
			 */
			const float actorMinX =
				actorCenter.x -
				collider->size.x * 0.5f;

			const float actorMaxX =
				actorCenter.x +
				collider->size.x * 0.5f;

			const float actorMinZ =
				actorCenter.z -
				collider->size.z * 0.5f;

			const float actorMaxZ =
				actorCenter.z +
				collider->size.z * 0.5f;

			/**
			 * Límites horizontales del cuerpo estático.
			 */
			const float otherMinX =
				otherCenter.x -
				otherCollider->size.x * 0.5f;

			const float otherMaxX =
				otherCenter.x +
				otherCollider->size.x * 0.5f;

			const float otherMinZ =
				otherCenter.z -
				otherCollider->size.z * 0.5f;

			const float otherMaxZ =
				otherCenter.z +
				otherCollider->size.z * 0.5f;

			const bool overlapX =
				actorMinX <= otherMaxX &&
				actorMaxX >= otherMinX;

			const bool overlapZ =
				actorMinZ <= otherMaxZ &&
				actorMaxZ >= otherMinZ;

			/**
			 * Si no coinciden horizontalmente, el actor no se
			 * encuentra sobre esta superficie.
			 */
			if (!overlapX || !overlapZ)
			{
				continue;
			}

			const float otherTop =
				otherCenter.y +
				otherCollider->size.y * 0.5f;

			const float actorHalfHeight =
				collider->size.y * 0.5f;

			const float previousBottom =
				previousPosition.y +
				collider->center.y -
				actorHalfHeight;

			const float currentBottom =
				transform->getPosition().y +
				collider->center.y -
				actorHalfHeight;

			/**
			 * El actor cruzó la superficie entre dos frames.
			 */
			const bool crossedSurface =
				previousBottom >= otherTop &&
				currentBottom <= otherTop;

			/**
			 * El actor se encuentra actualmente superpuesto.
			 */
			const bool overlappingNow =
				checkAABB(
					actorCenter,
					collider->size,
					otherCenter,
					otherCollider->size
				);

			/**
			 * Protección adicional contra tunneling:
			 * si está cayendo y ya quedó debajo de la superficie,
			 * se resuelve igualmente el contacto.
			 */
			const bool passedSurfaceWhileFalling =
				rigidbody->velocity.y <= 0.0f &&
				currentBottom <= otherTop &&
				previousPosition.y >= otherCenter.y;

			if (!crossedSurface &&
				!overlappingNow &&
				!passedSurfaceWhileFalling)
			{
				continue;
			}

			/**
			 * Un Trigger registra la superposición, pero no impide
			 * el movimiento del cuerpo.
			 */
			if (collider->isTrigger ||
				otherCollider->isTrigger)
			{
				continue;
			}

			/**
			 * Colocar el cuerpo exactamente encima de la superficie.
			 */
			EU::Vector3 correctedPosition =
				transform->getPosition();

			correctedPosition.y =
				otherTop +
				actorHalfHeight -
				collider->center.y;

			transform->setPosition(
				correctedPosition
			);

			/**
			 * Cancelar únicamente la velocidad descendente.
			 */
			if (rigidbody->velocity.y < 0.0f)
			{
				rigidbody->velocity.y = 0.0f;
			}

			rigidbody->isGrounded = true;

			/**
			 * Registrar únicamente el primer frame de contacto.
			 */
			if (!wasGrounded)
			{
				MESSAGE(
					"PhysicsSystem",
					"update",
					"Colision con suelo detectada"
				);
			}

			break;
		}
	}
}