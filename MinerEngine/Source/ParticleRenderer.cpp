#include "ParticleRenderer.h"

#include "Device.h"
#include "DeviceContext.h"
#include "ECS/ParticleEmitterComponent.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"

#include <cstring>
#include <algorithm>
#include <cctype>

//============================================================
// DESTRUCTOR
//============================================================

ParticleRenderer::~ParticleRenderer()
{
	destroy();
}

//============================================================
// INIT
//============================================================

HRESULT
ParticleRenderer::init(
	Device& device,
	unsigned int initialParticleCapacity)
{
	if (!device.m_device)
	{
		return E_POINTER;
	}

	LayoutBuilder layout;

	layout
		.Add(
			"POSITION",
			DXGI_FORMAT_R32G32B32_FLOAT
		)
		.Add(
			"TEXCOORD",
			DXGI_FORMAT_R32G32_FLOAT
		)
		.Add(
			"COLOR",
			DXGI_FORMAT_R32G32B32A32_FLOAT
		);

	HRESULT result =
		m_shader.init(
			device,
			"Particle.hlsl",
			layout
		);

	if (FAILED(result))
	{
		ERROR(
			"ParticleRenderer",
			"init",
			"No se pudo cargar Particle.hlsl"
		);

		return result;
	}

	result =
		m_frameBuffer.init(
			device,
			sizeof(ParticleFrameData)
		);

	if (FAILED(result))
	{
		ERROR(
			"ParticleRenderer",
			"init",
			"No se pudo crear el constant buffer"
		);

		destroy();
		return result;
	}

	result =
		createVertexBuffer(
			device,
			initialParticleCapacity
		);

	if (FAILED(result))
	{
		ERROR(
			"ParticleRenderer",
			"init",
			"No se pudo crear el vertex buffer"
		);

		destroy();
		return result;
	}

	result =
		createBlendState(
			device
		);

	if (FAILED(result))
	{
		ERROR(
			"ParticleRenderer",
			"init",
			"No se pudo crear el blend state"
		);

		destroy();
		return result;
	}

	D3D11_DEPTH_STENCIL_DESC depthDescription{};

	depthDescription.DepthEnable = TRUE;
	depthDescription.DepthWriteMask =
		D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDescription.DepthFunc =
		D3D11_COMPARISON_LESS_EQUAL;

	depthDescription.StencilEnable = FALSE;

	result =
		device.m_device->CreateDepthStencilState(
			&depthDescription,
			&m_depthReadState
		);

	D3D11_RASTERIZER_DESC rasterizerDescription{};

	rasterizerDescription.FillMode =
		D3D11_FILL_SOLID;

	rasterizerDescription.CullMode =
		D3D11_CULL_NONE;

	rasterizerDescription.FrontCounterClockwise =
		FALSE;

	rasterizerDescription.DepthBias = 0;
	rasterizerDescription.DepthBiasClamp = 0.0f;
	rasterizerDescription.SlopeScaledDepthBias = 0.0f;

	rasterizerDescription.DepthClipEnable =
		TRUE;

	rasterizerDescription.ScissorEnable =
		FALSE;

	rasterizerDescription.MultisampleEnable =
		FALSE;

	rasterizerDescription.AntialiasedLineEnable =
		FALSE;

	result =
		device.m_device->CreateRasterizerState(
			&rasterizerDescription,
			&m_noCullRasterizerState
		);

	if (FAILED(result))
	{
		ERROR(
			"ParticleRenderer",
			"init",
			"No se pudo crear el rasterizer sin culling"
		);

		destroy();
		return result;
	}

	if (FAILED(result))
	{
		ERROR(
			"ParticleRenderer",
			"init",
			"No se pudo crear el depth state"
		);

		destroy();
		return result;
	}

	
//========================================================
// SAMPLER PARA TEXTURAS DE PARTÍCULAS
//========================================================

D3D11_SAMPLER_DESC samplerDescription{};

samplerDescription.Filter =
D3D11_FILTER_MIN_MAG_MIP_LINEAR;

samplerDescription.AddressU =
D3D11_TEXTURE_ADDRESS_CLAMP;

samplerDescription.AddressV =
D3D11_TEXTURE_ADDRESS_CLAMP;

samplerDescription.AddressW =
D3D11_TEXTURE_ADDRESS_CLAMP;

samplerDescription.MipLODBias = 0.0f;
samplerDescription.MaxAnisotropy = 1;

samplerDescription.ComparisonFunc =
D3D11_COMPARISON_NEVER;

samplerDescription.BorderColor[0] = 0.0f;
samplerDescription.BorderColor[1] = 0.0f;
samplerDescription.BorderColor[2] = 0.0f;
samplerDescription.BorderColor[3] = 0.0f;

samplerDescription.MinLOD = 0.0f;
samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

result =
device.m_device->CreateSamplerState(
	&samplerDescription,
	&m_particleSampler
);

if (FAILED(result))
{
	ERROR(
		"ParticleRenderer",
		"init",
		"No se pudo crear el sampler de particulas"
	);

	destroy();
	return result;
}

if (FAILED(result))
{
	ERROR(
		"ParticleRenderer",
		"init",
		"No se pudo crear el sampler de particulas"
	);

	destroy();
	return result;
}

	MESSAGE(
		"ParticleRenderer",
		"init",
		"Renderizador de particulas inicializado"
	);

	return S_OK;
}

