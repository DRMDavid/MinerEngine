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

/**
 * @struct SsaoData
 * @brief Estructura de datos alineada para el Constant Buffer de SSAO (Screen Space Ambient Occlusion).
 */
struct SsaoData
{
	/** @brief Radio de muestreo en el espacio de vista para la detección de oclusión. */
	float radius = 1.50f;
	/** @brief Sesgo (bias) para evitar el acné de sombra o autodetección falsa. */
	float bias = 0.02f;
	/** @brief Multiplicador de la intensidad general de la oclusión ambiental. */
	float intensity = 1.20f;
	/** @brief Factor de potencia para ajustar la curva de contraste de la oclusión. */
	float power = 1.50f;

	/** @brief Ancho del texel (1.0 / ancho_de_pantalla) para muestreo en shader. */
	float texelSizeX = 0.0f;
	/** @brief Alto del texel (1.0 / alto_de_pantalla) para muestreo en shader. */
	float texelSizeY = 0.0f;
	/** @brief Radio de muestreo expresado en píxeles. */
	float sampleRadiusPixels = 4.0f;
	/** @brief Flag de activación de SSAO (1.0f activo, 0.0f inactivo). */
	float enabled = 0.0f;
};

/**
 * @struct FxaaData
 * @brief Estructura de datos alineada para el Constant Buffer del filtro FXAA (Fast Approximate Anti-Aliasing).
 */
struct FxaaData
{
	/** @brief Ancho del texel (1.0 / ancho_de_pantalla). */
	float texelSizeX = 0.0f;
	/** @brief Alto del texel (1.0 / alto_de_pantalla). */
	float texelSizeY = 0.0f;
	/** @brief Flag de activación de FXAA (1.0f activo, 0.0f inactivo). */
	float enabled = 0.0f;
	/** @brief Relleno de alineación de 16 bytes para HLSL cbuffer. */
	float padding = 0.0f;
};

/**
 * @struct BloomData
 * @brief Estructura de datos alineada para el Constant Buffer del paso de extracción de Bloom.
 */
struct BloomData
{
	/** @brief Umbral de luminancia a partir del cual las zonas de la imagen brillan. */
	float threshold = 0.80f;
	/** @brief Intensidad del resplandor (bloom) extraído. */
	float intensity = 0.80f;
	/** @brief Ancho del texel. */
	float texelSizeX = 0.0f;
	/** @brief Alto del texel. */
	float texelSizeY = 0.0f;

	/** @brief Flag de activación del efecto Bloom (1.0f activo, 0.0f inactivo). */
	float enabled = 0.0f;
	/** @brief Relleno de alineación HLSL (byte 1/3). */
	float padding0 = 0.0f;
	/** @brief Relleno de alineación HLSL (byte 2/3). */
	float padding1 = 0.0f;
	/** @brief Relleno de alineación HLSL (byte 3/3). */
	float padding2 = 0.0f;
};

/**
 * @struct BloomBlurData
 * @brief Estructura de datos alineada para el Constant Buffer del desenfoque separables (Blur) de Bloom.
 */
struct BloomBlurData
{
	/** @brief Ancho del texel. */
	float texelSizeX = 0.0f;
	/** @brief Alto del texel. */
	float texelSizeY = 0.0f;
	/** @brief Dirección X del paso de desenfoque (p. ej., 1.0f horizontal, 0.0f vertical). */
	float directionX = 1.0f;
	/** @brief Dirección Y del paso de desenfoque (p. ej., 0.0f horizontal, 1.0f vertical). */
	float directionY = 0.0f;
};

/**
 * @struct TonemappingData
 * @brief Datos enviados al shader de Tonemapping.
 */
struct TonemappingData
{
	/** @brief Exposición de la cámara/escena para ajustar la escala de luminancia HDR. */
	float exposure = 1.0f;

