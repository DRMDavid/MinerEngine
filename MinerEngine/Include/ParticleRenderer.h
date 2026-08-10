#pragma once

#include "Prerequisites.h"
#include "Buffer.h"
#include "ParticleSystem.h"
#include "ShaderProgram.h"
#include "Texture.h"

#include <cstddef>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class Camera;
class Device;
class DeviceContext;
class ParticleEmitterComponent;

/**
 * @class ParticleRenderer
 * @brief Convierte las partículas CPU en billboards y las dibuja.
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

	HRESULT init(
		Device& device,
		unsigned int initialParticleCapacity = 1000
	);

	void renderEmitter(
		DeviceContext& deviceContext,
		Device& device,
		const Camera& camera,
		const ParticleSystem& particleSystem,
		const ParticleEmitterComponent& emitter
	);

	void destroy();

	bool isReady() const;

private:

	/**
	 * @struct ParticleVertex
	 * @brief Vértice enviado al shader de partículas.
	 */
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

	/**
	 * @struct ParticleFrameData
	 * @brief Matriz utilizada por Particle.hlsl.
	 */
	struct ParticleFrameData
	{
		XMFLOAT4X4 viewProjection{};

		// x = 1 cuando la capa utiliza una textura.
		// x = 0 para utilizar el círculo generado por el shader.
		XMFLOAT4 textureSettings{
			0.0f,
			0.0f,
			0.0f,
			0.0f
		};
	};

	HRESULT createVertexBuffer(
		Device& device,
		unsigned int particleCapacity
	);

	HRESULT createBlendState(
		Device& device
	);

	void buildBillboards(
		const Camera& camera,
		const std::vector<Particle>& particles
	);

	void addBillboardVertex(
		const EU::Vector3& position,
		float texCoordX,
		float texCoordY,
		const ParticleColor& color
	);

	/**
    * @brief Obtiene una textura del caché o carga un PNG nuevo.
    * @param device Dispositivo gráfico.
    * @param texturePath Ruta completa del PNG.
    * @return SRV de la textura o nullptr si no existe.
    */
	ID3D11ShaderResourceView*
		getOrLoadParticleTexture(
			Device& device,
			const char* texturePath
		);

	ShaderProgram m_shader;
	Buffer m_frameBuffer;

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11BlendState* m_additiveBlendState = nullptr;
	ID3D11BlendState* m_alphaBlendState = nullptr;
	ID3D11DepthStencilState* m_depthReadState = nullptr;
	ID3D11RasterizerState* m_noCullRasterizerState = nullptr;

	// Sampler utilizado por los PNG de partículas.
	ID3D11SamplerState* m_particleSampler = nullptr;

	// Cada archivo se carga una sola vez y después se reutiliza.
	std::unordered_map<
		std::string,
		std::unique_ptr<Texture>
	> m_particleTextures;

	std::vector<ParticleVertex> m_vertices;

	unsigned int m_particleCapacity = 0;

	float m_blendFactor[4]{
		0.0f,
		0.0f,
		0.0f,
		0.0f
	};
};