//============================================================
// CREAR VERTEX BUFFER DINAMICO
//============================================================

HRESULT
ParticleRenderer::createVertexBuffer(
	Device& device,
	unsigned int particleCapacity)
{
	if (!device.m_device)
	{
		return E_POINTER;
	}

	if (particleCapacity == 0)
	{
		particleCapacity = 1;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	D3D11_BUFFER_DESC bufferDescription{};

	bufferDescription.Usage =
		D3D11_USAGE_DYNAMIC;

	bufferDescription.ByteWidth =
		static_cast<unsigned int>(
			sizeof(ParticleVertex) *
			particleCapacity *
			6
			);

	bufferDescription.BindFlags =
		D3D11_BIND_VERTEX_BUFFER;

	bufferDescription.CPUAccessFlags =
		D3D11_CPU_ACCESS_WRITE;

	bufferDescription.MiscFlags = 0;
	bufferDescription.StructureByteStride = 0;

	const HRESULT result =
		device.CreateBuffer(
			&bufferDescription,
			nullptr,
			&m_vertexBuffer
		);

	if (SUCCEEDED(result))
	{
		m_particleCapacity =
			particleCapacity;
	}

	return result;
}

//============================================================
// CREAR BLEND ADITIVO
//============================================================

HRESULT
ParticleRenderer::createBlendState(
	Device& device)
{
	if (!device.m_device)
	{
		return E_POINTER;
	}

	//========================================================
	// MEZCLA ADITIVA
	//========================================================

	D3D11_BLEND_DESC additiveDescription{};

	additiveDescription.AlphaToCoverageEnable =
		FALSE;

	additiveDescription.IndependentBlendEnable =
		FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC&
		additiveTarget =
		additiveDescription.RenderTarget[0];

	additiveTarget.BlendEnable = TRUE;

	additiveTarget.SrcBlend =
		D3D11_BLEND_SRC_ALPHA;

	additiveTarget.DestBlend =
		D3D11_BLEND_ONE;

	additiveTarget.BlendOp =
		D3D11_BLEND_OP_ADD;

	additiveTarget.SrcBlendAlpha =
		D3D11_BLEND_ONE;

	additiveTarget.DestBlendAlpha =
		D3D11_BLEND_ONE;

	additiveTarget.BlendOpAlpha =
		D3D11_BLEND_OP_ADD;

	additiveTarget.RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT result =
		device.m_device->CreateBlendState(
			&additiveDescription,
			&m_additiveBlendState
		);

	if (FAILED(result))
	{
		return result;
	}

	//========================================================
	// MEZCLA ALPHA TRADICIONAL
	//========================================================

	D3D11_BLEND_DESC alphaDescription{};

	alphaDescription.AlphaToCoverageEnable =
		FALSE;

	alphaDescription.IndependentBlendEnable =
		FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC&
		alphaTarget =
		alphaDescription.RenderTarget[0];

	alphaTarget.BlendEnable = TRUE;

	alphaTarget.SrcBlend =
		D3D11_BLEND_SRC_ALPHA;

	alphaTarget.DestBlend =
		D3D11_BLEND_INV_SRC_ALPHA;

	alphaTarget.BlendOp =
		D3D11_BLEND_OP_ADD;

	alphaTarget.SrcBlendAlpha =
		D3D11_BLEND_ONE;

	alphaTarget.DestBlendAlpha =
		D3D11_BLEND_INV_SRC_ALPHA;

	alphaTarget.BlendOpAlpha =
		D3D11_BLEND_OP_ADD;

	alphaTarget.RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	result =
		device.m_device->CreateBlendState(
			&alphaDescription,
			&m_alphaBlendState
		);

	if (FAILED(result))
	{
		return result;
	}

	return S_OK;
}

//============================================================
// CONSTRUIR BILLBOARDS
//============================================================

void
ParticleRenderer::buildBillboards(
	const Camera& camera,
	const std::vector<Particle>& particles)
{
	m_vertices.clear();

	const EU::Vector3 cameraRight =
		camera.GetRight();

	const EU::Vector3 cameraUp =
		camera.GetUp();

	for (const Particle& particle : particles)
	{
		if (!particle.alive)
		{
			continue;
		}

		const float halfSize =
			particle.size * 0.5f;

		const EU::Vector3 right{
			cameraRight.x * halfSize,
			cameraRight.y * halfSize,
			cameraRight.z * halfSize
		};

		const EU::Vector3 up{
			cameraUp.x * halfSize,
			cameraUp.y * halfSize,
			cameraUp.z * halfSize
		};

		const EU::Vector3 bottomLeft{
			particle.position.x - right.x - up.x,
			particle.position.y - right.y - up.y,
			particle.position.z - right.z - up.z
		};

		const EU::Vector3 topLeft{
			particle.position.x - right.x + up.x,
			particle.position.y - right.y + up.y,
			particle.position.z - right.z + up.z
		};

		const EU::Vector3 topRight{
			particle.position.x + right.x + up.x,
			particle.position.y + right.y + up.y,
			particle.position.z + right.z + up.z
		};

		const EU::Vector3 bottomRight{
			particle.position.x + right.x - up.x,
			particle.position.y + right.y - up.y,
			particle.position.z + right.z - up.z
		};

		// Primer triángulo.
		addBillboardVertex(
			bottomLeft,
			0.0f,
			1.0f,
			particle.color
		);

		addBillboardVertex(
			topLeft,
			0.0f,
			0.0f,
			particle.color
		);

		addBillboardVertex(
			topRight,
			1.0f,
			0.0f,
			particle.color
		);

		// Segundo triángulo.
		addBillboardVertex(
			bottomLeft,
			0.0f,
			1.0f,
			particle.color
		);

		addBillboardVertex(
			topRight,
			1.0f,
			0.0f,
			particle.color
		);

		addBillboardVertex(
			bottomRight,
			1.0f,
			1.0f,
			particle.color
		);
	}
}

void
ParticleRenderer::addBillboardVertex(
	const EU::Vector3& position,
	float texCoordX,
	float texCoordY,
	const ParticleColor& color)
{
	ParticleVertex vertex;

	vertex.position = position;
	vertex.texCoordX = texCoordX;
	vertex.texCoordY = texCoordY;
	vertex.color = color;

	m_vertices.push_back(
		vertex
	);
}

//============================================================
// CARGAR TEXTURA DE PARTÍCULAS
//============================================================

ID3D11ShaderResourceView*
ParticleRenderer::getOrLoadParticleTexture(
	Device& device,
	const char* texturePath)
{
	if (!texturePath ||
		texturePath[0] == '\0')
	{
		return nullptr;
	}

	const std::string path =
		texturePath;

	// Si ya intentamos cargar esta ruta, reutilizar el resultado.
	const auto cachedTexture =
		m_particleTextures.find(
			path
		);

	if (cachedTexture !=
		m_particleTextures.end())
	{
		if (!cachedTexture->second)
		{
			return nullptr;
		}

		return
			cachedTexture
			->second
			->m_textureFromImg;
	}

	std::string lowerPath =
		path;

	std::transform(
		lowerPath.begin(),
		lowerPath.end(),
		lowerPath.begin(),
		[](unsigned char character)
		{
			return static_cast<char>(
				std::tolower(character)
				);
		}
	);

	const std::string pngExtension =
		".png";

	if (lowerPath.size() <
		pngExtension.size() ||
		lowerPath.substr(
			lowerPath.size() -
			pngExtension.size()
		) != pngExtension)
	{
		ERROR(
			"ParticleRenderer",
			"getOrLoadParticleTexture",
			"Por ahora solamente se permiten texturas PNG"
		);

		m_particleTextures.emplace(
			path,
			nullptr
		);

		return nullptr;
	}

	// Texture::init agrega la extensión automáticamente.
	// Por eso debemos retirarla de la ruta.
	const std::string pathWithoutExtension =
		path.substr(
			0,
			path.size() -
			pngExtension.size()
		);

	std::unique_ptr<Texture> texture =
		std::make_unique<Texture>();

	const HRESULT result =
		texture->init(
			device,
			pathWithoutExtension,
			PNG
		);

	if (FAILED(result) ||
		!texture->m_textureFromImg)
	{
		ERROR(
			"ParticleRenderer",
			"getOrLoadParticleTexture",
			"No se pudo cargar la textura PNG de particulas"
		);

		texture->destroy();

		m_particleTextures.emplace(
			path,
			nullptr
		);

		return nullptr;
	}

	ID3D11ShaderResourceView* shaderResource =
		texture->m_textureFromImg;

	m_particleTextures.emplace(
		path,
		std::move(texture)
	);

	MESSAGE(
		"ParticleRenderer",
		"getOrLoadParticleTexture",
		"Textura PNG de particulas cargada"
	);

	return shaderResource;
}

//============================================================
// RENDER EMITTER
//============================================================

void
ParticleRenderer::renderEmitter(
	DeviceContext& deviceContext,
	Device& device,
	const Camera& camera,
	const ParticleSystem& particleSystem,
	const ParticleEmitterComponent& emitter)
{
	if (!isReady() ||
		!deviceContext.m_deviceContext ||
		!emitter.isEnabled())
	{
		return;
	}

	ID3D11ShaderResourceView* nullParticleTexture =
		nullptr;

	deviceContext.m_deviceContext
		->PSSetShaderResources(
			0,
			1,
			&nullParticleTexture
		);

	ID3D11SamplerState* nullParticleSampler =
		nullptr;

	deviceContext.m_deviceContext
		->PSSetSamplers(
			0,
			1,
			&nullParticleSampler
		);



	//========================================================
	// DATOS DE CÁMARA
	//========================================================

	ParticleFrameData frameData{};

	const XMMATRIX viewProjection =
		camera.getView() *
		camera.getProj();

	XMStoreFloat4x4(
		&frameData.viewProjection,
		XMMatrixTranspose(
			viewProjection
		)
	);

	m_frameBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&frameData,
		0,
		0
	);

	m_shader.render(
		deviceContext
	);

	m_frameBuffer.render(
		deviceContext,
		0,
		1,
		true
	);

	//========================================================
	// ESTADOS DE RENDER
	//========================================================

	deviceContext.IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	deviceContext.RSSetState(
		m_noCullRasterizerState
	);

	// Leer la profundidad, pero no escribir en ella.
	deviceContext.m_deviceContext
		->OMSetDepthStencilState(
			m_depthReadState,
			0
		);

	//========================================================
	// DIBUJAR TODAS LAS CAPAS
	//========================================================

	for (std::size_t layerIndex = 0;
		layerIndex < emitter.layers.size();
		++layerIndex)
	{
		const ParticleLayer& layer =
			emitter.layers[layerIndex];

		if (!layer.enabled)
		{
			continue;
		}

		const std::vector<Particle>* particles =
			particleSystem.getParticles(
				emitter,
				layerIndex
			);

		if (!particles)
		{
			continue;
		}

		buildBillboards(
			camera,
			*particles
		);

		if (m_vertices.empty())
		{
			continue;
		}

		//====================================================
// TEXTURA DE ESTA CAPA
//====================================================

		ID3D11ShaderResourceView* particleTexture =
			getOrLoadParticleTexture(
				device,
				layer.texturePath
			);

		// Indicar al shader si debe utilizar PNG o el círculo.
		frameData.textureSettings =
			XMFLOAT4(
				particleTexture ? 1.0f : 0.0f,
				0.0f,
				0.0f,
				0.0f
			);

		// Actualizar el constant buffer para esta capa.
		m_frameBuffer.update(
			deviceContext,
			nullptr,
			0,
			nullptr,
			&frameData,
			0,
			0
		);

		// Vincular el PNG al slot t0.
		// nullptr permite usar el círculo de respaldo.
		deviceContext.m_deviceContext
			->PSSetShaderResources(
				0,
				1,
				&particleTexture
			);

		// Vincular el sampler al slot s0.
		deviceContext.m_deviceContext
			->PSSetSamplers(
				0,
				1,
				&m_particleSampler
			);

		//====================================================
		// AMPLIAR VERTEX BUFFER SI ES NECESARIO
		//====================================================

		const unsigned int requiredParticles =
			static_cast<unsigned int>(
				m_vertices.size() / 6
				);

		if (requiredParticles >
			m_particleCapacity)
		{
			unsigned int newCapacity =
				m_particleCapacity > 0
				? m_particleCapacity * 2
				: 1;

			if (newCapacity <
				requiredParticles)
			{
				newCapacity =
					requiredParticles;
			}

			const HRESULT resizeResult =
				createVertexBuffer(
					device,
					newCapacity
				);

			if (FAILED(resizeResult))
			{
				ERROR(
					"ParticleRenderer",
					"renderEmitter",
					"No se pudo ampliar el vertex buffer"
				);

				continue;
			}
		}

		//====================================================
		// COPIAR VÉRTICES AL BUFFER
		//====================================================

		D3D11_MAPPED_SUBRESOURCE
			mappedResource{};

		const HRESULT mapResult =
			deviceContext.m_deviceContext->Map(
				m_vertexBuffer,
				0,
				D3D11_MAP_WRITE_DISCARD,
				0,
				&mappedResource
			);

		if (FAILED(mapResult))
		{
			ERROR(
				"ParticleRenderer",
				"renderEmitter",
				"No se pudo actualizar el vertex buffer"
			);

			continue;
		}

		std::memcpy(
			mappedResource.pData,
			m_vertices.data(),
			sizeof(ParticleVertex) *
			m_vertices.size()
		);

		deviceContext.m_deviceContext->Unmap(
			m_vertexBuffer,
			0
		);

		//====================================================
		// CONFIGURAR VERTEX BUFFER
		//====================================================

		const unsigned int stride =
			sizeof(ParticleVertex);

		const unsigned int offset = 0;

		deviceContext.IASetVertexBuffers(
			0,
			1,
			&m_vertexBuffer,
			&stride,
			&offset
		);

		//====================================================
		// DIBUJAR ESTA CAPA
		//====================================================

		//====================================================
        // BLEND MODE DE ESTA CAPA
        //====================================================

		ID3D11BlendState* layerBlendState =
			layer.blendMode ==
			ParticleBlendMode::Alpha
			? m_alphaBlendState
			: m_additiveBlendState;

		deviceContext.OMSetBlendState(
			layerBlendState,
			m_blendFactor,
			0xffffffff
		);

		deviceContext.m_deviceContext->Draw(
			static_cast<unsigned int>(
				m_vertices.size()
				),
			0
		);

		++deviceContext.m_drawCallCount;
	}

	//========================================================
	// RESTAURAR ESTADOS
	//========================================================

	// Restaurar Blend State directamente con Direct3D.
	// El wrapper del motor no acepta nullptr.
	deviceContext.m_deviceContext
		->OMSetBlendState(
			nullptr,
			m_blendFactor,
			0xffffffff
		);

	deviceContext.m_deviceContext
		->OMSetDepthStencilState(
			nullptr,
			0
		);

	deviceContext.m_deviceContext
		->RSSetState(
			nullptr
		);
}

