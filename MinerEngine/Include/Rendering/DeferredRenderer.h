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
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"
#include "SamplerState.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "Rendering/ISceneRenderer.h"

class Device;
class DeviceContext;
class Camera;
class Material;

/**
 * @class DeferredRenderer
 * @brief Implementa un pipeline de renderizado diferido (Deferred Shading) con GBuffer y pase de iluminación.
 *
 * El renderer usa deferred shading para superficies opacas y mantiene un subpass
 * forward para transparencias, de modo que el pipeline del editor siga funcionando
 * con el contenido actual del engine.
 */
class DeferredRenderer : public ISceneRenderer {
public:
    /**
     * @brief Inicializa los recursos base del renderer (Shaders globales, estados de mezcla, buffers, etc.).
     * @param device Referencia al dispositivo gráfico de Direct3D11.
     * @return HRESULT S_OK si la inicialización fue exitosa, o un código de error en caso contrario.
     */
    HRESULT init(Device& device) override;

    /**
     * @brief Redimensiona los pases y texturas del G-Buffer cuando cambia el tamaño de la ventana/viewport.
     * @param device Referencia al dispositivo gráfico.
     * @param width Nuevo ancho en píxeles.
     * @param height Nuevo alto en píxeles.
     */
    void resize(Device& device, unsigned int width, unsigned int height) override;

    /**
     * @brief Ejecuta el ciclo completo de renderizado de la escena.
     * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
     * @param camera Cámara desde la cual se renderiza la escena.
     * @param scene Escena que contiene los objetos y luces a procesar.
     * @param viewportPass Información del viewport y target final del editor.
     */
    void render(DeviceContext& deviceContext,
        const Camera& camera,
        RenderScene& scene,
        EditorViewportPass& viewportPass) override;

    /**
     * @brief Libera todos los recursos gráficos y de memoria asignados por el renderer.
     */
    void destroy() override;

    /** @name Getters de Recursos de Sombreado e Iluminación */
    ///@{
    /** @brief Obtiene la vista de recurso de shader (SRV) del mapa de sombras. */
    ID3D11ShaderResourceView* getShadowMapSRV() const override { return m_shadowDepthSRV.m_textureFromImg; }

    /** @brief Obtiene el SRV del pase de depuración previo a las sombras. */
    ID3D11ShaderResourceView* getPreShadowSRV() const override { return m_preShadowDebugPass.getSRV(); }

    /** @brief Obtiene el SRV del G-Buffer que almacena Albedo (RGB) y Metallic (A). */
    ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const override { return m_gBufferAlbedoMetallicSRV.m_textureFromImg; }

    /** @brief Obtiene el SRV del G-Buffer que almacena Normales (RGB) y Roughness (A). */
    ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const override { return m_gBufferNormalRoughnessSRV.m_textureFromImg; }

    /** @brief Obtiene el SRV del G-Buffer que almacena World Position/Vectors o Ambient Occlusion. */
    ID3D11ShaderResourceView* getGBufferWorldAoSRV() const override { return m_gBufferWorldAoSRV.m_textureFromImg; }

    /** @brief Obtiene el SRV del G-Buffer que almacena Emissive (RGB) y Alpha (A). */
    ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const override { return m_gBufferEmissiveAlphaSRV.m_textureFromImg; }
    ///@}

    /** @name Configuración de Depuración Visual */
    ///@{
    /** @brief Activa o desactiva la visualización del factor de sombras en el render. */
    void setShadowFactorDebugEnabled(bool enabled) override { m_shadowFactorDebugEnabled = enabled; }

    /** @brief Cambia el modo de vista de depuración para inspeccionar targets individuales del G-Buffer. */
    void setDeferredDebugViewMode(int mode) override { m_deferredDebugViewMode = mode; }
    ///@}

    /**
     * @brief Obtiene el nombre del renderer para logs y telemetría.
     * @return Cadena de caracteres estática con el nombre de la clase.
     */
    const char* getDebugName() const override { return "DeferredRenderer"; }

private:
    /**
     * @brief Clasifica y organiza los objetos de la escena en colas de renderizado (Opacos y Transparentes).
     */
    void buildQueues(RenderScene& scene, const Camera& camera);