	// El deferred actual ya entrega el color corregido.
	// Usar 1.0 evita aplicar gamma dos veces.
	/** @brief Valor del exponente de corrección gamma. */
	float gamma = 1.0f;

	// Temporalmente desactivado para comprobar
	// que el HDR conserva los colores originales.
	/** @brief Flag para habilitar o deshabilitar el mapa de tonos. */
	float enableTonemapping = 0.0f;

	/** @brief Relleno de alineación de 16 bytes para HLSL. */
	float padding = 0.0f;
};

/**
 * @class PostProcessSystem
 * @brief Administra los recursos utilizados por el postproceso.
 * @details Gestiona la cadena de efectos post-procesamiento (HDR, Bloom, Tonemapping, FXAA y SSAO),
 * asignando y ejecutando los pases de renderizado mediante sus respectivos buffers, pases de render y shaders.
 */
class PostProcessSystem
{
public:

	/** @brief Constructor por defecto. */
	PostProcessSystem() = default;

	/** @brief Destructor que garantiza la liberación de recursos. */
	~PostProcessSystem();

	/** @brief Constructor de copia eliminado para prevenir la duplicación de recursos de GPU. */
	PostProcessSystem(
		const PostProcessSystem&) = delete;

	/** @brief Operador de asignación por copia eliminado. */
	PostProcessSystem& operator=(
		const PostProcessSystem&) = delete;

	/**
	 * @brief Habilita o deshabilita la etapa de Tonemapping.
	 * @param enabled True para activar, false para desactivar.
	 */
	void setTonemappingEnabled(bool enabled);

	/**
	 * @brief Obtiene el estado de activación de Tonemapping.
	 * @return True si Tonemapping está habilitado, false en caso contrario.
	 */
	bool isTonemappingEnabled() const;

	/**
	 * @brief Establece el nivel de exposición para el tonemapping.
	 * @param exposure Valor de exposición.
	 */
	void setExposure(float exposure);

	/**
	 * @brief Obtiene el valor actual de exposición.
	 * @return Flotante con la exposición actual.
	 */
	float getExposure() const;

	/**
	 * @brief Establece el factor de corrección gamma.
	 * @param gamma Valor exponente gamma.
	 */
	void setGamma(float gamma);

	/**
	 * @brief Obtiene el valor actual de gamma.
	 * @return Valor de gamma.
	 */
	float getGamma() const;

	/**
	 * @brief Habilita o deshabilita la etapa de Bloom.
	 * @param enabled True para activar, false para desactivar.
	 */
	void setBloomEnabled(bool enabled);

	/**
	 * @brief Obtiene el estado de activación del Bloom.
	 * @return True si Bloom está activado, false en caso contrario.
	 */
	bool isBloomEnabled() const;

	/**
	 * @brief Define el umbral de luminancia para la extracción del Bloom.
	 * @param threshold Umbral de corte de brillo.
	 */
	void setBloomThreshold(float threshold);

	/**
	 * @brief Obtiene el umbral de luminancia actual de Bloom.
	 * @return Umbral de Bloom.
	 */
	float getBloomThreshold() const;

	/**
	 * @brief Establece la intensidad del brillo (Bloom).
	 * @param intensity Intensidad del efecto.
	 */
	void setBloomIntensity(float intensity);

	/**
	 * @brief Obtiene la intensidad de Bloom actual.
	 * @return Intensidad de Bloom.
	 */
	float getBloomIntensity() const;

	/**
	 * @brief Habilita o deshabilita el suavizado FXAA.
	 * @param enabled True para activar FXAA, false para desactivar.
	 */
	void setFxaaEnabled(bool enabled);

	/**
	 * @brief Consulta si FXAA está activo.
	 * @return True si está activo, false si no.
	 */
	bool isFxaaEnabled() const;

	/**
	 * @brief Habilita o deshabilita la Oclusión Ambiental en Espacio de Pantalla (SSAO).
	 * @param enabled True para activar SSAO, false para desactivar.
	 */
	void setSsaoEnabled(bool enabled);

