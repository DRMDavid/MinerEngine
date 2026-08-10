#pragma once

#include "Prerequisites.h"
#include "Buffer.h"
#include "ParticleSystem.h"
#include "ShaderProgram.h"
#include "Texture.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Camera;
class Device;
class DeviceContext;
class ParticleEmitterComponent;

/**
 * @class ParticleRenderer
 * @brief Convierte partículas CPU en billboards y las dibuja.
 * @details Se encarga del procesamiento de geometría de billboards orientados a la cámara,
 * la ordenación por profundidad (depth sorting), la gestión de estados de renderizado (blend states)
 * y el almacenamiento en caché de texturas de partículas.
 */
class ParticleRenderer
{
public:

	/** @brief Constructor por defecto. */
	ParticleRenderer() = default;

	/** @brief Destructor que asegura la liberación de recursos gráficos. */
	~ParticleRenderer();

	/** @brief Constructor de copia eliminado para evitar duplicación de recursos de GPU. */
	ParticleRenderer(
		const ParticleRenderer&
	) = delete;

	/** @brief Operador de asignación por copia eliminado. */
	ParticleRenderer& operator=(
		const ParticleRenderer&
		) = delete;

	//========================================================
	// INICIALIZACION
	//========================================================

	/**
	 * @brief Inicializa los shaders, estados de renderizado y buffers para el emisor.
	 * @param device Referencia al dispositivo gráfico.
	 * @param initialParticleCapacity Capacidad inicial de partículas reservada en el buffer de vértices.
	 * @return HRESULT indicando el resultado de la creación de recursos.
	 */
	HRESULT init(
		Device& device,
		unsigned int initialParticleCapacity = 1000
	);

	//========================================================
	// RENDER
	//========================================================

	/**
	 * @brief Renderiza todas las capas activas de un emisor de partículas.
	 * @param deviceContext Contexto del dispositivo para la ejecución de pases de dibujado.
	 * @param device Referencia al dispositivo gráfico para carga dinámica de recursos.
	 * @param camera Cámara activa para orientación de billboards y transformaciones de vista.
	 * @param particleSystem Referencia al sistema global de partículas.
	 * @param emitter Componente del emisor que contiene las capas y configuraciones a renderizar.
	 */
	void renderEmitter(
		DeviceContext& deviceContext,
		Device& device,
		const Camera& camera,
		const ParticleSystem& particleSystem,
		const ParticleEmitterComponent& emitter
	);

	//========================================================
	// DESTRUCCION
	//========================================================

	/**
	 * @brief Libera todos los recursos gráficos y limpia el caché de texturas.
	 */
	void destroy();

	/**
	 * @brief Comprueba si el renderizador está listo y sus recursos inicializados.
	 * @return True si los recursos requeridos son válidos, false en caso contrario.
	 */
	bool isReady() const;

private:

	//========================================================
	// VERTICE DE PARTICULA
	//========================================================

	/**
	 * @struct ParticleVertex
	 * @brief Estructura de vértice utilizada para la generación dinámica de billboards.
	 */
	struct ParticleVertex
	{
		EU::Vector3 position{
			0.0f,
			0.0f,
			0.0f
		}; /**< Posición 3D del vértice en el espacio de mundo. */

		float texCoordX = 0.0f; /**< Coordenada U del mapa de textura. */
		float texCoordY = 0.0f; /**< Coordenada V del mapa de textura. */

		ParticleColor color{
			1.0f,
			1.0f,
			1.0f,
			1.0f
		}; /**< Color RGBA del vértice. */
	};

	//========================================================
	// CONSTANT BUFFER
	//========================================================

	/**
	 * @struct ParticleFrameData
	 * @brief Datos constantes enviados al shader de partículas por cada pase de renderizado.
	 */
	struct ParticleFrameData
	{
		XMFLOAT4X4 viewProjection{}; /**< Matriz combinada de Vista y Proyección. */

		/**
		 * @brief Parámetros de configuración de la textura.
		 * @details x = 1 cuando la capa utiliza una textura; x = 0 cuando utiliza el círculo generado.
		 */
		XMFLOAT4 textureSettings{
			0.0f,
			0.0f,
			0.0f,
			0.0f
		};
	};

	//========================================================
	// CREACION DE RECURSOS
	//========================================================

	/**
	 * @brief Crea o redimensiona el buffer de vértices dinámico en GPU.
	 * @param device Dispositivo gráfico para la creación del recurso.
	 * @param particleCapacity Número máximo de partículas que soportará el buffer.
	 * @return HRESULT representando el resultado de la asignación.
	 */
	HRESULT createVertexBuffer(
		Device& device,
		unsigned int particleCapacity
	);

