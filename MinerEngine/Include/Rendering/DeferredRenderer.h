/**
 * @file DeferredRenderer.h
 * @brief Declara la API de DeferredRenderer dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Buffer.h"
#include "DepthStencilState.h"
#include "DepthStencilView.h"
#include "RasterizerState.h"
#include "Rendering/ISceneRenderer.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"
#include "SamplerState.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

class Device;
class DeviceContext;
class Camera;
class Material;

/**
 * @class DeferredRenderer
 * @brief Implementa un pipeline diferido con GBuffer y lighting pass.
 *
 * El renderer usa deferred shading para superficies opacas y mantiene un subpass
 * forward para transparencias, de modo que el pipeline del editor siga funcionando
 * con el contenido actual del engine.
 */
class DeferredRenderer : public ISceneRenderer {
public:
	/**
	 * @brief Inicializa los recursos del renderizador diferido.
	 * @param device Referencia al dispositivo gráfico para la creación de recursos.
	 * @return HRESULT Código de éxito (S_OK) o el código de error correspondiente.
	 */
	HRESULT init(Device& device) override;

	/**
	 * @brief Redimensiona los render targets y buffers para ajustarse a la nueva resolución.
	 * @param device Referencia al dispositivo gráfico.
	 * @param width Nuevo ancho en píxeles.
	 * @param height Nuevo alto en píxeles.
	 */
	void resize(Device& device, unsigned int width, unsigned int height) override;

