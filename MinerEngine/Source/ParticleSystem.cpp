#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>

//============================================================
// CONSTANTES
//============================================================

namespace
{
	constexpr float PARTICLE_PI =
		3.14159265358979323846f;

	constexpr float PARTICLE_TWO_PI =
		PARTICLE_PI * 2.0f;
}

//============================================================
// CONSTRUCTOR / DESTRUCTOR
//============================================================

ParticleSystem::ParticleSystem()
	: m_randomEngine(
		std::random_device{}()
	)
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
	if (!emitter.isEnabled() ||
		deltaTime <= 0.0f)
	{
		return;
	}

	EmitterRuntime& runtime =
		m_emitters[&emitter];

	// Cada capa necesita su propio estado de simulación.
	runtime.layers.resize(
		emitter.layers.size()
	);

	runtime.elapsedTime +=
		deltaTime;

	const float validDuration =
		(std::max)(
			0.01f,
			emitter.duration
			);

	const bool allowEmission =
		emitter.looping ||
		runtime.elapsedTime <
		validDuration;

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

			layerRuntime.emissionAccumulator =
				0.0f;

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

		runtime.emissionAccumulator =
			0.0f;

		return;
	}

	// Si el máximo se redujo desde el Inspector,
	// eliminar las partículas sobrantes.
	if (runtime.particles.size() >
		static_cast<std::size_t>(
			maximumParticles
			))
	{
		runtime.particles.resize(
			static_cast<std::size_t>(
				maximumParticles
				)
		);
	}

	//========================================================
	// ACTUALIZAR PARTICULAS VIVAS
	//========================================================

	for (Particle& particle :
		runtime.particles)
	{
		if (!particle.alive)
		{
			continue;
		}

		particle.age +=
			deltaTime;

		if (particle.age >=
			particle.lifetime)
		{
			particle.alive = false;
			continue;
		}

		// Gravedad.
		particle.velocity.y +=
			layer.gravityMultiplier *
			deltaTime;

		// Movimiento.
		particle.position.x +=
			particle.velocity.x *
			deltaTime;

		particle.position.y +=
			particle.velocity.y *
			deltaTime;

		particle.position.z +=
			particle.velocity.z *
			deltaTime;

		// Rotación.
		particle.rotation +=
			particle.angularVelocity *
			deltaTime;

		// Mantener la rotación dentro de un intervalo estable.
		if (particle.rotation >
			PARTICLE_TWO_PI)
		{
			particle.rotation =
				std::fmod(
					particle.rotation,
					PARTICLE_TWO_PI
				);
		}
		else if (particle.rotation <
			-PARTICLE_TWO_PI)
		{
			particle.rotation =
				std::fmod(
					particle.rotation,
					PARTICLE_TWO_PI
				);
		}

		const float safeLifetime =
			(std::max)(
				0.001f,
				particle.lifetime
				);

		const float normalizedAge =
			std::clamp(
				particle.age /
				safeLifetime,
				0.0f,
				1.0f
			);

		// Interpolar tamaño.
		particle.size =
			lerp(
				layer.startSize,
				layer.endSize,
				normalizedAge
			);

		// Interpolar color.
		particle.color =
			lerpColor(
				layer.startColor,
				layer.endColor,
				normalizedAge
			);
	}

	//========================================================
	// EMISION
	//========================================================

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
// EMITIR PARTICULA
//============================================================

