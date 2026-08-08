#include "Rendering/PostProcessSystem.h"

#include "Device.h"

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
	return resize(
		device,
		width,
		height
	);
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

void
PostProcessSystem::destroy()
{
	destroyHdrResources();

	m_width = 0;
	m_height = 0;
}

//============================================================
// GETTERS
//============================================================

ID3D11RenderTargetView*
PostProcessSystem::getHdrRTV() const
{
	return m_hdrRTV.get();
}

ID3D11ShaderResourceView*
PostProcessSystem::getHdrSRV() const
{
	return m_hdrSRV.m_textureFromImg;
}