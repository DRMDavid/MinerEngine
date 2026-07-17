#pragma once
#include "Prerequisites.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

/**
 * @struct AssetThumb
 * @brief Estructura que representa la miniatura de un asset para la interfaz.
 */
struct AssetThumb {
    std::string name;                   ///< Nombre del asset.
    ID3D11ShaderResourceView* srv;      ///< Vista del recurso (textura) de la miniatura.
};

/**
 * @class GUI
 * @brief Maneja y renderiza toda la interfaz gráfica del usuario y del editor usando ImGui.
 */
class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    /**
     * @brief Preparación inicial de la GUI antes de la creación de contextos.
     */
    void awake();

    /**
     * @brief Inicializa los contextos de ImGui y los backends de Win32 y DirectX 11.
     * @param window Referencia a la ventana de la aplicación.
     * @param device Referencia al dispositivo DirectX.
     * @param deviceContext Referencia al contexto del dispositivo DirectX.
     */
    void init(Window& window, Device& device, DeviceContext& deviceContext);

    /**
     * @brief Actualiza la lógica de la interfaz por cada fotograma.
     * @param viewport Referencia al viewport principal.
     * @param window Referencia a la ventana de la aplicación.
     */
    void update(Viewport& viewport, Window& window);

    /**
     * @brief Emite los comandos de dibujado de ImGui.
     */
    void render();

    /**
     * @brief Apaga ImGui y libera sus recursos.
     */
    void destroy();

    /**
     * @brief Dibuja la barra de herramientas principal.
     */
    void ToolBar();

    /**
     * @brief Dibuja el modal o ventana de confirmación para cerrar la aplicación.
     */
    void closeApp();

    /**
     * @brief Muestra información adicional o tooltips en la interfaz.
     */
    void toolTipData();

    /**
     * @brief Aplica un tema visual personalizado (estilo Apple Liquid) a ImGui.
     * @param opacity Opacidad general de las ventanas.
     * @param accent Color de acento principal.
     */
    void appleLiquidStyle(float opacity, ImVec4 accent);

    /**
     * @brief Dibuja un control para manipular vectores de 3 dimensiones (XYZ).
     * @param label Etiqueta del control.
     * @param values Puntero al arreglo de 3 flotantes a modificar.
     * @param resetValues Valor al que se reinician las componentes al hacer clic en reset.
     * @param columnWidth Ancho de la columna de la etiqueta.
     */
    void vec3Control(const std::string& label, float* values, float resetValues = 0.0f, float columnWidth = 100.0f);

    /**
     * @brief Dibuja el inspector general para un actor específico.
     * @param actor Puntero al actor seleccionado.
     */
    void inspectorGeneral(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja los componentes específicos (Mesh, Light, etc.) asociados a un actor.
     * @param actor Puntero al actor inspeccionado.
     */
    void inspectorContainer(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja el panel de jerarquía (Outliner) mostrando todos los actores de la escena.
     * @param actors Vector con todos los actores actuales.
     */
    void outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    /**
     * @brief Maneja la edición de transformaciones (traslación, rotación, escala) usando ImGuizmo.
     * @param cam Cámara activa que ve el gizmo.
     * @param window Ventana principal.
     * @param actor Actor que está siendo editado.
     */
    void editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja la barra de herramientas para seleccionar la operación del Gizmo.
     */
    void drawGizmoToolbar();

    /**
     * @brief Convierte una matriz XMMATRIX a un arreglo de flotantes continuo de 16 elementos.
     * @param mat Matriz a convertir.
     * @param dest Puntero al arreglo de flotantes destino.
     */
    void ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

    /**
     * @brief Dibuja la cinta (ribbon) superior del editor.
     */
    void drawStudioTopRibbon();

    /**
     * @brief Dibuja la ventana del Viewport donde se renderiza la escena.
     * @param viewportSRV Textura del viewport para mostrar.
     */
    void drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

    /**
     * @brief Dibuja la cuadrícula de fondo en el viewport.
     * @param cam Cámara del editor.
     */
    void drawViewportGrid(Camera& cam);

    /**
     * @brief Configura el área de acoplamiento (Dockspace) principal del editor.
     */
    void drawEditorDockspace();

    /**
     * @brief Dibuja un panel de depuración para inspeccionar mapas de sombras y texturas base.
     * @param preShadowSRV Mapa previo de sombras.
     * @param viewportSRV Textura del viewport general.
     * @param shadowMapSRV Mapa final de sombras.
     */
    void drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
        ID3D11ShaderResourceView* viewportSRV,
        ID3D11ShaderResourceView* shadowMapSRV);

    /**
     * @brief Dibuja un panel de depuración para visualizar los buffers del Deferred Rendering (G-Buffer).
     * @param albedoMetallicSRV Textura combinada de Albedo y Metálico.
     * @param normalRoughnessSRV Textura combinada de Normales y Rugosidad.
     * @param worldAoSRV Textura de Posición del Mundo y Oclusión Ambiental.
     * @param emissiveAlphaSRV Textura de Emisión y Alfa.
     */
    void drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
        ID3D11ShaderResourceView* normalRoughnessSRV,
        ID3D11ShaderResourceView* worldAoSRV,
        ID3D11ShaderResourceView* emissiveAlphaSRV);

    /**
     * @brief Dibuja el panel para configurar la luz direccional principal.
     * @param lightDir Puntero a la dirección de la luz.
     * @param lightColor Puntero al color de la luz.
     */
    void drawLightingPanel(float* lightDir, float* lightColor);

    // void drawStatsPanel(float deltaTime);

     /**
      * @brief Dibuja el panel de estadísticas de rendimiento (FPS, Draw Calls).
      * @param deltaTime Tiempo transcurrido en el último frame.
      * @param drawCalls Llamadas de dibujado realizadas.
      */
    void drawStatsPanel(float deltaTime, unsigned int drawCalls);

    /**
     * @brief Dibuja el navegador de contenido mostrando los assets disponibles.
     * @param textureThumbs Lista de miniaturas de texturas.
     */
    void drawContentBrowser(const std::vector<AssetThumb>& textureThumbs);

    /**
     * @brief Dibuja el panel de la consola para registros y errores.
     */
    void drawConsolePanel();

    /**
     * @brief Dibuja la ventana de vista previa detallada de texturas.
     */
    void drawTexturePreview();

    /** @brief Consume la petición de reseteo. @return true si se solicitó. */
    bool consumeResetRequest() { bool r = m_resetRequested; m_resetRequested = false; return r; }

    /** @brief Consume la petición de enfoque (Focus). @return true si se solicitó. */
    bool consumeFocusRequest() { bool r = m_focusRequested; m_focusRequested = false; return r; }

    /** @brief Consume la petición de ajuste (Fit) de cámara. @return true si se solicitó. */
    bool consumeFitRequest() { bool r = m_fitRequested;   m_fitRequested = false; return r; }

    /** @brief Consume la petición de deshacer (Undo). @return true si se solicitó. */
    bool consumeUndoRequest() { bool r = m_undoRequested; m_undoRequested = false; return r; }

    /** @brief Consume la petición de rehacer (Redo). @return true si se solicitó. */
    bool consumeRedoRequest() { bool r = m_redoRequested; m_redoRequested = false; return r; }