void
ParticleSystem::emitParticle(
	LayerRuntime& runtime,
	const ParticleLayer& layer,
	const EU::Vector3& emitterPosition)
{
	Particle* particleToUse =
		nullptr;

	// Reutilizar primero una partícula muerta.
	for (Particle& particle :
		runtime.particles)
	{
		if (!particle.alive)
		{
			particleToUse =
				&particle;

			break;
		}
	}

	// Crear una nueva si todavía hay espacio.
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

	Particle& particle =
		*particleToUse;

	EU::Vector3 generatedPosition{
		emitterPosition.x,
		emitterPosition.y,
		emitterPosition.z
	};

	EU::Vector3 generatedDirection{
		0.0f,
		1.0f,
		0.0f
	};

	//========================================================
	// FORMA DE EMISION
	//========================================================

	switch (layer.emissionShape)
	{
	case ParticleEmissionShape::Sphere:

		generateSphereEmission(
			layer,
			emitterPosition,
			generatedPosition,
			generatedDirection
		);

		break;

	case ParticleEmissionShape::Cone:

		generateConeEmission(
			layer,
			emitterPosition,
			generatedPosition,
			generatedDirection
		);

		break;

	case ParticleEmissionShape::Box:
	default:

		generateBoxEmission(
			layer,
			emitterPosition,
			generatedPosition,
			generatedDirection
		);

		break;
	}

	particle.position =
		generatedPosition;

	//========================================================
	// VELOCIDAD
	//========================================================

	const float baseSpeed =
		(std::max)(
			0.0f,
			layer.startSpeed
			);

	const float validSpeedVariation =
		(std::max)(
			0.0f,
			layer.speedVariation
			);

	const float minimumSpeed =
		(std::max)(
			0.0f,
			baseSpeed -
			validSpeedVariation
			);

	const float maximumSpeed =
		baseSpeed +
		validSpeedVariation;

	const float selectedSpeed =
		randomRange(
			minimumSpeed,
			maximumSpeed
		);

	particle.velocity =
		EU::Vector3(
			generatedDirection.x *
			selectedSpeed,
			generatedDirection.y *
			selectedSpeed,
			generatedDirection.z *
			selectedSpeed
		);

	//========================================================
	// TIEMPO DE VIDA
	//========================================================

	particle.age = 0.0f;

	float lifetimeMultiplier =
		1.0f;

	if (layer.randomizeLifetime)
	{
		const float validVariation =
			std::clamp(
				layer.lifetimeVariation,
				0.0f,
				0.95f
			);

		lifetimeMultiplier =
			randomRange(
				1.0f -
				validVariation,
				1.0f +
				validVariation
			);
	}

	particle.lifetime =
		(std::max)(
			0.01f,
			layer.particleLifetime *
			lifetimeMultiplier
			);

	//========================================================
	// TAMAÑO
	//========================================================

	const float validSizeVariation =
		(std::max)(
			0.0f,
			layer.sizeVariation
			);

	const float minimumSize =
		(std::max)(
			0.0f,
			layer.startSize -
			validSizeVariation
			);

	const float maximumSize =
		(std::max)(
			minimumSize,
			layer.startSize +
			validSizeVariation
			);

	particle.size =
		randomRange(
			minimumSize,
			maximumSize
		);

	//========================================================
	// ROTACION
	//========================================================

	float minimumRotation =
		layer.minimumStartRotation;

	float maximumRotation =
		layer.maximumStartRotation;

	if (minimumRotation >
		maximumRotation)
	{
		std::swap(
			minimumRotation,
			maximumRotation
		);
	}

	particle.rotation =
		degreesToRadians(
			randomRange(
				minimumRotation,
				maximumRotation
			)
		);

	float minimumAngularVelocity =
		layer.minimumAngularVelocity;

	float maximumAngularVelocity =
		layer.maximumAngularVelocity;

	if (minimumAngularVelocity >
		maximumAngularVelocity)
	{
		std::swap(
			minimumAngularVelocity,
			maximumAngularVelocity
		);
	}

	particle.angularVelocity =
		degreesToRadians(
			randomRange(
				minimumAngularVelocity,
				maximumAngularVelocity
			)
		);

	//========================================================
	// COLOR Y ESTADO
	//========================================================

	particle.color =
		layer.startColor;

	particle.alive = true;
}

//============================================================
// EMISION BOX
//============================================================

void
ParticleSystem::generateBoxEmission(
	const ParticleLayer& layer,
	const EU::Vector3& emitterPosition,
	EU::Vector3& outPosition,
	EU::Vector3& outDirection)
{
	const float halfEmitterX =
		(std::max)(
			0.0f,
			layer.emitterSize.x *
			0.5f
			);

	const float halfEmitterY =
		(std::max)(
			0.0f,
			layer.emitterSize.y *
			0.5f
			);

	const float halfEmitterZ =
		(std::max)(
			0.0f,
			layer.emitterSize.z *
			0.5f
			);

	outPosition =
		EU::Vector3(
			emitterPosition.x +
			randomRange(
				-halfEmitterX,
				halfEmitterX
			),
			emitterPosition.y +
			randomRange(
				-halfEmitterY,
				halfEmitterY
			),
			emitterPosition.z +
			randomRange(
				-halfEmitterZ,
				halfEmitterZ
			)
		);

	// Dirección aleatoria con tendencia hacia arriba.
	outDirection =
		normalizeVector(
			EU::Vector3(
				randomRange(
					-1.0f,
					1.0f
				),
				randomRange(
					0.25f,
					1.0f
				),
				randomRange(
					-1.0f,
					1.0f
				)
			)
		);
}

//============================================================
// EMISION SPHERE
//============================================================