	/**
	 * @brief Consulta si el efecto SSAO está activo.
	 * @return True si está activo, false si no.
	 */
	bool isSsaoEnabled() const;

	/**
	 * @brief Establece el radio del hemisferio de muestreo SSAO.
	 * @param radius Radio en unidades de espacio de vista.
	 */
	void setSsaoRadius(float radius);

	/**
	 * @brief Obtiene el radio de muestreo actual de SSAO.
	 * @return Radio del muestreo SSAO.
	 */
	float getSsaoRadius() const;

	/**
	 * @brief Establece el sesgo (bias) de SSAO.
	 * @param bias Valor del sesgo anti-acné.
	 */
	void setSsaoBias(float bias);

	/**
	 * @brief Obtiene el sesgo de SSAO.
	 * @return Valor actual del sesgo.
	 */
	float getSsaoBias() const;

	/**
	 * @brief Establece la intensidad del sombreado de SSAO.
	 * @param intensity Multiplicador de sombreado.
	 */
	void setSsaoIntensity(float intensity);

	/**
	 * @brief Obtiene la intensidad del efecto SSAO.
	 * @return Intensidad de SSAO.
	 */
	float getSsaoIntensity() const;

	/**
	 * @brief Establece el exponente de potencia del contraste de SSAO.
	 * @param power Exponente de potencia.
	 */
	void setSsaoPower(float power);

	/**
	 * @brief Obtiene la potencia del contraste de SSAO.
	 * @return Valor de la potencia de SSAO.
	 */
	float getSsaoPower() const;

	/**
	 * @brief Inicializa el render target HDR.
	 * @param device Referencia al dispositivo gráfico (DirectX Device).
	 * @param width Ancho inicial del viewport/pantalla.
	 * @param height Alto inicial del viewport/pantalla.
	 * @return HRESULT representando el resultado de la creación de recursos.
	 */
	HRESULT init(
		Device& device,
		unsigned int width,
		unsigned int height
	);

	/**
	 * @brief Recrea los recursos cuando cambia el viewport.
	 * @param device Referencia al dispositivo gráfico.
	 * @param width Nuevo ancho de pantalla.
	 * @param height Nuevo alto de pantalla.
	 * @return HRESULT indicando si la reconfiguración fue exitosa.
	 */
	HRESULT resize(
		Device& device,
		unsigned int width,
		unsigned int height
	);

	/**
	 * @brief Ejecuta el pase para extraer los píxeles brillantes de la escena HDR.
	 * @param deviceContext Contexto de ejecución del dispositivo gráfico.
	 * @param fullscreenVertexBuffer Buffer con la geometría del cuadriculado a pantalla completa.
	 * @param fullscreenIndexBuffer Buffer de índices para la pantalla completa.
	 * @param fullscreenRasterizer Estado de rasterizado sin cull mode.
	 * @param disabledDepthStencil Estado de Depth Stencil desactivado.
	 */
	void extractBloom(
		DeviceContext& deviceContext,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
	);

	/**
	 * @brief Aplica el desenfoque Gaussiano/separado a las texturas de Bloom.
	 * @param deviceContext Contexto del dispositivo gráfico.
	 * @param fullscreenVertexBuffer Buffer de vértices de la pantalla completa.
	 * @param fullscreenIndexBuffer Buffer de índices de la pantalla completa.
	 * @param fullscreenRasterizer Estado del rasterizador.
	 * @param disabledDepthStencil Estado de profundidad desactivado.
	 */
	void blurBloom(
		DeviceContext& deviceContext,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
	);