	/**
	 * @brief Crea los estados de mezcla (additive y alpha blend), profundidad y rasterizado.
	 * @param device Dispositivo gráfico.
	 * @return HRESULT de la operación de creación.
	 */
	HRESULT createBlendState(
		Device& device
	);

	//========================================================
	// BILLBOARDS
	//========================================================

	/**
	 * @brief Construye los billboards de una capa.
	 * @param camera Cámara utilizada para determinar la orientación y distancia.
	 * @param particles Vector de partículas a procesar.
	 * @param depthSorting Cuando es true, las partículas se ordenan desde la más lejana hasta la más cercana a la cámara.
	 */
	void buildBillboards(
		const Camera& camera,
		const std::vector<Particle>& particles,
		bool depthSorting
	);

	/**
	 * @brief Agrega los seis vértices de una partícula (dos triángulos) orientados a la cámara.
	 * @param camera Cámara hacia la cual se orientará el billboard.
	 * @param particle Referencia a la partícula procesada.
	 */
	void addParticleBillboard(
		const Camera& camera,
		const Particle& particle
	);

	/**
	 * @brief Añade un vértice individual al arreglo temporal de vértices.
	 * @param position Posición 3D del vértice.
	 * @param texCoordX Coordenada de textura U.
	 * @param texCoordY Coordenada de textura V.
	 * @param color Color del vértice.
	 */
	void addBillboardVertex(
		const EU::Vector3& position,
		float texCoordX,
		float texCoordY,
		const ParticleColor& color
	);

	/**
	 * @brief Calcula la distancia cuadrada entre dos posiciones.
	 * @param firstPosition Primer punto en el espacio.
	 * @param secondPosition Segundo punto en el espacio.
	 * @return Distancia euclidiana al cuadrado entre ambos puntos.
	 */
	static float calculateSquaredDistance(
		const EU::Vector3& firstPosition,
		const EU::Vector3& secondPosition
	);

	//========================================================
	// TEXTURAS
	//========================================================

	/**
	 * @brief Obtiene una textura del caché o carga un PNG.
	 * @param device Dispositivo gráfico para cargar la textura si no está en memoria.
	 * @param texturePath Ruta del archivo de la textura.
	 * @return Puntero a ID3D11ShaderResourceView de la textura, o nullptr si falla.
	 */
	ID3D11ShaderResourceView*
		getOrLoadParticleTexture(
			Device& device,
			const char* texturePath
		);

	//========================================================
	// RECURSOS
	//========================================================

	ShaderProgram m_shader; /**< Sombreador para el renderizado de partículas. */

	Buffer m_frameBuffer; /**< Constant buffer para datos por frame (ParticleFrameData). */

	ID3D11Buffer* m_vertexBuffer =
		nullptr; /**< Buffer de vértices dinámico de DirectX 11. */

	ID3D11BlendState* m_additiveBlendState =
		nullptr; /**< Estado de mezcla aditiva. */

	ID3D11BlendState* m_alphaBlendState =
		nullptr; /**< Estado de mezcla alfa. */

	ID3D11DepthStencilState* m_depthReadState =
		nullptr; /**< Estado de Depth/Stencil con lectura activada y escritura desactivada. */

	ID3D11RasterizerState* m_noCullRasterizerState =
		nullptr; /**< Estado de rasterizado sin culling de caras. */

	ID3D11SamplerState* m_particleSampler =
		nullptr; /**< Estado del muestreador de textura para partículas. */

	//========================================================
	// CACHE DE TEXTURAS
	//========================================================

	/**
	 * @brief Contenedor hash para evitar cargar múltiples veces la misma textura.
	 */
	std::unordered_map<
		std::string,
		std::unique_ptr<Texture>
	> m_particleTextures;

	//========================================================
	// DATOS TEMPORALES
	//========================================================

	std::vector<ParticleVertex> m_vertices; /**< Buffer local de vértices construidos en CPU. */

	/**
	 * @brief Punteros usados para ordenar sin copiar partículas.
	 */
	std::vector<const Particle*>
		m_sortedParticles;

	unsigned int m_particleCapacity =
		0; /**< Capacidad actual del buffer de vértices (expresada en número de partículas). */

	float m_blendFactor[4]{
		0.0f,
		0.0f,
		0.0f,
		0.0f
	}; /**< Factores de mezcla de color pasados a OMSetBlendState. */
};