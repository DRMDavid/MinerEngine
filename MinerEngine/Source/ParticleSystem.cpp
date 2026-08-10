#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>

//============================================================
// CONSTRUCTOR / DESTRUCTOR
//============================================================

ParticleSystem::ParticleSystem()
	: m_randomEngine(std::random_device{}())
{
}

ParticleSystem::~ParticleSystem()
{
	clear();
}

//============================================================
// ACTUALIZAR EMISOR
//============================================================

void
ParticleSystem::updateEmitter(
	ParticleEmitterComponent& emitter,
	const EU::Vector3& emitterPosition,
	float deltaTime)
{
	if (!emitter.isEnabled() || deltaTime <= 0.0f)
	{
		return;
	}

	EmitterRuntime& runtime =
		m_emitters[&emitter];

	// Mantener un runtime independiente para cada capa.
	runtime.layers.resize(
		emitter.layers.size()
	);

	runtime.elapsedTime += deltaTime;

	const float validDuration =
		(std::max)(
			0.01f,
			emitter.duration
			);

	const bool allowEmission =
		emitter.looping ||
		runtime.elapsedTime < validDuration;

	for (std::size_t layerIndex = 0;
		layerIndex < emitter.layers.size();
		++layerIndex)
	{
		const ParticleLayer& layer =
			emitter.layers[layerIndex];

		LayerRuntime& layerRuntime =
			runtime.layers[layerIndex];

		if (!layer.enabled)
		{
			layerRuntime.particles.clear();
			layerRuntime.emissionAccumulator = 0.0f;
			continue;
		}

		updateLayer(
			layerRuntime,
			layer,
			emitterPosition,
			deltaTime,
			allowEmission
		);
	}
}

//============================================================
// ACTUALIZAR CAPA
//============================================================

void
ParticleSystem::updateLayer(
	LayerRuntime& runtime,
	const ParticleLayer& layer,
	const EU::Vector3& emitterPosition,
	float deltaTime,
	bool allowEmission)
{
	const unsigned int maximumParticles =
		layer.maxParticles;

	if (maximumParticles == 0)
	{
		runtime.particles.clear();
		runtime.emissionAccumulator = 0.0f;
		return;
	}

	// Si el usuario reduce el máximo desde el Inspector,
	// eliminamos las partículas sobrantes.
	if (runtime.particles.size() >
		static_cast<std::size_t>(maximumParticles))
	{
		runtime.particles.resize(
			static_cast<std::size_t>(
				maximumParticles
				)
		);
	}

	// Actualizar partículas vivas.
	for (Particle& particle : runtime.particles)
	{
		if (!particle.alive)
		{
			continue;
		}

		particle.age += deltaTime;

		if (particle.age >= particle.lifetime)
		{
			particle.alive = false;
			continue;
		}

		particle.velocity.y +=
			layer.gravityMultiplier *
			deltaTime;

		particle.position.x +=
			particle.velocity.x *
			deltaTime;

		particle.position.y +=
			particle.velocity.y *
			deltaTime;

		particle.position.z +=
			particle.velocity.z *
			deltaTime;

		const float normalizedAge =
			std::clamp(
				particle.age /
				(std::max)(
					0.001f,
					particle.lifetime
					),
				0.0f,
				1.0f
			);

		particle.size =
			lerp(
				layer.startSize,
				layer.endSize,
				normalizedAge
			);

		particle.color =
			lerpColor(
				layer.startColor,
				layer.endColor,
				normalizedAge
			);
	}

	if (!allowEmission ||
		layer.emissionRate <= 0.0f)
	{
		return;
	}

	runtime.emissionAccumulator +=
		layer.emissionRate *
		deltaTime;

	int particlesToEmit =
		static_cast<int>(
			runtime.emissionAccumulator
			);

	runtime.emissionAccumulator -=
		static_cast<float>(
			particlesToEmit
			);

	unsigned int aliveCount = 0;

	for (const Particle& particle :
		runtime.particles)
	{
		if (particle.alive)
		{
			++aliveCount;
		}
	}

	const unsigned int availableSlots =
		maximumParticles > aliveCount
		? maximumParticles - aliveCount
		: 0;

	particlesToEmit =
		(std::min)(
			particlesToEmit,
			static_cast<int>(
				availableSlots
				)
			);

	// Crear las partículas calculadas para esta capa.
	for (int particleIndex = 0;
		particleIndex < particlesToEmit;
		++particleIndex)
	{
		emitParticle(
			runtime,
			layer,
			emitterPosition
		);
	}
}
	


//============================================================
// EMITIR PARTÍCULA
//============================================================

