#include "Rendering/PostProcessSystem.h"

#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"

//============================================================
// DESTRUCTOR
//============================================================

PostProcessSystem::~PostProcessSystem()
{
	destroy();
}

//============================================================
// INIT
//============================================================

HRESULT
PostProcessSystem::init(
	Device& device,
	unsigned int width,
	unsigned int height)
{
	// Evitar conservar recursos de una inicialización anterior.
	destroy();

	// El fullscreen quad utiliza el mismo layout
	// que el lighting pass del deferred.
	LayoutBuilder fullscreenBuilder;

	fullscreenBuilder
		.Add(
			"POSITION",
			DXGI_FORMAT_R32G32B32_FLOAT
		)
		.Add(
			"NORMAL",
			DXGI_FORMAT_R32G32B32_FLOAT
		)
		.Add(
			"TANGENT",
			DXGI_FORMAT_R32G32B32_FLOAT
		)
		.Add(
			"BITANGENT",
			DXGI_FORMAT_R32G32B32_FLOAT
		)
		.Add(
			"TEXCOORD",
			DXGI_FORMAT_R32G32_FLOAT
		);

	//========================================================
	// SHADER DE TONEMAPPING
	//========================================================

	HRESULT result =
		m_tonemappingShader.init(
			device,
			"Tonemapping.hlsl",
			fullscreenBuilder
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo cargar Tonemapping.hlsl"
		);

		destroy();
		return result;
	}

	result =
		m_bloomExtractShader.init(
			device,
			"BloomExtract.hlsl",
			fullscreenBuilder
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo cargar BloomExtract.hlsl"
		);

		return result;
	}

	result =
		m_bloomBlurShader.init(
			device,
			"BloomBlur.hlsl",
			fullscreenBuilder
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo cargar BloomBlur.hlsl"
		);

		return result;
	}

	result =
		m_bloomBlurBuffer.init(
			device,
			sizeof(BloomBlurData)
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo crear el constant buffer de Bloom Blur"
		);

		return result;
	}

	//========================================================
	// SAMPLER
	//========================================================

	result =
		m_linearSampler.init(
			device
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo crear el sampler"
		);

		destroy();
		return result;
	}

	//========================================================
	// CONSTANT BUFFER
	//========================================================

	result =
		m_tonemappingBuffer.init(
			device,
			sizeof(TonemappingData)
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo crear el constant buffer"
		);

		destroy();
		return result;
	}

	result =
		m_bloomBuffer.init(
			device,
			sizeof(BloomData)
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo crear el constant buffer de Bloom"
		);

		return result;
	}
	//========================================================
	// RENDER TARGET HDR
	//========================================================

	result =
		resize(
			device,
			width,
			height
		);

	if (FAILED(result))
	{
		destroy();
		return result;
	}

	MESSAGE(
		"PostProcessSystem",
		"init",
		"Tonemapping inicializado correctamente"
	);

	return S_OK;
}

//============================================================
// RESIZE
//============================================================

HRESULT
PostProcessSystem::resize(
	Device& device,
	unsigned int width,
	unsigned int height)
{
	// Evitar texturas de tamaño cero.
	if (width < 64)
	{
		width = 64;
	}

	if (height < 64)
	{
		height = 64;
	}

	// No recrear si el tamaño no cambió.
	if (m_width == width &&
		m_height == height &&
		isReady())
	{
		return S_OK;
	}

	destroyHdrResources();

	const HRESULT result =
		createHdrResources(
			device,
			width,
			height
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"resize",
			"No se pudo crear el render target HDR"
		);

		m_width = 0;
		m_height = 0;

		return result;
	}

	m_width = width;
	m_height = height;

	MESSAGE(
		"PostProcessSystem",
		"resize",
		"Render target HDR creado correctamente"
	);

	return S_OK;
}

//============================================================
// CREAR RECURSOS HDR
//============================================================

