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
	// Eliminar cualquier inicialización anterior.
	destroy();

	//========================================================
	// LAYOUT DEL FULLSCREEN QUAD
	//========================================================

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

	HRESULT result = S_OK;

	//========================================================
	// SHADER DE TONEMAPPING
	//========================================================

	result =
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

	//========================================================
	// SHADER DE EXTRACCIÓN BLOOM
	//========================================================

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

		destroy();
		return result;
	}

	//========================================================
	// SHADER DE DESENFOQUE BLOOM
	//========================================================

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

		destroy();
		return result;
	}

	//========================================================
	// SHADER DE FXAA
	//========================================================

	result =
		m_fxaaShader.init(
			device,
			"FXAA.hlsl",
			fullscreenBuilder
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo cargar FXAA.hlsl"
		);

		destroy();
		return result;
	}

	result =
		m_ssaoShader.init(
			device,
			"SSAO.hlsl",
			fullscreenBuilder
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo cargar SSAO.hlsl"
		);

		destroy();
		return result;
	}

	//========================================================
	// SAMPLER LINEAL
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
			"No se pudo crear el sampler lineal"
		);

		destroy();
		return result;
	}

	//========================================================
	// CONSTANT BUFFER DE TONEMAPPING
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
			"No se pudo crear el constant buffer de Tonemapping"
		);

		destroy();
		return result;
	}

	//========================================================
	// CONSTANT BUFFER DE BLOOM
	//========================================================

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

		destroy();
		return result;
	}

	//========================================================
	// CONSTANT BUFFER DE BLOOM BLUR
	//========================================================

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

		destroy();
		return result;
	}

	//========================================================
	// CONSTANT BUFFER DE FXAA
	//========================================================

	result =
		m_fxaaBuffer.init(
			device,
			sizeof(FxaaData)
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo crear el constant buffer de FXAA"
		);

		destroy();
		return result;
	}

	result =
		m_ssaoBuffer.init(
			device,
			sizeof(SsaoData)
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudo crear el constant buffer de SSAO"
		);

		destroy();
		return result;
	}

	//========================================================
	// CREAR RENDER TARGETS
	//========================================================

	result =
		resize(
			device,
			width,
			height
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"init",
			"No se pudieron crear los render targets"
		);

		destroy();
		return result;
	}

	MESSAGE(
		"PostProcessSystem",
		"init",
		"Post Processing inicializado correctamente"
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
// TEXTURA LDR PARA FXAA
//========================================================

	const DXGI_FORMAT ldrFormat =
		DXGI_FORMAT_R8G8B8A8_UNORM;

	result =
		m_ldrTexture.init(
			device,
			width,
			height,
			ldrFormat,
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
		m_ldrRTV.init(
			device,
			m_ldrTexture,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			ldrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_ldrSRV.init(
			device,
			m_ldrTexture,
			ldrFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	//========================================================
// TEXTURA SSAO
//========================================================

	const DXGI_FORMAT ssaoFormat =
		DXGI_FORMAT_R8_UNORM;

	result =
		m_ssaoTexture.init(
			device,
			width,
			height,
			ssaoFormat,
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
		m_ssaoRTV.init(
			device,
			m_ssaoTexture,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			ssaoFormat
		);

	if (FAILED(result))
	{
		destroyHdrResources();
		return result;
	}

	result =
		m_ssaoSRV.init(
			device,
			m_ssaoTexture,
			ssaoFormat
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
	// SSAO
	m_ssaoRTV.destroy();
	m_ssaoSRV.destroy();
	m_ssaoTexture.destroy();

	// Resultado LDR
	m_ldrRTV.destroy();
	m_ldrSRV.destroy();
	m_ldrTexture.destroy();

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

	m_ssaoBuffer.destroy();
	m_fxaaBuffer.destroy();
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

	m_ssaoShader.destroy();
	m_fxaaShader.destroy();
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
// GENERAR SSAO DESDE EL G-BUFFER
//============================================================

void
PostProcessSystem::renderSsao(
	DeviceContext& deviceContext,
	ID3D11ShaderResourceView* worldPositionSRV,
	ID3D11ShaderResourceView* normalRoughnessSRV,
	Buffer& fullscreenVertexBuffer,
	Buffer& fullscreenIndexBuffer,
	RasterizerState& fullscreenRasterizer,
	DepthStencilState& disabledDepthStencil)
{
	if (!isReady() ||
		!isSsaoEnabled() ||
		worldPositionSRV == nullptr ||
		normalRoughnessSRV == nullptr)
	{
		return;
	}

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

	ID3D11RenderTargetView* nullRenderTarget[1] =
	{
		nullptr
	};

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);

	ID3D11RenderTargetView* ssaoTarget =
		m_ssaoRTV.get();

	deviceContext.OMSetRenderTargets(
		1,
		&ssaoTarget,
		nullptr
	);

	const float clearColor[4] =
	{
		1.0f,
		1.0f,
		1.0f,
		1.0f
	};

	deviceContext.ClearRenderTargetView(
		ssaoTarget,
		clearColor
	);

	ID3D11ShaderResourceView* ssaoInputs[2] =
	{
		worldPositionSRV,
		normalRoughnessSRV
	};

	deviceContext.PSSetShaderResources(
		0,
		2,
		ssaoInputs
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

	m_ssaoShader.render(
		deviceContext
	);

	m_ssaoData.texelSizeX =
		1.0f /
		static_cast<float>(m_width);

	m_ssaoData.texelSizeY =
		1.0f /
		static_cast<float>(m_height);

	m_ssaoBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_ssaoData,
		0,
		0
	);

	m_ssaoBuffer.render(
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

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);
}

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

	// Tonemapping escribe en la textura LDR.
	// FXAA llevará después este resultado al viewport.
	ID3D11RenderTargetView* ldrTarget =
		m_ldrRTV.get();

	deviceContext.OMSetRenderTargets(
		1,
		&ldrTarget,
		nullptr
	);

	// Conectar la textura HDR al shader de tonemapping.
	ID3D11ShaderResourceView* postProcessResources[3] =
	{
		getHdrSRV(),
		m_bloomSRVA.m_textureFromImg,
		m_ssaoSRV.m_textureFromImg
	};

	deviceContext.PSSetShaderResources(
		0,
		3,
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

	m_ssaoBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_ssaoData,
		0,
		0
	);

	m_ssaoBuffer.render(
		deviceContext,
		2,
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
	ID3D11ShaderResourceView* nullResources[3] =
	{
		nullptr,
		nullptr,
		nullptr
	};

	deviceContext.PSSetShaderResources(
		0,
		3,
		nullResources
	);
}

//============================================================
// APLICAR FXAA Y PRESENTAR EN EL VIEWPORT
//============================================================

void
PostProcessSystem::renderFxaa(
	DeviceContext& deviceContext,
	ID3D11RenderTargetView* outputRTV,
	Buffer& fullscreenVertexBuffer,
	Buffer& fullscreenIndexBuffer,
	RasterizerState& fullscreenRasterizer,
	DepthStencilState& disabledDepthStencil)
{
	if (!isReady() ||
		outputRTV == nullptr)
	{
		return;
	}

	// Configurar el viewport completo.
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

	// Desconectar la textura LDR como render target.
	ID3D11RenderTargetView* nullRenderTarget[1] =
	{
		nullptr
	};

	deviceContext.OMSetRenderTargets(
		1,
		nullRenderTarget,
		nullptr
	);

	// Seleccionar el viewport como destino final.
	deviceContext.OMSetRenderTargets(
		1,
		&outputRTV,
		nullptr
	);

	// Conectar el resultado del tonemapping.
	ID3D11ShaderResourceView* ldrResource =
		m_ldrSRV.m_textureFromImg;

	deviceContext.PSSetShaderResources(
		0,
		1,
		&ldrResource
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

	m_fxaaShader.render(
		deviceContext
	);

	m_fxaaData.texelSizeX =
		1.0f /
		static_cast<float>(m_width);

	m_fxaaData.texelSizeY =
		1.0f /
		static_cast<float>(m_height);

	m_fxaaBuffer.update(
		deviceContext,
		nullptr,
		0,
		nullptr,
		&m_fxaaData,
		0,
		0
	);

	m_fxaaBuffer.render(
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

	// Desconectar la textura LDR.
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

//============================================================
// CONTROLES DE FXAA
//============================================================

void
PostProcessSystem::setFxaaEnabled(
	bool enabled)
{
	m_fxaaData.enabled =
		enabled ? 1.0f : 0.0f;
}

bool
PostProcessSystem::isFxaaEnabled() const
{
	return
		m_fxaaData.enabled >
		0.5f;
}

//============================================================
// CONTROLES DE SSAO
//============================================================

void
PostProcessSystem::setSsaoEnabled(
	bool enabled)
{
	m_ssaoData.enabled =
		enabled ? 1.0f : 0.0f;
}

bool
PostProcessSystem::isSsaoEnabled() const
{
	return
		m_ssaoData.enabled >
		0.5f;
}

void
PostProcessSystem::setSsaoRadius(
	float radius)
{
	if (radius < 0.05f)
	{
		radius = 0.05f;
	}

	if (radius > 10.0f)
	{
		radius = 10.0f;
	}

	m_ssaoData.radius =
		radius;
}

float
PostProcessSystem::getSsaoRadius() const
{
	return m_ssaoData.radius;
}

void
PostProcessSystem::setSsaoBias(
	float bias)
{
	if (bias < 0.0f)
	{
		bias = 0.0f;
	}

	if (bias > 0.50f)
	{
		bias = 0.50f;
	}

	m_ssaoData.bias =
		bias;
}

float
PostProcessSystem::getSsaoBias() const
{
	return m_ssaoData.bias;
}

void
PostProcessSystem::setSsaoIntensity(
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

	m_ssaoData.intensity =
		intensity;
}

float
PostProcessSystem::getSsaoIntensity() const
{
	return m_ssaoData.intensity;
}

void
PostProcessSystem::setSsaoPower(
	float power)
{
	if (power < 0.10f)
	{
		power = 0.10f;
	}

	if (power > 5.0f)
	{
		power = 5.0f;
	}

	m_ssaoData.power =
		power;
}

float
PostProcessSystem::getSsaoPower() const
{
	return m_ssaoData.power;
}