	/**
	 * @brief Ejecuta el pipeline de renderizado completo para un frame.
	 * @param deviceContext Contexto del dispositivo para emitir los comandos de dibujado.
	 * @param camera Cámara desde la cual se renderizará la escena.
	 * @param scene Escena que contiene los objetos y luces a renderizar.
	 * @param viewportPass Render target final (usualmente el viewport del editor).
	 */
	void render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass) override;

	/**
	 * @brief Libera todos los recursos y memoria gráfica asociados al renderizador.
	 */
	void destroy() override;

	/**
	 * @brief Obtiene el Shader Resource View (SRV) del mapa de sombras.
	 * @return Puntero al SRV de profundidad de las sombras.
	 */
	ID3D11ShaderResourceView* getShadowMapSRV() const override { return m_shadowDepthSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el SRV de la pasada de depuración previa al sombreado.
	 * @return Puntero al SRV de debug.
	 */
	ID3D11ShaderResourceView* getPreShadowSRV() const override { return m_preShadowDebugPass.getSRV(); }

	/**
	 * @brief Obtiene el SRV correspondiente al Albedo y Metallic del GBuffer.
	 * @return Puntero al SRV (RGB: Albedo, A: Metallic).
	 */
	ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const override { return m_gBufferAlbedoMetallicSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el SRV correspondiente a las Normales y Roughness del GBuffer.
	 * @return Puntero al SRV (RGB: Normales, A: Roughness).
	 */
	ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const override { return m_gBufferNormalRoughnessSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el SRV correspondiente a la Posición en el Mundo y Ambient Occlusion del GBuffer.
	 * @return Puntero al SRV (RGB: World Position, A: Ambient Occlusion).
	 */
	ID3D11ShaderResourceView* getGBufferWorldAoSRV() const override { return m_gBufferWorldAoSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el SRV correspondiente a la Emisión y Alpha del GBuffer.
	 * @return Puntero al SRV (RGB: Emissive, A: Alpha).
	 */
	ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const override { return m_gBufferEmissiveAlphaSRV.m_textureFromImg; }

	/**
	 * @brief Habilita o deshabilita la vista de depuración del factor de sombra.
	 * @param enabled True para habilitar el debug de sombras, false para comportamiento normal.
	 */
	void setShadowFactorDebugEnabled(bool enabled) override { m_shadowFactorDebugEnabled = enabled; }

	/**
	 * @brief Establece el modo de vista de depuración del pipeline diferido.
	 * @param mode Entero que representa el modo de visualización (ej. 0: Normal, 1: Albedo, 2: Normales, etc.).
	 */
	void setDeferredDebugViewMode(int mode) override { m_deferredDebugViewMode = mode; }

	/**
	 * @brief Obtiene el nombre del renderizador para propósitos de depuración o profiling.
	 * @return Cadena de texto con el nombre "DeferredRenderer".
	 */
	const char* getDebugName() const override { return "DeferredRenderer"; }

private:
	// =========================================================================
	// Métodos de Pipeline Interno
	// =========================================================================

	/** @brief Separa los objetos de la escena en colas de opacos y transparentes. */
	void buildQueues(RenderScene& scene, const Camera& camera);
	/** @brief Actualiza los constant buffers de ámbito de frame (cámara, tiempo, etc.). */
	void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);
	/** @brief Calcula y actualiza las matrices de proyección y vista de las luces. */
	void updateLightMatrices(const Camera& camera, const RenderScene& scene);
	/** @brief Orquesta el renderizado completo hacia el target principal. */
	void renderSceneToTarget(DeviceContext& deviceContext, RenderScene& scene, EditorViewportPass& targetPass, bool applyShadows);
	/** @brief Enlaza los Render Targets del GBuffer al pipeline. */
	void bindGBufferTargets(DeviceContext& deviceContext, ID3D11DepthStencilView* depthStencilView);
	/** @brief Enlaza el Render Target final donde se aplicará la iluminación. */
	void bindFinalTarget(DeviceContext& deviceContext, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView);
	/** @brief Limpia los recursos (SRVs) del GBuffer para el nuevo frame. */
	void clearDeferredSRVs(DeviceContext& deviceContext);

	/** @brief Ejecuta la pasada de geometría llenando el GBuffer. */
	void renderGeometryPass(DeviceContext& deviceContext);
	/** @brief Renderiza un objeto individual durante la pasada de geometría. */
	void renderGeometryObject(DeviceContext& deviceContext, const RenderObject& object);

	/** @brief Ejecuta la pasada de iluminación usando los datos del GBuffer. */
	void renderLightingPass(DeviceContext& deviceContext);
	/** @brief Dibuja el skybox de la escena. */
	void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);
	/** @brief Ejecuta el subpass forward para la cola de objetos transparentes. */
	void renderTransparentPass(DeviceContext& deviceContext);
	/** @brief Renderiza un objeto mediante shading forward tradicional. */
	void renderForwardObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);

	/** @brief Ejecuta la pasada de generación de mapas de sombras. */
	void renderShadowPass(DeviceContext& deviceContext);
	/** @brief Dibuja la profundidad de un objeto en el mapa de sombras. */
	void renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);

	// =========================================================================
	// Métodos de Inicialización de Recursos
	// =========================================================================

	HRESULT createShadowResources(Device& device);
	HRESULT createGBufferResources(Device& device, unsigned int width, unsigned int height);
	HRESULT createGBufferTarget(Device& device,
		unsigned int width,
		unsigned int height,
		DXGI_FORMAT format,
		Texture& texture,
		Texture& srv,
		RenderTargetView& rtv);
	HRESULT createLightingResources(Device& device);
	HRESULT createFullScreenQuad(Device& device);
	HRESULT createBlendStates(Device& device);

	/** @brief Retorna el estado de blending adecuado según las propiedades del material. */
	ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
	// =========================================================================
	// Recursos Gráficos
	// =========================================================================

	Buffer m_perFrameBuffer;          ///< Constant buffer para datos por frame.
	Buffer m_perObjectBuffer;         ///< Constant buffer para transformaciones de objeto.
	Buffer m_perMaterialBuffer;       ///< Constant buffer para propiedades de material.
	Buffer m_lightingDebugBuffer;     ///< Constant buffer para opciones de debug en iluminación.
	Buffer m_fullscreenVertexBuffer;  ///< Vertex buffer para el quad de pantalla completa.
	Buffer m_fullscreenIndexBuffer;   ///< Index buffer para el quad de pantalla completa.

	DepthStencilState m_transparentDepthStencil; ///< Estado de profundidad para transparentes.
	DepthStencilState m_disabledDepthStencil;    ///< Estado con prueba de profundidad desactivada.
	DepthStencilState m_shadowDepthStencil;      ///< Estado para la escritura en el mapa de sombras.

	ID3D11BlendState* m_alphaBlendState = nullptr;
	ID3D11BlendState* m_opaqueBlendState = nullptr;
	ID3D11BlendState* m_additiveBlendState = nullptr;
	ID3D11BlendState* m_premultipliedBlendState = nullptr;
	float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// =========================================================================
	// Recursos de Sombras
	// =========================================================================

	Texture m_shadowDepthTexture;
	Texture m_shadowDepthSRV;
	DepthStencilView m_shadowDSV;
	ShaderProgram m_shadowShader;
	RasterizerState m_shadowRasterizer;
	unsigned int m_shadowMapSize = 2048; ///< Resolución del shadow map.

	// =========================================================================
	// Recursos de GBuffer e Iluminación
	// =========================================================================

	ShaderProgram m_gBufferShader;
	ShaderProgram m_deferredLightingShader;
	SamplerState m_lightingSampler;
	RasterizerState m_fullscreenRasterizer;

	Texture m_gBufferAlbedoMetallicTexture;
	Texture m_gBufferAlbedoMetallicSRV;
	RenderTargetView m_gBufferAlbedoMetallicRTV;

	Texture m_gBufferNormalRoughnessTexture;
	Texture m_gBufferNormalRoughnessSRV;
	RenderTargetView m_gBufferNormalRoughnessRTV;

	Texture m_gBufferWorldAoTexture;
	Texture m_gBufferWorldAoSRV;
	RenderTargetView m_gBufferWorldAoRTV;

	Texture m_gBufferEmissiveAlphaTexture;
	Texture m_gBufferEmissiveAlphaSRV;
	RenderTargetView m_gBufferEmissiveAlphaRTV;

	// =========================================================================
	// Estado Interno y Colas
	// =========================================================================

	EditorViewportPass m_preShadowDebugPass;
	bool m_applyShadows = true;
	unsigned int m_renderWidth = 1280;
	unsigned int m_renderHeight = 720;

	CBPerFrame m_cbPerFrame{};
	CBPerObject m_cbPerObject{};
	CBPerMaterial m_cbPerMaterial{};

	/** @brief Estructura interna para enviar datos de debug al shader de iluminación. */
	struct DeferredLightingDebugData {
		int DebugViewMode = 0;
		float ShadowStrength = 1.0f;
		float pad0 = 0.0f;
		float pad1 = 0.0f;
	} m_lightingDebugData{};

	bool m_shadowFactorDebugEnabled = false;
	int m_deferredDebugViewMode = 0;

	std::vector<const RenderObject*> m_opaqueQueue;      ///< Objetos a procesar en el GBuffer.
	std::vector<const RenderObject*> m_transparentQueue; ///< Objetos a procesar en la pasada forward.
};