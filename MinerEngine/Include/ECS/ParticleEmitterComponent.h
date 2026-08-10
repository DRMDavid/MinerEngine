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
// CAPA DE PARTICULAS
//============================================================

/**
 * @struct ParticleLayer
 * @brief Configuración independiente de un tipo de partícula.
 */
struct ParticleLayer
{
	ParticleLayer() = default;

	bool enabled = true;

	char name[64] =
		"Particle Layer";

	char texturePath[260] =
		"";

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

	ParticleBlendMode blendMode =
		ParticleBlendMode::Additive;
};

//============================================================
// COMPONENTE EMISOR
//============================================================

/**
 * @class ParticleEmitterComponent
 * @brief Contiene una o más capas de partículas.
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
	 * @brief Duración de la emisión cuando Looping está desactivado.
	 */
	float duration = 5.0f;

	/**
	 * @brief Capas independientes del emisor.
	 */
	std::vector<ParticleLayer> layers;

	//========================================================
	// CONFIGURACION LEGACY TEMPORAL
	//
	// Se conserva durante la migración para que ParticleSystem,
	// ParticleRenderer y GUI continúen compilando.
	// Se eliminará cuando todos utilicen layers.
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