#pragma once

#include "ECS/Component.h"

#include <vector>

//============================================================
// COLOR DE PARTICULA
//============================================================

/**
 * @struct ParticleColor
 * @brief Representa un color RGBA mediante valores de punto flotante.
 */
struct ParticleColor
{
	float x; /**< Componente de color Rojo (R). Range: [0.0, 1.0]. */
	float y; /**< Componente de color Verde (G). Range: [0.0, 1.0]. */
	float z; /**< Componente de color Azul (B). Range: [0.0, 1.0]. */
	float w; /**< Componente de canal Alfa/Opacidad (A). Range: [0.0, 1.0]. */

	/**
	 * @brief Constructor parametrizado para definir el color RGBA.
	 * @param red Canal de color rojo.
	 * @param green Canal de color verde.
	 * @param blue Canal de color azul.
	 * @param alpha Canal de opacidad.
	 */
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

/**
 * @enum ParticleBlendMode
 * @brief Define los modos de mezcla (blending) para el renderizado de partículas.
 */
enum class ParticleBlendMode
{
	Alpha = 0, /**< Mezcla Alfa transparente estándar. */
	Additive   /**< Mezcla aditiva para efectos brillantes o luminosos (fuego, luz, etc.). */
};

//============================================================
// FORMA DE EMISION
//============================================================

/**
 * @enum ParticleEmissionShape
 * @brief Geometrías de origen desde las cuales nacen las partículas.
 */
enum class ParticleEmissionShape
{
	Box = 0, /**< Emisión dentro de una caja 3D. */
	Sphere,  /**< Emisión dentro o sobre una esfera. */
	Cone     /**< Emisión en forma de cono tridimensional. */
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
	/** @brief Constructor por defecto. */
	ParticleLayer() = default;

	//========================================================
	// INFORMACION GENERAL
	//========================================================

	bool enabled = true; /**< Determina si esta capa de partículas está activa. */

	char name[64] =
		"Particle Layer"; /**< Nombre identificativo de la capa. */

	char texturePath[260] =
		""; /**< Ruta relativa o absoluta del recurso de textura aplicado a las partículas. */

	//========================================================
	// EMISION
	//========================================================

	float emissionRate = 20.0f; /**< Tasa de generación de partículas (partículas creadas por segundo). */

	unsigned int maxParticles = 500; /**< Cantidad máxima de partículas vivas simultáneas permitidas en esta capa. */

	float particleLifetime = 2.0f; /**< Tiempo de vida base de cada partícula (en segundos). */

	bool randomizeLifetime = true; /**< Indica si se aplica variación aleatoria al tiempo de vida. */

	float lifetimeVariation = 0.15f; /**< Rango de variación aleatoria del tiempo de vida. */

	//========================================================
	// VELOCIDAD
	//========================================================

	float startSpeed = 2.0f; /**< Velocidad inicial con la que nacen las partículas. */

	float speedVariation = 0.0f; /**< Factor de variación aleatoria sobre la velocidad inicial. */

	float gravityMultiplier = 0.0f; /**< Multiplicador de fuerza de gravedad aplicado al movimiento de las partículas. */

	//========================================================
	// TAMAÑO
	//========================================================

	float startSize = 0.20f; /**< Escala/tamaño inicial de la partícula al nacer. */
	float endSize = 0.0f;    /**< Escala/tamaño final de la partícula al morir. */

	float sizeVariation = 0.0f; /**< Factor de variación aleatoria aplicado al tamaño inicial. */

	//========================================================
	// ROTACION
	//
	// Todos los valores se muestran en grados en el Inspector.
	// ParticleSystem los convierte internamente a radianes.
	//========================================================

	float minimumStartRotation = 0.0f;   /**< Ángulo de rotación inicial mínimo (en grados). */
	float maximumStartRotation = 360.0f; /**< Ángulo de rotación inicial máximo (en grados). */

	float minimumAngularVelocity = 0.0f; /**< Velocidad angular mínima de rotación continua (en grados/s). */
	float maximumAngularVelocity = 0.0f; /**< Velocidad angular máxima de rotación continua (en grados/s). */

	//========================================================
	// COLOR
	//========================================================

	/**
	 * @brief Color inicial de la partícula al nacer.
	 */
	ParticleColor startColor{
		1.0f,
		0.65f,
		0.15f,
		1.0f
	};

	/**
	 * @brief Color final que alcanza la partícula antes de desaparecer.
	 */
	ParticleColor endColor{
		1.0f,
		0.10f,
		0.0f,
		0.0f
	};

	//========================================================
	// FORMA DEL EMISOR
	//========================================================

	/**
	 * @brief Forma geométrica del área de generación de partículas.
	 */
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

	/**
	 * @brief Modo de mezcla gráfica (blending) utilizado para dibujar las partículas.
	 */
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

	/**
	 * @brief Constructor por defecto.
	 * @details Inicializa el componente con el tipo PARTICLE_EMITTER y crea una capa predeterminada.
	 */
	ParticleEmitterComponent()
		: Component(
			ComponentType::PARTICLE_EMITTER
		)
	{
		// Todo emisor comienza con una capa predeterminada.
		layers.emplace_back();
	}

	/**
	 * @brief Destructor virtual de la clase.
	 */
	~ParticleEmitterComponent() override = default;

	//========================================================
	// COMPONENT
	//========================================================

	/**
	 * @brief Inicializa los datos y el estado del componente.
	 */
	void init() override
	{
	}

	/**
	 * @brief Actualiza la lógica interna del componente en cada frame.
	 * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
	 */
	void update(float deltaTime) override
	{
		(void)deltaTime;
	}

	/**
	 * @brief Renderiza el emisor o sus gizmos de depuración.
	 * @param deviceContext Referencia al contexto de renderizado del dispositivo.
	 */
	void render(
		DeviceContext& deviceContext
	) override
	{
		(void)deviceContext;
	}

	/**
	 * @brief Libera y limpia las capas almacenadas en el componente.
	 */
	void destroy() override
	{
		layers.clear();
	}

	//========================================================
	// CONFIGURACION GENERAL
	//========================================================

	bool playOnStart = true; /**< Indica si la simulación comienza a reproducirse automáticamente al crearse. */
	bool looping = true;     /**< Indica si el efecto se repite continuamente en bucle. */

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

	float emissionRate = 20.0f; /**< [LEGACY] Tasa de emisión global. */

	unsigned int maxParticles = 500; /**< [LEGACY] Límite máximo de partículas global. */

	float particleLifetime = 2.0f; /**< [LEGACY] Tiempo de vida global de las partículas. */
	float startSpeed = 2.0f;       /**< [LEGACY] Velocidad inicial global. */

	float startSize = 0.20f; /**< [LEGACY] Tamaño inicial global. */
	float endSize = 0.0f;    /**< [LEGACY] Tamaño final global. */

	float gravityMultiplier = 0.0f; /**< [LEGACY] Factor de gravedad global. */

	/**
	 * @brief [LEGACY] Color inicial global.
	 */
	ParticleColor startColor{
		1.0f,
		0.65f,
		0.15f,
		1.0f
	};

	/**
	 * @brief [LEGACY] Color final global.
	 */
	ParticleColor endColor{
		1.0f,
		0.10f,
		0.0f,
		0.0f
	};

	/**
	 * @brief [LEGACY] Tamaño del emisor global.
	 */
	EU::Vector3 emitterSize{
		0.25f,
		0.25f,
		0.25f
	};
};