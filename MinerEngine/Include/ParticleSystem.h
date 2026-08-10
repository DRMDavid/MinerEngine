#pragma once

#include "Prerequisites.h"
#include "ECS/ParticleEmitterComponent.h"

#include <cstddef>
#include <random>
#include <unordered_map>
#include <vector>

//============================================================
// PARTICULA
//============================================================

/**
 * @struct Particle
 * @brief Representa el estado físico y visual de una partícula individual en la simulación.
 */
struct Particle
{
	EU::Vector3 position{
		0.0f,
		0.0f,
		0.0f
	}; /**< Posición actual de la partícula en el espacio mundo. */

	EU::Vector3 velocity{
		0.0f,
		0.0f,
		0.0f
	}; /**< Vector de velocidad actual. */

	ParticleColor color{
		1.0f,
		1.0f,
		1.0f,
		1.0f
	}; /**< Color actual de la partícula (RGBA). */

	float age = 0.0f;      /**< Tiempo acumulado que ha vivido la partícula (en segundos). */
	float lifetime = 1.0f; /**< Tiempo total de vida útil de la partícula (en segundos). */

	float size = 1.0f; /**< Escala o tamaño actual de la partícula. */

	/**
	 * @brief Rotación actual del billboard en radianes.
	 */
	float rotation = 0.0f;

	/**
	 * @brief Velocidad de rotación en radianes por segundo.
	 */
	float angularVelocity = 0.0f;

	/**
	 * @brief Flag que indica si la partícula está activa y debe ser procesada/renderizada.
	 */
	bool alive = false;
};

//============================================================
// PARTICLE SYSTEM
//============================================================

/**
 * @class ParticleSystem
 * @brief Sistema centralizado para la gestión y actualización de la simulación de partículas.
 * @details Se encarga de la lógica de actualización física, el ciclo de vida de las partículas (emisión, envejecimiento, muerte)
 * y la gestión de estados de ejecución para cada emisor registrado en la escena.
 */
class ParticleSystem
{
public:

	ParticleSystem();
	~ParticleSystem();

	ParticleSystem(
		const ParticleSystem&
	) = delete;

	ParticleSystem& operator=(
		const ParticleSystem&
		) = delete;

	//========================================================
	// ACTUALIZACION
	//========================================================

	/**
	 * @brief Actualiza la lógica de simulación de un emisor de partículas específico.
	 * @param emitter Componente del emisor a actualizar.
	 * @param emitterPosition Posición global del emisor en el mundo.
	 * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
	 */
	void updateEmitter(
		ParticleEmitterComponent& emitter,
		const EU::Vector3& emitterPosition,
		float deltaTime
	);

	/**
	 * @brief Detiene la emisión de partículas de un emisor, pero permite que las existentes terminen su vida.
	 * @param emitter Componente del emisor a detener.
	 */
	void stopEmitter(
		ParticleEmitterComponent& emitter
	);

	/**
	 * @brief Elimina todas las partículas y reinicia el estado de todos los emisores registrados.
	 */
	void clear();

	//========================================================
	// CONSULTAS
	//========================================================

	/**
	 * @brief Obtiene la cantidad total de partículas vivas en todas las capas de un emisor.
	 * @param emitter Emisor a consultar.
	 * @return Número de partículas vivas.
	 */
	std::size_t getAliveParticleCount(
		const ParticleEmitterComponent& emitter
	) const;

	/**
	 * @brief Obtiene la cantidad de partículas vivas en una capa específica de un emisor.
	 * @param emitter Emisor a consultar.
	 * @param layerIndex Índice de la capa.
	 * @return Número de partículas vivas en la capa.
	 */
	std::size_t getAliveParticleCount(
		const ParticleEmitterComponent& emitter,
		std::size_t layerIndex
	) const;

	/**
	 * @brief Compatibilidad temporal: devuelve la primera capa de partículas del emisor.
	 * @param emitter Emisor a consultar.
	 * @return Puntero al vector de partículas de la capa base.
	 */
	const std::vector<Particle>* getParticles(
		const ParticleEmitterComponent& emitter
	) const;

