#pragma once

#include "ECS/Component.h"

#include <vector>

//============================================================
// COLOR DE PARTICULA
//============================================================

struct ParticleColor
{
	float x;
	float y;
	float z;
	float w;

	ParticleColor(
		float red = 1.0f,
		float green = 1.0f,
		float blue = 1.0f,
		float alpha = 1.0f)
		: x(red),
		y(green),
		z(blue),
		w(alpha)
	{
	}
};

//============================================================
// MODO DE MEZCLA
//============================================================

enum class ParticleBlendMode
{
	Alpha = 0,
	Additive
};

//============================================================
// FORMA DE EMISION
//============================================================

enum class ParticleEmissionShape
{
	Box = 0,
	Sphere,
	Cone
};

//============================================================
// CAPA DE PARTICULAS
//============================================================

/**
 * @struct ParticleLayer
 * @brief Configuración independiente de una capa de partículas.
 */
struct ParticleLayer
{
	ParticleLayer() = default;

	//========================================================
	// INFORMACION GENERAL
	//========================================================

	bool enabled = true;

	char name[64] =
		"Particle Layer";

	char texturePath[260] =
		"";

	//========================================================
	// EMISION
	//========================================================

	float emissionRate = 20.0f;

	unsigned int maxParticles = 500;

	float particleLifetime = 2.0f;

	bool randomizeLifetime = true;

	float lifetimeVariation = 0.15f;

	//========================================================
	// VELOCIDAD
	//========================================================

	float startSpeed = 2.0f;

	float speedVariation = 0.0f;

	float gravityMultiplier = 0.0f;

	//========================================================
	// TAMAÑO
	//========================================================

	float startSize = 0.20f;
	float endSize = 0.0f;

	float sizeVariation = 0.0f;

	//========================================================
	// ROTACION
	//
	// Todos los valores se muestran en grados en el Inspector.
	// ParticleSystem los convierte internamente a radianes.
	//========================================================

	float minimumStartRotation = 0.0f;
	float maximumStartRotation = 360.0f;

	float minimumAngularVelocity = 0.0f;
	float maximumAngularVelocity = 0.0f;

	//========================================================
	// COLOR
	//========================================================

	ParticleColor startColor{
		1.0f,
		0.65f,
		0.15f,
		1.0f
	};

	ParticleColor endColor{
		1.0f,
		0.10f,
		0.0f,
		0.0f
	};

	//========================================================
	// FORMA DEL EMISOR
	//========================================================

	ParticleEmissionShape emissionShape =
		ParticleEmissionShape::Box;

	/**
	 * @brief Tamaño utilizado por la forma Box.
	 *
	 * Para Cone, X y Z también determinan el tamaño de la
	 * base desde la cual nacen las partículas.
	 */
	EU::Vector3 emitterSize{
		0.25f,
		0.25f,
		0.25f
	};

	/**
	 * @brief Radio utilizado por la forma Sphere.
	 */
	float sphereRadius = 0.50f;

	/**
	 * @brief Ángulo de apertura utilizado por Cone.
	 */
	float coneAngleDegrees = 25.0f;

	/**
	 * @brief Radio de la base desde donde nacen las partículas.
	 */
	float coneBaseRadius = 0.10f;

	//========================================================
	// RENDER
	//========================================================

	ParticleBlendMode blendMode =
		ParticleBlendMode::Additive;

	/**
	 * @brief Ordena las partículas de atrás hacia delante.
	 *
	 * Es especialmente importante cuando Blend Mode es Alpha.
	 */
	bool depthSorting = true;
};

//============================================================
// COMPONENTE EMISOR
//============================================================

/**
 * @class ParticleEmitterComponent
 * @brief Contiene una o más capas independientes de partículas.
 */
class ParticleEmitterComponent : public Component
{
public:

	ParticleEmitterComponent()
		: Component(
			ComponentType::PARTICLE_EMITTER
		)
	{
		// Todo emisor comienza con una capa predeterminada.
		layers.emplace_back();
	}

	~ParticleEmitterComponent() override = default;

	//========================================================
	// COMPONENT
	//========================================================

	void init() override
	{
	}

	void update(float deltaTime) override
	{
		(void)deltaTime;
	}

	void render(
		DeviceContext& deviceContext
	) override
	{
		(void)deviceContext;
	}

	void destroy() override
	{
		layers.clear();
	}

	//========================================================
	// CONFIGURACION GENERAL
	//========================================================

	bool playOnStart = true;
	bool looping = true;

	/**
	 * @brief Duración de la emisión cuando Looping está apagado.
	 */
	float duration = 5.0f;

	/**
	 * @brief Capas independientes del emisor.
	 */
	std::vector<ParticleLayer> layers;

	//========================================================
	// CONFIGURACION LEGACY
	//
	// Estos valores se mantienen para conservar compatibilidad
	// con otras partes actuales del motor.
	//========================================================

	float emissionRate = 20.0f;

	unsigned int maxParticles = 500;

	float particleLifetime = 2.0f;
	float startSpeed = 2.0f;

	float startSize = 0.20f;
	float endSize = 0.0f;

	float gravityMultiplier = 0.0f;

	ParticleColor startColor{
		1.0f,
		0.65f,
		0.15f,
		1.0f
	};

	ParticleColor endColor{
		1.0f,
		0.10f,
		0.0f,
		0.0f
	};

	EU::Vector3 emitterSize{
		0.25f,
		0.25f,
		0.25f
	};
};