private:
    bool show_exit_popup = false;               ///< Bandera para mostrar el popup de salida.
    ImDrawList* m_viewportDrawList = nullptr;   ///< Lista de dibujado superpuesta al viewport.
    bool m_viewportActive = false;              ///< Bandera que indica si el viewport está activo.
    bool m_dockLayoutInitialized = false;       ///< Bandera que indica si el dockspace fue inicializado.

public:
    bool m_isUsingGizmo = false;                ///< Indica si un Gizmo está siendo manipulado activamente.
    int selectedActorIndex = -1;                ///< Índice del actor actualmente seleccionado.
    ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);  ///< Posición del viewport.
    ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f); ///< Tamaño del viewport.
    bool m_viewportHovered = false;             ///< Indica si el ratón está sobre el viewport.
    bool m_viewportFocused = false;             ///< Indica si el viewport tiene el foco.

    bool m_visualizeDeferredShadowFactor = false; ///< Activa la visualización de sombras diferidas en depuración.
    int  m_deferredDebugViewMode = 0;             ///< Índice del modo de depuración activo.

    bool m_logShowInfo = true;                  ///< Filtro: Mostrar información en consola.
    bool m_logShowWarning = true;               ///< Filtro: Mostrar advertencias en consola.
    bool m_logShowError = true;                 ///< Filtro: Mostrar errores en consola.
    bool m_logAutoScroll = true;                ///< Autoscroll activado en consola.
    ImGuiTextFilter m_logFilter;                ///< Objeto de filtrado de texto para la consola.

    bool m_resetRequested = false;              ///< Petición de reseteo de escena.

    ID3D11ShaderResourceView* m_previewSRV = nullptr; ///< Textura a mostrar en la vista previa.
    std::string m_previewLabel;                       ///< Etiqueta de la textura en vista previa.
    bool m_showPreview = false;                       ///< Bandera para abrir la ventana de vista previa.

    // Camara / grid / snap
    bool  m_showGrid = true;                    ///< Muestra u oculta la cuadrícula.
    float m_gridSize = 10.0f;                   ///< Tamaño de la cuadrícula.
    bool  m_snapEnabled = false;                ///< Activa el ajuste por pasos (snap) en Gizmos.
    float m_snapTranslate = 0.5f;               ///< Incremento de ajuste de traslación.
    float m_snapRotate = 15.0f;                 ///< Incremento de ajuste de rotación.
    float m_snapScale = 0.1f;                   ///< Incremento de ajuste de escala.
    bool  m_focusRequested = false;             ///< Petición para enfocar actor.
    bool  m_fitRequested = false;               ///< Petición para encuadrar la escena.

    bool m_undoRequested = false;               ///< Petición para deshacer.
    bool m_redoRequested = false;               ///< Petición para rehacer.

    bool m_duplicateRequested = false;          ///< Petición para duplicar.
    bool m_deleteRequested = false;             ///< Petición para eliminar.
    bool m_copyRequested = false;               ///< Petición para copiar.
    bool m_pasteRequested = false;              ///< Petición para pegar.
    bool m_savePrefabRequested = false;         ///< Petición para guardar prefab.
    bool m_loadPrefabRequested = false;         ///< Petición para cargar prefab.

    std::string m_assetSpawnPath;               ///< Ruta del asset a generar (spawn).
    bool m_assetSpawnRequested = false;         ///< Petición para generar un asset en escena.

    /**
     * @brief Dibuja un contorno (outline) de selección sobre el objeto seleccionado.
     * @param cam Cámara activa.
     * @param localMin AABB Mínimo local.
     * @param localMax AABB Máximo local.
     * @param world Matriz de mundo del objeto.
     */
    void drawSelectionOutline(Camera& cam, const EU::Vector3& localMin, const EU::Vector3& localMax, const XMMATRIX& world);
    void drawLightGizmo(Camera& cam, EU::TSharedPointer<Actor> actor);

    bool m_createDirectionalLightRequested = false; ///< Petición para luz direccional.
    bool m_createPointLightRequested = false;       ///< Petición para luz puntual.
    bool m_createSpotLightRequested = false;        ///< Petición para luz spot.

    /** @brief Consume la petición de crear luz direccional. @return true si se solicitó. */
    bool consumeCreateDirectionalLightRequest()
    {
        bool r = m_createDirectionalLightRequested;
        m_createDirectionalLightRequested = false;
        return r;
    }

    /** @brief Consume la petición de crear luz puntual. @return true si se solicitó. */
    bool consumeCreatePointLightRequest()
    {
        bool r = m_createPointLightRequested;
        m_createPointLightRequested = false;
        return r;
    }

    /** @brief Consume la petición de crear luz focal (spot). @return true si se solicitó. */
    bool consumeCreateSpotLightRequest()
    {
        bool r = m_createSpotLightRequested;
        m_createSpotLightRequested = false;
        return r;
    }

};