	/**
	 * @brief Renderiza el pase de Tonemapping convirtiendo HDR a LDR.
	 * @param deviceContext Contexto del dispositivo gráfico.
	 * @param outputRTV RenderTargetView de salida donde se dibujará el resultado.
	 * @param fullscreenVertexBuffer Buffer de vértices.
	 * @param fullscreenIndexBuffer Buffer de índices.
	 * @param fullscreenRasterizer Estado del rasterizador.
	 * @param disabledDepthStencil Estado de profundidad desactivado.
	 */
	void renderTonemapping(
		DeviceContext& deviceContext,
		ID3D11RenderTargetView* outputRTV,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
	);

	/**
	 * @brief Ejecuta el pase de anti-aliasing FXAA sobre la textura LDR resultante.
	 * @param deviceContext Contexto del dispositivo gráfico.
	 * @param outputRTV RenderTargetView destino final (por ejemplo, el back buffer).
	 * @param fullscreenVertexBuffer Buffer de vértices.
	 * @param fullscreenIndexBuffer Buffer de índices.
	 * @param fullscreenRasterizer Estado del rasterizador.
	 * @param disabledDepthStencil Estado de profundidad desactivado.
	 */
	void renderFxaa(
		DeviceContext& deviceContext,
		ID3D11RenderTargetView* outputRTV,
		Buffer& fullscreenVertexBuffer,
		Buffer& fullscreenIndexBuffer,
		RasterizerState& fullscreenRasterizer,
		DepthStencilState& disabledDepthStencil
	);

