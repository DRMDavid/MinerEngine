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
	// TEXTURA HDR
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
		ERROR(
			"PostProcessSystem",
			"createHdrResources",
			"No se pudo crear la textura HDR"
		);

		return result;
	}

	//========================================================
	// RENDER TARGET VIEW
	//========================================================

	result =
		m_hdrRTV.init(
			device,
			m_hdrTexture,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			hdrFormat
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"createHdrResources",
			"No se pudo crear el HDR RTV"
		);

		destroyHdrResources();
		return result;
	}

	//========================================================
	// SHADER RESOURCE VIEW
	//========================================================

	result =
		m_hdrSRV.init(
			device,
			m_hdrTexture,
			hdrFormat
		);

	if (FAILED(result))
	{
		ERROR(
			"PostProcessSystem",
			"createHdrResources",
			"No se pudo crear el HDR SRV"
		);

		destroyHdrResources();
		return result;
	}

	return S_OK;
}

//============================================================
// DESTRUIR RECURSOS HDR
//============================================================

void
PostProcessSystem::destroyHdrResources()
{
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
	// Primero destruir los recursos que dependen
	// de las texturas.
	destroyHdrResources();

	// Después destruir los recursos del shader.
	m_tonemappingBuffer.destroy();
	m_linearSampler.destroy();
	m_tonemappingShader.destroy();

	m_width = 0;
	m_height = 0;
}

//============================================================
// GET HDR RTV
//============================================================

ID3D11RenderTargetView*
PostProcessSystem::getHdrRTV() const
{
	return
		m_hdrRTV.get();
}

//============================================================
// GET HDR SRV
//============================================================

ID3D11ShaderResourceView*
PostProcessSystem::getHdrSRV() const
{
	return
		m_hdrSRV.m_textureFromImg;
}

//============================================================
// APLICAR TONEMAPPING
//============================================================

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
	ID3D11ShaderResourceView* hdrResource =
		getHdrSRV();

	deviceContext.PSSetShaderResources(
		0,
		1,
		&hdrResource
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