HRESULT
PostProcessSystem::createHdrResources(
	Device& device,
	unsigned int width,
	unsigned int height)
{
	const DXGI_FORMAT hdrFormat =
		DXGI_FORMAT_R16G16B16A16_FLOAT;

	//========================================================
	// TEXTURA HDR PRINCIPAL
	//========================================================

	HRESULT result =
		m_hdrTexture.init(
			device,
			width,
			height,
			hdrFormat,
			D3D11_BIND_RENDER_TARGET |
			D3D11_BIND_SHADER_RESOURCE,
			1,
			0
		);

	if (FAILED(result))
	{
		return result;
	}

	result =
		m_hdrRTV.init(
			device,
			m_hdrTexture,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			hdrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_hdrSRV.init(
			device,
			m_hdrTexture,
			hdrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	//========================================================
	// TAMAÑO DE BLOOM
	//========================================================

	unsigned int bloomWidth =
		width / 2;

	unsigned int bloomHeight =
		height / 2;

	if (bloomWidth < 1)
	{
		bloomWidth = 1;
	}

	if (bloomHeight < 1)
	{
		bloomHeight = 1;
	}

	//========================================================
	// TEXTURA BLOOM A
	//========================================================

	result =
		m_bloomTextureA.init(
			device,
			bloomWidth,
			bloomHeight,
			hdrFormat,
			D3D11_BIND_RENDER_TARGET |
			D3D11_BIND_SHADER_RESOURCE,
			1,
			0
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_bloomRTVA.init(
			device,
			m_bloomTextureA,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			hdrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_bloomSRVA.init(
			device,
			m_bloomTextureA,
			hdrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	//========================================================
	// TEXTURA BLOOM B
	//========================================================

	result =
		m_bloomTextureB.init(
			device,
			bloomWidth,
			bloomHeight,
			hdrFormat,
			D3D11_BIND_RENDER_TARGET |
			D3D11_BIND_SHADER_RESOURCE,
			1,
			0
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_bloomRTVB.init(
			device,
			m_bloomTextureB,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			hdrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_bloomSRVB.init(
			device,
			m_bloomTextureB,
			hdrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	MESSAGE(
		"PostProcessSystem",
		"createHdrResources",
		"Texturas auxiliares de Bloom creadas"
	);

	return S_OK;
}
//============================================================
// DESTRUIR RECURSOS HDR
//============================================================

void
PostProcessSystem::destroyHdrResources()
{
	// Bloom B
	m_bloomRTVB.destroy();
	m_bloomSRVB.destroy();
	m_bloomTextureB.destroy();

	// Bloom A
	m_bloomRTVA.destroy();
	m_bloomSRVA.destroy();
	m_bloomTextureA.destroy();

	// HDR principal
	m_hdrRTV.destroy();
	m_hdrSRV.destroy();
	m_hdrTexture.destroy();
}
//============================================================
// DESTROY COMPLETO
//============================================================

void
PostProcessSystem::destroy()
{
	//========================================================
	// TEXTURAS Y RENDER TARGETS
	//========================================================

	destroyHdrResources();

	//========================================================
	// CONSTANT BUFFERS
	//========================================================

	m_bloomBlurBuffer.destroy();
	m_bloomBuffer.destroy();
	m_tonemappingBuffer.destroy();

	//========================================================
	// SAMPLERS
	//========================================================

	m_linearSampler.destroy();

	//========================================================
	// SHADERS
	//========================================================

	m_bloomBlurShader.destroy();
	m_bloomExtractShader.destroy();
	m_tonemappingShader.destroy();

	//========================================================
	// ESTADO
	//========================================================

	m_width = 0;
	m_height = 0;
}
//============================================================
// GET HDR RTV
//============================================================

ID3D11RenderTargetView*
PostProcessSystem::getHdrRTV() const
{
	return m_hdrRTV.get();
}

//============================================================
// GET HDR SRV
//============================================================

ID3D11ShaderResourceView*
PostProcessSystem::getHdrSRV() const
{
	return m_hdrSRV.m_textureFromImg;
}
//============================================================
// APLICAR TONEMAPPING
//============================================================
//============================================================
// EXTRAER ZONAS BRILLANTES PARA BLOOM
//============================================================

void
PostProcessSystem::extractBloom(
	DeviceContext& deviceContext,
	Buffer& fullscreenVertexBuffer,
	Buffer& fullscreenIndexBuffer,
	RasterizerState& fullscreenRasterizer,
	DepthStencilState& disabledDepthStencil)
{
	if (!isReady() ||
		!isBloomEnabled())
	{
		return;
	}
	unsigned int bloomWidth =
		m_width / 2;

	unsigned int bloomHeight =
		m_height / 2;

	if (bloomWidth < 1)
	{
		bloomWidth = 1;
	}

	if (bloomHeight < 1)
	{
		bloomHeight = 1;
	}

	// Desconectar el HDR como render target antes de leerlo.
	ID3D11RenderTargetView* nullRenderTarget[1] =
	{
		nullptr
	};

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);

	// Configurar el viewport de Bloom a media resolución.
	D3D11_VIEWPORT bloomViewport{};

	bloomViewport.TopLeftX = 0.0f;
	bloomViewport.TopLeftY = 0.0f;
	bloomViewport.Width =
		static_cast<float>(bloomWidth);
	bloomViewport.Height =
		static_cast<float>(bloomHeight);
	bloomViewport.MinDepth = 0.0f;
	bloomViewport.MaxDepth = 1.0f;

	deviceContext.RSSetViewports(
		1,
		&bloomViewport
	);

	// Seleccionar Bloom A como destino.
	ID3D11RenderTargetView* bloomTarget =
		m_bloomRTVA.get();

	deviceContext.OMSetRenderTargets(
		1,
		&bloomTarget,
		nullptr
	);

	const float clearColor[4] =
	{
		0.0f,
		0.0f,
		0.0f,
		0.0f
	};

	deviceContext.ClearRenderTargetView(
		bloomTarget,
		clearColor
	);

	// Leer la escena HDR.
	ID3D11ShaderResourceView* hdrResource =
		getHdrSRV();

	deviceContext.PSSetShaderResources(
		0,
		1,
		&hdrResource
	);

	disabledDepthStencil.render(
		deviceContext,
		0,
		false
	);

	fullscreenRasterizer.render(
		deviceContext
	);

	m_linearSampler.render(
		deviceContext,
		0,
		1
	);

	m_bloomExtractShader.render(
		deviceContext
	);

	m_bloomData.texelSizeX =
		1.0f /
		static_cast<float>(bloomWidth);

	m_bloomData.texelSizeY =
		1.0f /
		static_cast<float>(bloomHeight);

	m_bloomBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_bloomData,
		0,
		0
	);

	m_bloomBuffer.render(
		deviceContext,
		0,
		1,
		true
	);

	deviceContext.IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	fullscreenVertexBuffer.render(
		deviceContext,
		0,
		1
	);

	fullscreenIndexBuffer.render(
		deviceContext,
		0,
		1,
		false,
		DXGI_FORMAT_R32_UINT
	);

	deviceContext.DrawIndexed(
		6,
		0,
		0
	);

	// Desconectar el HDR del shader.
	ID3D11ShaderResourceView* nullResource[1] =
	{
		nullptr
	};

	deviceContext.PSSetShaderResources(
		0,
		1,
		nullResource
	);
}

//============================================================
// DESENFOCAR BLOOM
//============================================================

void
PostProcessSystem::blurBloom(
	DeviceContext& deviceContext,
	Buffer& fullscreenVertexBuffer,
	Buffer& fullscreenIndexBuffer,
	RasterizerState& fullscreenRasterizer,
	DepthStencilState& disabledDepthStencil)
{
	if (!isReady() ||
		!isBloomEnabled())
	{
		return;
	}

	unsigned int bloomWidth =
		m_width / 2;

	unsigned int bloomHeight =
		m_height / 2;

	if (bloomWidth < 1)
	{
		bloomWidth = 1;
	}

	if (bloomHeight < 1)
	{
		bloomHeight = 1;
	}

	D3D11_VIEWPORT bloomViewport{};

	bloomViewport.TopLeftX = 0.0f;
	bloomViewport.TopLeftY = 0.0f;
	bloomViewport.Width =
		static_cast<float>(bloomWidth);
	bloomViewport.Height =
		static_cast<float>(bloomHeight);
	bloomViewport.MinDepth = 0.0f;
	bloomViewport.MaxDepth = 1.0f;

	deviceContext.RSSetViewports(
		1,
		&bloomViewport
	);

	disabledDepthStencil.render(
		deviceContext,
		0,
		false
	);

	fullscreenRasterizer.render(
		deviceContext
	);

	m_linearSampler.render(
		deviceContext,
		0,
		1
	);

	m_bloomBlurShader.render(
		deviceContext
	);

	deviceContext.IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	fullscreenVertexBuffer.render(
		deviceContext,
		0,
		1
	);

	fullscreenIndexBuffer.render(
		deviceContext,
		0,
		1,
		false,
		DXGI_FORMAT_R32_UINT
	);

	m_bloomBlurData.texelSizeX =
		1.0f /
		static_cast<float>(bloomWidth);

	m_bloomBlurData.texelSizeY =
		1.0f /
		static_cast<float>(bloomHeight);

	//========================================================
	// PASE HORIZONTAL: BLOOM A -> BLOOM B
	//========================================================

	ID3D11RenderTargetView* nullRenderTarget[1] =
	{
		nullptr
	};

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);

	ID3D11RenderTargetView* bloomTargetB =
		m_bloomRTVB.get();

	deviceContext.OMSetRenderTargets(
		1,
		&bloomTargetB,
		nullptr
	);

	const float clearColor[4] =
	{
		0.0f,
		0.0f,
		0.0f,
		0.0f
	};

	deviceContext.ClearRenderTargetView(
		bloomTargetB,
		clearColor
	);

	ID3D11ShaderResourceView* bloomResourceA =
		m_bloomSRVA.m_textureFromImg;

	deviceContext.PSSetShaderResources(
		0,
		1,
		&bloomResourceA
	);

	m_bloomBlurData.directionX = 1.0f;
	m_bloomBlurData.directionY = 0.0f;

	m_bloomBlurBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_bloomBlurData,
		0,
		0
	);

	m_bloomBlurBuffer.render(
		deviceContext,
		0,
		1,
		true
	);

	deviceContext.DrawIndexed(
		6,
		0,
		0
	);

	ID3D11ShaderResourceView* nullResource[1] =
	{
		nullptr
	};

	deviceContext.PSSetShaderResources(
		0,
		1,
		nullResource
	);

	//========================================================
	// PASE VERTICAL: BLOOM B -> BLOOM A
	//========================================================

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);

	ID3D11RenderTargetView* bloomTargetA =
		m_bloomRTVA.get();

	deviceContext.OMSetRenderTargets(
		1,
		&bloomTargetA,
		nullptr
	);

	deviceContext.ClearRenderTargetView(
		bloomTargetA,
		clearColor
	);

	ID3D11ShaderResourceView* bloomResourceB =
		m_bloomSRVB.m_textureFromImg;

	deviceContext.PSSetShaderResources(
		0,
		1,
		&bloomResourceB
	);

	m_bloomBlurData.directionX = 0.0f;
	m_bloomBlurData.directionY = 1.0f;

	m_bloomBlurBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_bloomBlurData,
		0,
		0
	);

	m_bloomBlurBuffer.render(
		deviceContext,
		0,
		1,
		true
	);

	deviceContext.DrawIndexed(
		6,
		0,
		0
	);

	deviceContext.PSSetShaderResources(
		0,
		1,
		nullResource
	);

	// Dejar Bloom A desconectado como render target.
	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);
}

void
PostProcessSystem::renderTonemapping(
	DeviceContext& deviceContext,
	ID3D11RenderTargetView* outputRTV,
	Buffer& fullscreenVertexBuffer,
	Buffer& fullscreenIndexBuffer,
	RasterizerState& fullscreenRasterizer,
	DepthStencilState& disabledDepthStencil)
{
	if (!isReady() || outputRTV == nullptr)
	{
		return;
	}
	// Restaurar el viewport completo después del pase Bloom.
	D3D11_VIEWPORT fullViewport{};

	fullViewport.TopLeftX = 0.0f;
	fullViewport.TopLeftY = 0.0f;
	fullViewport.Width =
		static_cast<float>(m_width);
	fullViewport.Height =
		static_cast<float>(m_height);
	fullViewport.MinDepth = 0.0f;
	fullViewport.MaxDepth = 1.0f;

	deviceContext.RSSetViewports(
		1,
		&fullViewport
	);

	// Desconectar el HDR como Render Target antes de utilizarlo
	// como Shader Resource.
	ID3D11RenderTargetView* nullRenderTarget[1] =
	{
		nullptr
	};

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);

	// Seleccionar el render target final del viewport.
	deviceContext.OMSetRenderTargets(
		1,
		&outputRTV,
		nullptr
	);

	// Conectar la textura HDR al shader de tonemapping.
	ID3D11ShaderResourceView* postProcessResources[2] =
	{
		getHdrSRV(),
		m_bloomSRVA.m_textureFromImg
	};

	deviceContext.PSSetShaderResources(
		0,
		2,
		postProcessResources
	);

	// Configurar el fullscreen pass.
	disabledDepthStencil.render(
		deviceContext,
		0,
		false
	);

	fullscreenRasterizer.render(
		deviceContext
	);

	m_linearSampler.render(
		deviceContext,
		0,
		1
	);

	m_tonemappingShader.render(
		deviceContext
	);

	// Actualizar Exposure, Gamma y el estado del tonemapping.
	m_tonemappingBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_tonemappingData,
		0,
		0
	);

	m_tonemappingBuffer.render(
		deviceContext,
		0,
		1,
		true
	);
	m_bloomBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_bloomData,
		0,
		0
	);

	m_bloomBuffer.render(
		deviceContext,
		1,
		1,
		true
	);
	// Dibujar el fullscreen quad.
	deviceContext.IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	fullscreenVertexBuffer.render(
		deviceContext,
		0,
		1
	);

	fullscreenIndexBuffer.render(
		deviceContext,
		0,
		1,
		false,
		DXGI_FORMAT_R32_UINT
	);

	deviceContext.DrawIndexed(
		6,
		0,
		0
	);

	// Desconectar la textura HDR para evitar conflictos
	// en el siguiente frame.
	ID3D11ShaderResourceView* nullResources[2] =
	{
		nullptr,
		nullptr
	};

	deviceContext.PSSetShaderResources(
		0,
		2,
		nullResources
	);
}