	/**
	 * @brief Calcula el pase de Oclusión Ambiental en Espacio de Pantalla (SSAO).
	 * @param deviceContext Contexto del dispositivo gráfico.
	 * @param worldPositionSRV Shader Resource View del G-Buffer con las posiciones globales.
	 * @param normalRoughnessSRV Shader Resource View del G-Buffer con las normales de la escena.
	 * @param fullscreenVertexBuffer Buffer de vértices.
	 * @param fullscreenIndexBuffer Buffer de índices.
	 * @param fullscreenRasterizer Estado del rasterizador.
	 * @param disabledDepthStencil Estado de profundidad desactivado.
	 */
	void renderSsao(
		DeviceContext& deviceContext,
		ID3D11ShaderResourceView* worldPositionSRV,
		ID3D11ShaderResourceView* normalRoughnessSRV,
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
	 * @return Puntero al ID3D11RenderTargetView de la textura HDR principal.
	 */
	ID3D11RenderTargetView*
		getHdrRTV() const;

	/**
	 * @brief Obtiene la vista para leer la textura HDR.
	 * @return Puntero al ID3D11ShaderResourceView de la textura HDR.
	 */
	ID3D11ShaderResourceView*
		getHdrSRV() const;

	/**
	 * @brief Obtiene el ancho actual de las texturas de postprocesamiento.
	 * @return Ancho en píxeles.
	 */
	unsigned int getWidth() const
	{
		return m_width;
	}

	/**
	 * @brief Obtiene el alto actual de las texturas de postprocesamiento.
	 * @return Alto en píxeles.
	 */
	unsigned int getHeight() const
	{
		return m_height;
	}

	/**
	 * @brief Comprueba si todos los recursos gráficos críticos han sido creados e inicializados correctamente.
	 * @return True si todas las texturas y vistas principales son válidas, false en caso contrario.
	 */
	bool isReady() const
	{
		return
			m_hdrTexture.m_texture != nullptr &&
			m_hdrSRV.m_textureFromImg != nullptr &&
			m_hdrRTV.get() != nullptr &&

			m_ldrTexture.m_texture != nullptr &&
			m_ldrSRV.m_textureFromImg != nullptr &&
			m_ldrRTV.get() != nullptr &&

			m_ssaoTexture.m_texture != nullptr &&
			m_ssaoSRV.m_textureFromImg != nullptr &&
			m_ssaoRTV.get() != nullptr;
	}

private:

	/**
	 * @brief Asigna e inicializa internamente las texturas HDR, LDR, SSAO y Render Targets requeridos.
	 * @param device Dispositivo DirectX para instanciar los recursos.
	 * @param width Ancho objetivo.
	 * @param height Alto objetivo.
	 * @return HRESULT indicando éxito o fracaso en la creación.
	 */
	HRESULT createHdrResources(
		Device& device,
		unsigned int width,
		unsigned int height
	);

	/**
	 * @brief Libera los recursos internos asignados de Direct3D.
	 */
	void destroyHdrResources();

private:

	Texture m_hdrTexture; /**< Textura base HDR. */
	Texture m_hdrSRV;     /**< Vista de recurso de shader para la textura HDR. */
	RenderTargetView m_hdrRTV; /**< Render Target para escribir el pase HDR inicial. */

	unsigned int m_width = 0;  /**< Ancho actual del sistema de postprocesado. */
	unsigned int m_height = 0; /**< Alto actual del sistema de postprocesado. */

	ShaderProgram m_tonemappingShader;  /**< Shader para el passe de Tonemapping. */
	ShaderProgram m_bloomExtractShader; /**< Shader para extraer las altas luces de Bloom. */
	SamplerState m_linearSampler;      /**< Estado del muestreador lineal utilizado en pases de pantalla. */
	ShaderProgram m_bloomBlurShader;    /**< Shader para el desenfoque de Bloom. */
	ShaderProgram m_fxaaShader;         /**< Shader de suavizado FXAA. */
	ShaderProgram m_ssaoShader;         /**< Shader para calcular la oclusión ambiental. */
	Buffer m_tonemappingBuffer;         /**< Constant buffer de parámetros de Tonemapping. */
	Buffer m_bloomBuffer;               /**< Constant buffer de parámetros de extracción de Bloom. */
	Buffer m_bloomBlurBuffer;           /**< Constant buffer de parámetros de desenfoque de Bloom. */
	Buffer m_fxaaBuffer;                /**< Constant buffer para FXAA. */
	Buffer m_ssaoBuffer;                /**< Constant buffer para SSAO. */

	TonemappingData m_tonemappingData{}; /**< Datos de configuración en CPU para Tonemapping. */
	BloomData m_bloomData{};             /**< Datos de configuración en CPU para extracción de Bloom. */
	BloomBlurData m_bloomBlurData{};     /**< Datos de configuración en CPU para desenfoque de Bloom. */
	FxaaData m_fxaaData{};               /**< Datos de configuración en CPU para FXAA. */
	SsaoData m_ssaoData{};               /**< Datos de configuración en CPU para SSAO. */

	// Texturas de Bloom a media resolución.
	// Se utilizan alternadamente para el desenfoque.
	Texture m_bloomTextureA;  /**< Textura 'A' de ping-pong para el procesamiento de Bloom. */
	Texture m_bloomSRVA;      /**< SRV de la textura A de Bloom. */
	RenderTargetView m_bloomRTVA; /**< Render Target View A de Bloom. */

	Texture m_bloomTextureB;  /**< Textura 'B' de ping-pong para el procesamiento de Bloom. */
	Texture m_bloomSRVB;      /**< SRV de la textura B de Bloom. */
	RenderTargetView m_bloomRTVB; /**< Render Target View B de Bloom. */

	// Resultado LDR después de Tonemapping.
	// FXAA leerá esta textura antes de escribir al viewport.
	Texture m_ldrTexture;      /**< Textura LDR resultado del Tonemapping. */
	Texture m_ldrSRV;          /**< SRV de la textura LDR. */
	RenderTargetView m_ldrRTV; /**< Render Target View de la textura LDR. */

	// Textura de oclusión ambiental en espacio de pantalla.
	Texture m_ssaoTexture;      /**< Textura que contiene el mapa de oclusión SSAO resultante. */
	Texture m_ssaoSRV;          /**< SRV de la textura de SSAO. */
	RenderTargetView m_ssaoRTV; /**< Render Target View para calcular SSAO. */
};