//============================================================
// ESTADO
//============================================================

bool
ParticleRenderer::isReady() const
{
	return
		m_shader.m_VertexShader != nullptr &&
		m_shader.m_PixelShader != nullptr &&
		m_frameBuffer.m_buffer != nullptr &&
		m_vertexBuffer != nullptr &&
		m_additiveBlendState != nullptr &&
		m_alphaBlendState != nullptr &&
		m_depthReadState != nullptr &&
		m_noCullRasterizerState != nullptr &&
		m_particleSampler != nullptr;
}

//============================================================
// DESTROY
//============================================================

void
ParticleRenderer::destroy()
{

	// Destruir todas las texturas almacenadas en caché.
	for (auto& textureEntry :
		m_particleTextures)
	{
		if (textureEntry.second)
		{
			textureEntry.second->destroy();
		}
	}

	m_particleTextures.clear();

	if (m_particleSampler)
	{
		m_particleSampler->Release();
		m_particleSampler = nullptr;
	}
	if (m_noCullRasterizerState)
	{
		m_noCullRasterizerState->Release();
		m_noCullRasterizerState = nullptr;
	}

	if (m_depthReadState)
	{
		m_depthReadState->Release();
		m_depthReadState = nullptr;
	}

	if (m_alphaBlendState)
	{
		m_alphaBlendState->Release();
		m_alphaBlendState = nullptr;
	}

	if (m_additiveBlendState)
	{
		m_additiveBlendState->Release();
		m_additiveBlendState = nullptr;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	m_frameBuffer.destroy();
	m_shader.destroy();

	m_vertices.clear();
	m_particleCapacity = 0;
}