void
ParticleSystem::emitParticle(
	LayerRuntime& runtime,
	const ParticleLayer& layer,
	const EU::Vector3& emitterPosition)
{
	Particle* particleToUse = nullptr;

	// Reutilizar primero una partícula muerta.
	for (Particle& particle : runtime.particles)
	{
		if (!particle.alive)
		{
			particleToUse = &particle;
			break;
		}
	}

	// Si no hay partículas libres, crear una nueva.
	if (!particleToUse)
	{
		const unsigned int maximumParticles =
			layer.maxParticles;

		if (runtime.particles.size() >=
			static_cast<std::size_t>(
				maximumParticles
				))
		{
			return;
		}

		runtime.particles.emplace_back();

		particleToUse =
			&runtime.particles.back();
	}

	std::uniform_real_distribution<float>
		randomHorizontal(
			-1.0f,
			1.0f
		);

	std::uniform_real_distribution<float>
		randomVertical(
			0.25f,
			1.0f
		);

	std::uniform_real_distribution<float>
		randomLifetime(
			0.85f,
			1.15f
		);

	const float halfEmitterX =
		(std::max)(
			0.0f,
			layer.emitterSize.x * 0.5f
			);

	const float halfEmitterY =
		(std::max)(
			0.0f,
			layer.emitterSize.y * 0.5f
			);

	const float halfEmitterZ =
		(std::max)(
			0.0f,
			layer.emitterSize.z * 0.5f
			);

	std::uniform_real_distribution<float>
		randomPositionX(
			-halfEmitterX,
			halfEmitterX
		);

	std::uniform_real_distribution<float>
		randomPositionY(
			-halfEmitterY,
			halfEmitterY
		);

	std::uniform_real_distribution<float>
		randomPositionZ(
			-halfEmitterZ,
			halfEmitterZ
		);

	Particle& particle =
		*particleToUse;

	particle.position =
		EU::Vector3(
			emitterPosition.x +
			randomPositionX(
				m_randomEngine
			),
			emitterPosition.y +
			randomPositionY(
				m_randomEngine
			),
			emitterPosition.z +
			randomPositionZ(
				m_randomEngine
			)
		);

	float directionX =
		randomHorizontal(
			m_randomEngine
		);

	float directionY =
		randomVertical(
			m_randomEngine
		);

	float directionZ =
		randomHorizontal(
			m_randomEngine
		);

	const float directionLength =
		std::sqrt(
			directionX * directionX +
			directionY * directionY +
			directionZ * directionZ
		);

	if (directionLength > 0.0001f)
	{
		directionX /= directionLength;
		directionY /= directionLength;
		directionZ /= directionLength;
	}

	const float startSpeed =
		(std::max)(
			0.0f,
			layer.startSpeed
			);

	particle.velocity =
		EU::Vector3(
			directionX * startSpeed,
			directionY * startSpeed,
			directionZ * startSpeed
		);

	particle.age = 0.0f;

	particle.lifetime =
		(std::max)(
			0.01f,
			layer.particleLifetime *
			randomLifetime(
				m_randomEngine
			)
			);

	particle.size =
		(std::max)(
			0.0f,
			layer.startSize
			);

	particle.color =
		layer.startColor;

	particle.alive = true;
}

//============================================================
// DETENER EMISOR
//============================================================

void
ParticleSystem::stopEmitter(
	ParticleEmitterComponent& emitter)
{
	auto iterator =
		m_emitters.find(
			&emitter
		);

	if (iterator == m_emitters.end())
	{
		return;
	}

	m_emitters.erase(
		iterator
	);
}

//============================================================
// LIMPIAR TODO
//============================================================

void
ParticleSystem::clear()
{
	m_emitters.clear();
}

//============================================================
// CONTAR TODAS LAS PARTÍCULAS
//============================================================

std::size_t
ParticleSystem::getAliveParticleCount(
	const ParticleEmitterComponent& emitter) const
{
	const auto iterator =
		m_emitters.find(
			&emitter
		);

	if (iterator == m_emitters.end())
	{
		return 0;
	}

	std::size_t aliveCount = 0;

	for (const LayerRuntime& layerRuntime :
		iterator->second.layers)
	{
		for (const Particle& particle :
			layerRuntime.particles)
		{
			if (particle.alive)
			{
				++aliveCount;
			}
		}
	}

	return aliveCount;
}

//============================================================
// CONTAR PARTÍCULAS DE UNA CAPA
//============================================================

std::size_t
ParticleSystem::getAliveParticleCount(
	const ParticleEmitterComponent& emitter,
	std::size_t layerIndex) const
{
	const std::vector<Particle>* particles =
		getParticles(
			emitter,
			layerIndex
		);

	if (!particles)
	{
		return 0;
	}

	std::size_t aliveCount = 0;

	for (const Particle& particle :
		*particles)
	{
		if (particle.alive)
		{
			++aliveCount;
		}
	}

	return aliveCount;
}

//============================================================
// OBTENER PRIMERA CAPA — COMPATIBILIDAD
//============================================================

const std::vector<Particle>*
ParticleSystem::getParticles(
	const ParticleEmitterComponent& emitter) const
{
	return getParticles(
		emitter,
		0
	);
}

//============================================================
// OBTENER UNA CAPA ESPECÍFICA
//============================================================

const std::vector<Particle>*
ParticleSystem::getParticles(
	const ParticleEmitterComponent& emitter,
	std::size_t layerIndex) const
{
	const auto iterator =
		m_emitters.find(
			&emitter
		);

	if (iterator == m_emitters.end())
	{
		return nullptr;
	}

	if (layerIndex >=
		iterator->second.layers.size())
	{
		return nullptr;
	}

	return
		&iterator
		->second
		.layers[layerIndex]
		.particles;
}

//============================================================
// INTERPOLACIÓN
//============================================================

float
ParticleSystem::lerp(
	float start,
	float end,
	float amount)
{
	return
		start +
		(end - start) *
		amount;
}

ParticleColor
ParticleSystem::lerpColor(
	const ParticleColor& start,
	const ParticleColor& end,
	float amount)
{
	return
		ParticleColor(
			lerp(
				start.x,
				end.x,
				amount
			),
			lerp(
				start.y,
				end.y,
				amount
			),
			lerp(
				start.z,
				end.z,
				amount
			),
			lerp(
				start.w,
				end.w,
				amount
			)
		);
}