//============================================================
// CONTROLES DE TONEMAPPING
//============================================================

void
PostProcessSystem::setTonemappingEnabled(
	bool enabled)
{
	m_tonemappingData.enableTonemapping =
		enabled ? 1.0f : 0.0f;
}

bool
PostProcessSystem::isTonemappingEnabled() const
{
	return
		m_tonemappingData.enableTonemapping >
		0.5f;
}

void
PostProcessSystem::setExposure(
	float exposure)
{
	if (exposure < 0.01f)
	{
		exposure = 0.01f;
	}

	if (exposure > 10.0f)
	{
		exposure = 10.0f;
	}

	m_tonemappingData.exposure =
		exposure;
}

float
PostProcessSystem::getExposure() const
{
	return m_tonemappingData.exposure;
}

void
PostProcessSystem::setGamma(
	float gamma)
{
	if (gamma < 0.1f)
	{
		gamma = 0.1f;
	}

	if (gamma > 4.0f)
	{
		gamma = 4.0f;
	}

	m_tonemappingData.gamma =
		gamma;
}

float
PostProcessSystem::getGamma() const
{
	return m_tonemappingData.gamma;
}

//============================================================
// CONTROLES DE BLOOM
//============================================================

void
PostProcessSystem::setBloomEnabled(
	bool enabled)
{
	m_bloomData.enabled =
		enabled ? 1.0f : 0.0f;
}

bool
PostProcessSystem::isBloomEnabled() const
{
	return
		m_bloomData.enabled >
		0.5f;
}

void
PostProcessSystem::setBloomThreshold(
	float threshold)
{
	if (threshold < 0.0f)
	{
		threshold = 0.0f;
	}

	if (threshold > 5.0f)
	{
		threshold = 5.0f;
	}

	m_bloomData.threshold =
		threshold;
}

float
PostProcessSystem::getBloomThreshold() const
{
	return m_bloomData.threshold;
}

void
PostProcessSystem::setBloomIntensity(
	float intensity)
{
	if (intensity < 0.0f)
	{
		intensity = 0.0f;
	}

	if (intensity > 5.0f)
	{
		intensity = 5.0f;
	}

	m_bloomData.intensity =
		intensity;
}

float
PostProcessSystem::getBloomIntensity() const
{
	return m_bloomData.intensity;
}