    /**
     * @brief Actualiza los Constant Buffers globales por frame (Matrices de vista, proyección, etc.).
     */
    void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);

    /**
     * @brief Calcula y actualiza las matrices de vista y proyección desde la perspectiva de la luz (Sombras).
     */
    void updateLightMatrices(const Camera& camera, const RenderScene& scene);

    /**
     * @brief Coordina la lógica interna de renderizado enviando la geometría al target especificado.
     */
    void renderSceneToTarget(DeviceContext& deviceContext, RenderScene& scene, EditorViewportPass& targetPass, bool applyShadows);

    /**
     * @brief Vincula todos los Render Target Views (RTVs) del G-Buffer al pipeline de Direct3D.
     */
    void bindGBufferTargets(DeviceContext& deviceContext, ID3D11DepthStencilView* depthStencilView);

    /**
     * @brief Vincula el Render Target final (normalmente el viewport de salida) y el buffer de profundidad.
     */
    void bindFinalTarget(DeviceContext& deviceContext, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView);

    /**
     * @brief Desvincula los SRVs del G-Buffer de los slots del shader para evitar conflictos de lectura/escritura.
     */
    void clearDeferredSRVs(DeviceContext& deviceContext);

    /**
     * @brief Ejecuta el pase de geometría: Dibuja los objetos opacos llenando los targets del G-Buffer.
     */
    void renderGeometryPass(DeviceContext& deviceContext);

    /**
     * @brief Renderiza un objeto individual durante el pase de geometría (G-Buffer).
     */
    void renderGeometryObject(DeviceContext& deviceContext, const RenderObject& object);

    /**
     * @brief Ejecuta el pase de iluminación diferida (calcula luces usando la información analizada en el G-Buffer).
     */
    void renderLightingPass(DeviceContext& deviceContext);

    /**
     * @brief Renderiza el Skybox/Fondo de la escena.
     */
    void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);

    /**
     * @brief Ejecuta el pase Forward para objetos que poseen transparencias.
     */
    void renderTransparentPass(DeviceContext& deviceContext);

    /**
     * @brief Renderiza un objeto individual en modo Forward (como transparencias).
     */
    void renderForwardObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);

    /**
     * @brief Genera el mapa de profundidad desde la perspectiva de las luces (Shadow Mapping).
     */
    void renderShadowPass(DeviceContext& deviceContext);

    /**
     * @brief Dibuja un objeto desde la perspectiva de la luz para la generación de sombras.
     */
    void renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);

    /** @name Métodos de Creación de Recursos */
    ///@{
    HRESULT createShadowResources(Device& device);
    HRESULT createGBufferResources(Device& device, unsigned int width, unsigned int height);

    /**
     * @brief Helper genérico para inicializar las texturas y vistas de un componente del G-Buffer.
     */
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
    ///@}

    /**
     * @brief Evalúa el material del objeto y retorna el estado de mezcla (Blend State) correspondiente.
     */
    ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
    /** @name Constant Buffers gráficos */
    ///@{
    Buffer m_perFrameBuffer;          ///< Datos que cambian una vez por frame (Cámara, ambiente).
    Buffer m_perObjectBuffer;         ///< Datos de transformación del objeto (Matriz World).
    Buffer m_perMaterialBuffer;       ///< Propiedades específicas del material actual.
    Buffer m_lightingDebugBuffer;     ///< Parámetros enviados a los shaders para modos de depuración visual.
    Buffer m_fullscreenVertexBuffer;  ///< Vértices del plano que cubre la pantalla para el cálculo diferido.
    Buffer m_fullscreenIndexBuffer;   ///< Índices del plano para el pase de iluminación.
    ///@}

    /** @name Estados del Pipeline */
    ///@{
    DepthStencilState m_transparentDepthStencil; ///< Configuración de profundidad para pases translúcidos (Read-only depth).
    DepthStencilState m_disabledDepthStencil;    ///< Desactiva las pruebas y escrituras de profundidad.
    DepthStencilState m_shadowDepthStencil;      ///< Configuración optimizada para pasadas de sombras.

    ID3D11BlendState* m_alphaBlendState = nullptr;
    ID3D11BlendState* m_opaqueBlendState = nullptr;
    ID3D11BlendState* m_additiveBlendState = nullptr;
    ID3D11BlendState* m_premultipliedBlendState = nullptr;
    float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ///@}

    /** @name Recursos de Sombras */
    ///@{
    Texture m_shadowDepthTexture;
    Texture m_shadowDepthSRV;
    DepthStencilView m_shadowDSV;
    ShaderProgram m_shadowShader;
    RasterizerState m_shadowRasterizer;
    unsigned int m_shadowMapSize = 2048;
    ///@}

    /** @name Shaders y Estados de Iluminación */
    ///@{
    ShaderProgram m_gBufferShader;
    ShaderProgram m_deferredLightingShader;
    SamplerState m_lightingSampler;
    RasterizerState m_fullscreenRasterizer;
    ///@}

    /** @name Elementos del G-Buffer (Render Targets y Texturas) */
    ///@{
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
    ///@}

    /** @name Estado Interno y Dimensiones */
    ///@{
    EditorViewportPass m_preShadowDebugPass;
    bool m_applyShadows = true;
    unsigned int m_renderWidth = 1280;
    unsigned int m_renderHeight = 720;
    ///@}

    /** @name Estructuras de Datos de Memoria Intermedia (CPU-side) */
    ///@{
    CBPerFrame m_cbPerFrame{};
    CBPerObject m_cbPerObject{};
    CBPerMaterial m_cbPerMaterial{};

    struct DeferredLightingDebugData {
        int DebugViewMode = 0;      ///< Modo activo de visualización del G-Buffer.
        float ShadowStrength = 1.0f;///< Multiplicador de la intensidad de las sombras.
        float pad0 = 0.0f;          ///< Padding para alineación de 16 bytes exigida por DirectX.
        float pad1 = 0.0f;          ///< Padding para alineación de 16 bytes exigida por DirectX.
    } m_lightingDebugData{};

    bool m_shadowFactorDebugEnabled = false;
    int m_deferredDebugViewMode = 0;
    ///@}

    /** @name Colas de Renderizado Filtradas */
    ///@{
    std::vector<const RenderObject*> m_opaqueQueue;      ///< Objetos recolectados para el renderizado diferido.
    std::vector<const RenderObject*> m_transparentQueue; ///< Objetos recolectados para el subpass Forward.
    ///@}
};