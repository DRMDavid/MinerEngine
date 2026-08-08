#pragma once

#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"

class Device;

/**
 * @class PostProcessSystem
 * @brief Administra los recursos utilizados por el postproceso.
 */
class PostProcessSystem
{
public:

	PostProcessSystem() = default;
	~PostProcessSystem();

	PostProcessSystem(
		const PostProcessSystem&) = delete;

	PostProcessSystem& operator=(
		const PostProcessSystem&) = delete;

	/**
	 * @brief Inicializa el render target HDR.
	 */
	HRESULT init(
		Device& device,
		unsigned int width,
		unsigned int height
	);

	/**
	 * @brief Recrea los recursos cuando cambia el viewport.
	 */
	HRESULT resize(
		Device& device,
		unsigned int width,
		unsigned int height
	);

	/**
	 * @brief Destruye todos los recursos.
	 */
	void destroy();

	/**
	 * @brief Obtiene el render target HDR.
	 */
	ID3D11RenderTargetView*
		getHdrRTV() const;

	/**
	 * @brief Obtiene la vista para leer la textura HDR.
	 */
	ID3D11ShaderResourceView*
		getHdrSRV() const;

	unsigned int getWidth() const
	{
		return m_width;
	}

	unsigned int getHeight() const
	{
		return m_height;
	}

	bool isReady() const
	{
		return
			m_hdrTexture.m_texture != nullptr &&
			m_hdrSRV.m_textureFromImg != nullptr &&
			m_hdrRTV.get() != nullptr;
	}

private:

	HRESULT createHdrResources(
		Device& device,
		unsigned int width,
		unsigned int height
	);

	void destroyHdrResources();

private:

	Texture m_hdrTexture;
	Texture m_hdrSRV;
	RenderTargetView m_hdrRTV;

	unsigned int m_width = 0;
	unsigned int m_height = 0;
};