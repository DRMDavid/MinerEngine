#pragma once

#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "ShaderProgram.h"

class Device;
class DeviceContext;
class Buffer;
class RasterizerState;
class DepthStencilState;


struct BloomData
{
	float threshold = 0.80f;
	float intensity = 0.80f;
	float texelSizeX = 0.0f;
	float texelSizeY = 0.0f;

	float enabled = 0.0f;
	float padding0 = 0.0f;
	float padding1 = 0.0f;
	float padding2 = 0.0f;
};

struct BloomBlurData
{
	float texelSizeX = 0.0f;
	float texelSizeY = 0.0f;
	float directionX = 1.0f;
	float directionY = 0.0f;
};

/**
 * @struct TonemappingData
 * @brief Datos enviados al shader de Tonemapping.
 */
struct TonemappingData
{
	float exposure = 1.0f;

	// El deferred actual ya entrega el color corregido.
	// Usar 1.0 evita aplicar gamma dos veces.
	float gamma = 1.0f;

	// Temporalmente desactivado para comprobar
	// que el HDR conserva los colores originales.
	float enableTonemapping = 0.0f;

	float padding = 0.0f;
};
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

	void setTonemappingEnabled(bool enabled);
	bool isTonemappingEnabled() const;

	void setExposure(float exposure);
	float getExposure() const;

	void setGamma(float gamma);
	float getGamma() const;

	void setBloomEnabled(bool enabled);
	bool isBloomEnabled() const;

	void setBloomThreshold(float threshold);
	float getBloomThreshold() const;

	void setBloomIntensity(float intensity);
	float getBloomIntensity() const;

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

	void extractBloom(
		DeviceContext& deviceContext,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
	);

	void blurBloom(
		DeviceContext& deviceContext,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
	);

	void renderTonemapping(
		DeviceContext& deviceContext,
		ID3D11RenderTargetView* outputRTV,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
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

	ShaderProgram m_tonemappingShader;
	ShaderProgram m_bloomExtractShader;
	SamplerState m_linearSampler;
	ShaderProgram m_bloomBlurShader;
	Buffer m_tonemappingBuffer;
	Buffer m_bloomBuffer;
	Buffer m_bloomBlurBuffer;

	TonemappingData m_tonemappingData{};
	BloomData m_bloomData{};
	BloomBlurData m_bloomBlurData{};
	// Texturas de Bloom a media resolución.
    // Se utilizan alternadamente para el desenfoque.
	Texture m_bloomTextureA;
	Texture m_bloomSRVA;
	RenderTargetView m_bloomRTVA;

	Texture m_bloomTextureB;
	Texture m_bloomSRVB;
	RenderTargetView m_bloomRTVB;
};