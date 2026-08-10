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
 */
class ParticleRenderer
{
public:

	ParticleRenderer() = default;

	~ParticleRenderer();

	ParticleRenderer(
		const ParticleRenderer&
	) = delete;

	ParticleRenderer& operator=(
		const ParticleRenderer&
		) = delete;

	//========================================================
	// INICIALIZACION
	//========================================================

	HRESULT init(
		Device& device,
		unsigned int initialParticleCapacity = 1000
	);

	//========================================================
	// RENDER
	//========================================================

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

	void destroy();

	bool isReady() const;

private:

	//========================================================
	// VERTICE DE PARTICULA
	//========================================================

	struct ParticleVertex
	{
		EU::Vector3 position{
			0.0f,
			0.0f,
			0.0f
		};

		float texCoordX = 0.0f;
		float texCoordY = 0.0f;

		ParticleColor color{
			1.0f,
			1.0f,
			1.0f,
			1.0f
		};
	};

	//========================================================
	// CONSTANT BUFFER
	//========================================================

	struct ParticleFrameData
	{
		XMFLOAT4X4 viewProjection{};

		/**
		 * x = 1 cuando la capa utiliza una textura.
		 * x = 0 cuando utiliza el círculo generado.
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

	HRESULT createVertexBuffer(
		Device& device,
		unsigned int particleCapacity
	);

	HRESULT createBlendState(
		Device& device
	);

	//========================================================
	// BILLBOARDS
	//========================================================

	/**
	 * @brief Construye los billboards de una capa.
	 *
	 * Cuando depthSorting es true, las partículas se ordenan
	 * desde la más lejana hasta la más cercana a la cámara.
	 */
	void buildBillboards(
		const Camera& camera,
		const std::vector<Particle>& particles,
		bool depthSorting
	);

	/**
	 * @brief Agrega los seis vértices de una partícula.
	 */
	void addParticleBillboard(
		const Camera& camera,
		const Particle& particle
	);

	void addBillboardVertex(
		const EU::Vector3& position,
		float texCoordX,
		float texCoordY,
		const ParticleColor& color
	);

	/**
	 * @brief Calcula la distancia cuadrada entre dos posiciones.
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
	 */
	ID3D11ShaderResourceView*
		getOrLoadParticleTexture(
			Device& device,
			const char* texturePath
		);

	//========================================================
	// RECURSOS
	//========================================================

	ShaderProgram m_shader;

	Buffer m_frameBuffer;

	ID3D11Buffer* m_vertexBuffer =
		nullptr;

	ID3D11BlendState* m_additiveBlendState =
		nullptr;

	ID3D11BlendState* m_alphaBlendState =
		nullptr;

	ID3D11DepthStencilState* m_depthReadState =
		nullptr;

	ID3D11RasterizerState* m_noCullRasterizerState =
		nullptr;

	ID3D11SamplerState* m_particleSampler =
		nullptr;

	//========================================================
	// CACHE DE TEXTURAS
	//========================================================

	std::unordered_map<
		std::string,
		std::unique_ptr<Texture>
	> m_particleTextures;

	//========================================================
	// DATOS TEMPORALES
	//========================================================

	std::vector<ParticleVertex> m_vertices;

	/**
	 * @brief Punteros usados para ordenar sin copiar partículas.
	 */
	std::vector<const Particle*>
		m_sortedParticles;

	unsigned int m_particleCapacity =
		0;

	float m_blendFactor[4]{
		0.0f,
		0.0f,
		0.0f,
		0.0f
	};
};