void
ParticleSystem::generateSphereEmission(
	const ParticleLayer& layer,
	const EU::Vector3& emitterPosition,
	EU::Vector3& outPosition,
	EU::Vector3& outDirection)
{
	const float radius =
		(std::max)(
			0.0f,
			layer.sphereRadius
			);

	const float azimuth =
		randomRange(
			0.0f,
			PARTICLE_TWO_PI
		);

	const float verticalValue =
		randomRange(
			-1.0f,
			1.0f
		);

	const float horizontalLength =
		std::sqrt(
			(std::max)(
				0.0f,
				1.0f -
				verticalValue *
				verticalValue
				)
		);

	outDirection =
		normalizeVector(
			EU::Vector3(
				horizontalLength *
				std::cos(
					azimuth
				),
				verticalValue,
				horizontalLength *
				std::sin(
					azimuth
				)
			)
		);

	// Raíz cúbica para distribuir partículas
	// uniformemente dentro del volumen.
	const float randomRadius =
		radius *
		std::cbrt(
			randomRange(
				0.0f,
				1.0f
			)
		);

	outPosition =
		EU::Vector3(
			emitterPosition.x +
			outDirection.x *
			randomRadius,
			emitterPosition.y +
			outDirection.y *
			randomRadius,
			emitterPosition.z +
			outDirection.z *
			randomRadius
		);
}

//============================================================
// EMISION CONE
//============================================================

void
ParticleSystem::generateConeEmission(
	const ParticleLayer& layer,
	const EU::Vector3& emitterPosition,
	EU::Vector3& outPosition,
	EU::Vector3& outDirection)
{
	const float baseRadius =
		(std::max)(
			0.0f,
			layer.coneBaseRadius
			);

	// Distribución uniforme dentro del disco de la base.
	const float baseAngle =
		randomRange(
			0.0f,
			PARTICLE_TWO_PI
		);

	const float selectedBaseRadius =
		baseRadius *
		std::sqrt(
			randomRange(
				0.0f,
				1.0f
			)
		);

	outPosition =
		EU::Vector3(
			emitterPosition.x +
			std::cos(
				baseAngle
			) *
			selectedBaseRadius,
			emitterPosition.y,
			emitterPosition.z +
			std::sin(
				baseAngle
			) *
			selectedBaseRadius
		);

	const float clampedConeAngle =
		std::clamp(
			layer.coneAngleDegrees,
			0.0f,
			89.0f
		);

	const float coneAngleRadians =
		degreesToRadians(
			clampedConeAngle
		);

	const float minimumCosine =
		std::cos(
			coneAngleRadians
		);

	// Distribuir uniformemente dentro del cono.
	const float selectedCosine =
		randomRange(
			minimumCosine,
			1.0f
		);

	const float selectedSine =
		std::sqrt(
			(std::max)(
				0.0f,
				1.0f -
				selectedCosine *
				selectedCosine
				)
		);

	const float directionAngle =
		randomRange(
			0.0f,
			PARTICLE_TWO_PI
		);

	// El cono apunta hacia +Y.
	outDirection =
		normalizeVector(
			EU::Vector3(
				std::cos(
					directionAngle
				) *
				selectedSine,
				selectedCosine,
				std::sin(
					directionAngle
				) *
				selectedSine
			)
		);
}

//============================================================
// DETENER EMISOR
//============================================================

void
ParticleSystem::stopEmitter(
	ParticleEmitterComponent& emitter)
{
	const auto iterator =
		m_emitters.find(
			&emitter
		);

	if (iterator ==
		m_emitters.end())
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
// CONTAR TODAS LAS PARTICULAS
//============================================================

std::size_t
ParticleSystem::getAliveParticleCount(
	const ParticleEmitterComponent& emitter) const
{
	const auto iterator =
		m_emitters.find(
			&emitter
		);

	if (iterator ==
		m_emitters.end())
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
// CONTAR PARTICULAS DE UNA CAPA
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
// OBTENER PRIMERA CAPA
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
// OBTENER UNA CAPA
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

	if (iterator ==
		m_emitters.end())
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
// NUMERO ALEATORIO
//============================================================

float
ParticleSystem::randomRange(
	float minimumValue,
	float maximumValue)
{
	if (minimumValue >
		maximumValue)
	{
		std::swap(
			minimumValue,
			maximumValue
		);
	}

	if (minimumValue ==
		maximumValue)
	{
		return minimumValue;
	}

	std::uniform_real_distribution<float>
		distribution(
			minimumValue,
			maximumValue
		);

	return distribution(
		m_randomEngine
	);
}

//============================================================
// NORMALIZAR VECTOR
//============================================================

EU::Vector3
ParticleSystem::normalizeVector(
	const EU::Vector3& vector)
{
	const float length =
		std::sqrt(
			vector.x * vector.x +
			vector.y * vector.y +
			vector.z * vector.z
		);

	if (length <= 0.0001f)
	{
		return
			EU::Vector3(
				0.0f,
				1.0f,
				0.0f
			);
	}

	return
		EU::Vector3(
			vector.x / length,
			vector.y / length,
			vector.z / length
		);
}

//============================================================
// GRADOS A RADIANES
//============================================================

float
ParticleSystem::degreesToRadians(
	float degrees)
{
	return
		degrees *
		(PARTICLE_PI / 180.0f);
}

//============================================================
// INTERPOLACION
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