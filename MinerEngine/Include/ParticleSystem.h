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

struct Particle
{
	EU::Vector3 position{
		0.0f,
		0.0f,
		0.0f
	};

	EU::Vector3 velocity{
		0.0f,
		0.0f,
		0.0f
	};

	ParticleColor color{
		1.0f,
		1.0f,
		1.0f,
		1.0f
	};

	float age = 0.0f;
	float lifetime = 1.0f;

	float size = 1.0f;

	/**
	 * @brief Rotación actual del billboard en radianes.
	 */
	float rotation = 0.0f;

	/**
	 * @brief Velocidad de rotación en radianes por segundo.
	 */
	float angularVelocity = 0.0f;

	bool alive = false;
};

//============================================================
// PARTICLE SYSTEM
//============================================================

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

	void updateEmitter(
		ParticleEmitterComponent& emitter,
		const EU::Vector3& emitterPosition,
		float deltaTime
	);

	void stopEmitter(
		ParticleEmitterComponent& emitter
	);

	void clear();

	//========================================================
	// CONSULTAS
	//========================================================

	std::size_t getAliveParticleCount(
		const ParticleEmitterComponent& emitter
	) const;

	std::size_t getAliveParticleCount(
		const ParticleEmitterComponent& emitter,
		std::size_t layerIndex
	) const;

	/**
	 * @brief Compatibilidad temporal: devuelve la primera capa.
	 */
	const std::vector<Particle>* getParticles(
		const ParticleEmitterComponent& emitter
	) const;

	/**
	 * @brief Devuelve las partículas de una capa concreta.
	 */
	const std::vector<Particle>* getParticles(
		const ParticleEmitterComponent& emitter,
		std::size_t layerIndex
	) const;

private:

	//========================================================
	// ESTADO RUNTIME DE UNA CAPA
	//========================================================

	struct LayerRuntime
	{
		std::vector<Particle> particles;

		float emissionAccumulator = 0.0f;
	};

	//========================================================
	// ESTADO RUNTIME DE UN EMISOR
	//========================================================

	struct EmitterRuntime
	{
		std::vector<LayerRuntime> layers;

		float elapsedTime = 0.0f;
	};

	//========================================================
	// ACTUALIZACION INTERNA
	//========================================================

	void updateLayer(
		LayerRuntime& runtime,
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		float deltaTime,
		bool allowEmission
	);

	void emitParticle(
		LayerRuntime& runtime,
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition
	);

	//========================================================
	// FORMAS DE EMISION
	//========================================================

	void generateBoxEmission(
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		EU::Vector3& outPosition,
		EU::Vector3& outDirection
	);

	void generateSphereEmission(
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		EU::Vector3& outPosition,
		EU::Vector3& outDirection
	);

	void generateConeEmission(
		const ParticleLayer& layer,
		const EU::Vector3& emitterPosition,
		EU::Vector3& outPosition,
		EU::Vector3& outDirection
	);

	//========================================================
	// UTILIDADES
	//========================================================

	float randomRange(
		float minimumValue,
		float maximumValue
	);

	static EU::Vector3 normalizeVector(
		const EU::Vector3& vector
	);

	static float degreesToRadians(
		float degrees
	);

	static float lerp(
		float start,
		float end,
		float amount
	);

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
	> m_emitters;

	std::mt19937 m_randomEngine;
};