	/**
	 * @brief Devuelve las partículas de una capa concreta.
	 * @param emitter Emisor a consultar.
	 * @param layerIndex Índice de la capa.
	 * @return Puntero al vector de partículas de la capa solicitada.
	 */
	const std::vector<Particle>* getParticles(
		const ParticleEmitterComponent& emitter,
		std::size_t layerIndex
	) const;

private:

	//========================================================
	// ESTADO RUNTIME DE UNA CAPA
	//========================================================

	/**
	 * @struct LayerRuntime
	 * @brief Almacena el estado interno de una capa de partículas durante la simulación.
	 */
	struct LayerRuntime
	{
		std::vector<Particle> particles; /**< Lista de partículas pertenecientes a esta capa. */

		float emissionAccumulator = 0.0f; /**< Acumulador de tiempo para gestionar la tasa de emisión. */
	};

	//========================================================
	// ESTADO RUNTIME DE UN EMISOR
	//========================================================

	/**
	 * @struct EmitterRuntime
	 * @brief Almacena el estado interno de un emisor completo (todas sus capas).
	 */
	struct EmitterRuntime
	{
		std::vector<LayerRuntime> layers; /**< Conjunto de estados de ejecución de las capas del emisor. */

		float elapsedTime = 0.0f; /**< Tiempo total transcurrido desde que inició la emisión. */
	};

	//========================================================
	// ACTUALIZACION INTERNA
	//========================================================

	/**
	 * @brief Actualiza la lógica de una sola capa (emisión, movimiento, envejecimiento).
	 * @param runtime Estado de ejecución de la capa.
	 * @param layer Configuración de la capa.
	 * @param emitterPosition Posición del emisor.
	 * @param deltaTime Delta time.
	 * @param allowEmission Flag para habilitar/deshabilitar la creación de nuevas partículas.
	 */
	void updateLayer(
		LayerRuntime& runtime,
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		float deltaTime,
		bool allowEmission
	);

	/**
	 * @brief Instancia una nueva partícula en la capa especificada.
	 */
	void emitParticle(
		LayerRuntime& runtime,
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition
	);

	//========================================================
	// FORMAS DE EMISION
	//========================================================

	/** @brief Calcula posición y dirección para emisión en forma de caja. */
	void generateBoxEmission(
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		EU::Vector3& outPosition,
		EU::Vector3& outDirection
	);

	/** @brief Calcula posición y dirección para emisión en forma de esfera. */
	void generateSphereEmission(
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		EU::Vector3& outPosition,
		EU::Vector3& outDirection
	);

	/** @brief Calcula posición y dirección para emisión en forma de cono. */
	void generateConeEmission(
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		EU::Vector3& outPosition,
		EU::Vector3& outDirection
	);

	//========================================================
	// UTILIDADES
	//========================================================

	/** @brief Obtiene un número aleatorio en un rango [min, max]. */
	float randomRange(
		float minimumValue,
		float maximumValue
	);

	/** @brief Normaliza un vector 3D. */
	static EU::Vector3 normalizeVector(
		const EU::Vector3& vector
	);

	/** @brief Convierte grados a radianes. */
	static float degreesToRadians(
		float degrees
	);

	/** @brief Realiza interpolación lineal (lerp). */
	static float lerp(
		float start,
		float end,
		float amount
	);

	/** @brief Realiza interpolación lineal entre colores. */
	static ParticleColor lerpColor(
		const ParticleColor& start,
		const ParticleColor& end,
		float amount
	);

	//========================================================
	// DATOS
	//========================================================

	std::unordered_map<
		const ParticleEmitterComponent*,
		EmitterRuntime
	> m_emitters; /**< Caché de estados de ejecución para cada componente emisor. */

	std::mt19937 m_randomEngine; /**< Motor de generación de números aleatorios. */
};