#pragma once
#include "Prerequisites.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

// Forward declarations
class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

/**
 * @struct AssetThumb
 * @brief Estructura que representa una vista previa (thumbnail) de un asset en la interfaz.
 */
struct AssetThumb {
    std::string name;                   ///< Nombre del asset.
    ID3D11ShaderResourceView* srv;      ///< Vista de recurso de shader para renderizar la imagen.
};

/**
 * @class GUI
 * @brief Clase principal encargada de gestionar y renderizar la interfaz gráfica de usuario (GUI) del motor utilizando ImGui.
 */
class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    /**
     * @brief Inicializa configuraciones previas a la creación de la ventana (si las hay).
     */
    void awake();

    /**
     * @brief Inicializa el contexto de ImGui y los backends asociados (Win32 y DirectX 11).
     * @param window Referencia a la ventana principal de la aplicación.
     * @param device Referencia al dispositivo de DirectX 11.
     * @param deviceContext Referencia al contexto del dispositivo de DirectX 11.
     */
    void init(Window& window, Device& device, DeviceContext& deviceContext);

    /**
     * @brief Actualiza la lógica de la interfaz de usuario en cada fotograma.
     * @param viewport Referencia al viewport actual.
     * @param window Referencia a la ventana principal.
     */
    void update(Viewport& viewport, Window& window);

    /**
     * @brief Renderiza todos los comandos de dibujado de ImGui acumulados durante el frame.
     */
    void render();

    /**
     * @brief Libera los recursos de ImGui y cierra los backends.
     */
    void destroy();

    /**
     * @brief Dibuja la barra de herramientas superior (ToolBar).
     */
    void ToolBar();

    /**
     * @brief Dibuja el modal o lógica para cerrar la aplicación.
     */
    void closeApp();

    /**
     * @brief Muestra información adicional en forma de ToolTip.
     */
    void toolTipData();

    /**
     * @brief Aplica un estilo visual específico (Apple Liquid) a la interfaz.
     * @param opacity Nivel de opacidad global de la interfaz.
     * @param accent Color de acento para los elementos interactivos.
     */
    void appleLiquidStyle(float opacity, ImVec4 accent);

    /**
     * @brief Dibuja un control personalizado para vectores de 3 componentes (XYZ).
     * @param label Etiqueta del control.
     * @param values Puntero al arreglo de 3 flotantes que se modificarán.
     * @param resetValues Valor por defecto al que se reinician las componentes.
     * @param columnWidth Ancho de la columna para la etiqueta.
     */
    void vec3Control(const std::string& label, float* values, float resetValues = 0.0f, float columnWidth = 100.0f);

    /**
     * @brief Dibuja el panel del inspector general para un actor seleccionado.
     * @param actor Puntero compartido al actor a inspeccionar.
     */
    void inspectorGeneral(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja los componentes y propiedades específicas del actor en el inspector.
     * @param actor Puntero compartido al actor.
     */
    void inspectorContainer(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja el panel de jerarquía (Outliner) mostrando la lista de actores en la escena.
     * @param actors Vector con los punteros compartidos de los actores de la escena.
     */
    void outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    /**
     * @brief Maneja la lógica y renderizado de los gizmos de transformación (ImGuizmo) para un actor.
     * @param cam Cámara activa que visualiza el gizmo.
     * @param window Ventana principal.
     * @param actor Actor que será transformado.
     */
    void editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja la barra de herramientas para controlar los modos de transformación (Traslación, Rotación, Escala).
     */
    void drawGizmoToolbar();

    /**
     * @brief Convierte una matriz XMMATRIX a un arreglo de flotantes continuo.
     * @param mat Matriz de entrada.
     * @param dest Puntero al arreglo destino de 16 flotantes.
     */
    void ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

    /**
     * @brief Dibuja el panel superior con herramientas y menús del editor.
     */
    void drawStudioTopRibbon();

    /**
     * @brief Dibuja el panel central donde se renderiza la vista principal (Viewport).
     * @param viewportSRV Textura del viewport para renderizar en la ventana de ImGui.
     */
    void drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

    /**
     * @brief Dibuja una cuadrícula (grid) en el viewport.
     * @param cam Cámara activa desde la cual se proyecta la cuadrícula.
     */
    void drawViewportGrid(Camera& cam);

    /**
     * @brief Configura y dibuja el sistema de ventanas acoplables (Dockspace).
     */
    void drawEditorDockspace();

    /**
     * @brief Dibuja un panel de depuración para observar los pases de renderizado (sombras, etc.).
     * @param preShadowSRV Textura de mapa de sombras previo.
     * @param viewportSRV Textura principal del viewport.
     * @param shadowMapSRV Textura final del mapa de sombras.
     */
    void drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
        ID3D11ShaderResourceView* viewportSRV,
        ID3D11ShaderResourceView* shadowMapSRV);

    /**
     * @brief Dibuja un panel para visualizar los diferentes objetivos de renderizado (G-Buffer) del Deferred Shading.
     * @param albedoMetallicSRV Textura que contiene Albedo y Metalizado.
     * @param normalRoughnessSRV Textura que contiene Normales y Rugosidad.
     * @param worldAoSRV Textura que contiene Posición en el mundo y Oclusión Ambiental.
     * @param emissiveAlphaSRV Textura que contiene Emisión y Alfa.
     */
    void drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
        ID3D11ShaderResourceView* normalRoughnessSRV,
        ID3D11ShaderResourceView* worldAoSRV,
        ID3D11ShaderResourceView* emissiveAlphaSRV);

    /**
     * @brief Dibuja un panel para la configuración de iluminación global.
     * @param lightDir Puntero a la dirección de la luz (arreglo de 3).
     * @param lightColor Puntero al color de la luz (arreglo de 3 o 4).
     */
    void drawLightingPanel(float* lightDir, float* lightColor);

    /**
     * @brief Dibuja un panel con estadísticas de rendimiento.
     * @param deltaTime Tiempo transcurrido en el último fotograma.
     * @param drawCalls Número total de llamadas de dibujado emitidas en el frame.
     */
    void drawStatsPanel(float deltaTime, unsigned int drawCalls);

    /**
     * @brief Dibuja el navegador de contenido (Content Browser) mostrando los assets disponibles.
     * @param textureThumbs Lista de vistas previas de assets cargados.
     */
    void drawContentBrowser(const std::vector<AssetThumb>& textureThumbs);

    /**
     * @brief Dibuja la consola de registro para mostrar mensajes de depuración.
     */
    void drawConsolePanel();

    /**
     * @brief Dibuja una ventana de vista previa para inspeccionar una textura específica en detalle.
     */
    void drawTexturePreview();

    // ==========================================
    // MÉTODOS DE CONSUMO DE EVENTOS (REQUESTS)
    // ==========================================

    /** @brief Consume y reinicia la petición de reseteo. @return true si se solicitó un reseteo. */
    bool consumeResetRequest() { bool r = m_resetRequested; m_resetRequested = false; return r; }

    /** @brief Consume y reinicia la petición de enfoque (Focus). @return true si se solicitó un enfoque. */
    bool consumeFocusRequest() { bool r = m_focusRequested; m_focusRequested = false; return r; }

    /** @brief Consume y reinicia la petición de ajuste en pantalla (Fit). @return true si se solicitó un ajuste. */
    bool consumeFitRequest() { bool r = m_fitRequested;   m_fitRequested = false; return r; }

    /** @brief Consume y reinicia la petición de deshacer (Undo). @return true si se solicitó deshacer. */
    bool consumeUndoRequest() { bool r = m_undoRequested; m_undoRequested = false; return r; }

    /** @brief Consume y reinicia la petición de rehacer (Redo). @return true si se solicitó rehacer. */
    bool consumeRedoRequest() { bool r = m_redoRequested; m_redoRequested = false; return r; }

private:
    bool m_show_exit_popup = false;             ///< Indica si el popup de salida está activo.
    ImDrawList* m_viewportDrawList = nullptr;   ///< Lista de dibujado superpuesta al viewport.
    bool m_viewportActive = false;              ///< Indica si la pestaña del viewport está activa.
    bool m_dockLayoutInitialized = false;       ///< Indica si el diseño del dockspace ya se inicializó.

public:
    // Variables de estado del Viewport y Gizmos
    bool m_isUsingGizmo = false;                ///< Indica si el usuario está interactuando activamente con un Gizmo.
    int selectedActorIndex = -1;                ///< Índice del actor seleccionado actualmente.
    ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);  ///< Posición del viewport en pantalla.
    ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f); ///< Tamaño actual del panel del viewport.
    bool m_viewportHovered = false;             ///< Indica si el cursor está sobre el viewport.
    bool m_viewportFocused = false;             ///< Indica si el panel del viewport tiene el foco.

    // Variables de depuración (Deferred Rendering)
    bool m_visualizeDeferredShadowFactor = false; ///< Activa/Desactiva la visualización de factor de sombras.
    int  m_deferredDebugViewMode = 0;             ///< Índice del modo de depuración de vistas múltiples.

    // Variables de la Consola
    bool m_logShowInfo = true;                  ///< Muestra mensajes de información en la consola.
    bool m_logShowWarning = true;               ///< Muestra advertencias en la consola.
    bool m_logShowError = true;                 ///< Muestra errores en la consola.
    bool m_logAutoScroll = true;                ///< Activa el auto-scroll en la consola.
    ImGuiTextFilter m_logFilter;                ///< Filtro de texto de ImGui para la consola.

    bool m_resetRequested = false;              ///< Bandera interna de petición de reinicio.

    // Variables de Vista Previa de Texturas
    ID3D11ShaderResourceView* m_previewSRV = nullptr; ///< SRV de la textura en vista previa.
    std::string m_previewLabel;                       ///< Etiqueta identificadora de la textura.
    bool m_showPreview = false;                       ///< Bandera para mostrar la ventana de vista previa.

    // Configuración de Cámara, Cuadrícula y Ajuste de Precisión (Snap)
    bool  m_showGrid = true;                    ///< Activa/Desactiva el dibujo de la cuadrícula.
    float m_gridSize = 10.0f;                   ///< Tamaño total de la cuadrícula.
    bool  m_snapEnabled = false;                ///< Activa el ajuste (snapping) para los gizmos.
    float m_snapTranslate = 0.5f;               ///< Incremento de ajuste para traslación.
    float m_snapRotate = 15.0f;                 ///< Incremento de ajuste para rotación.
    float m_snapScale = 0.1f;                   ///< Incremento de ajuste para escala.
    bool  m_focusRequested = false;             ///< Petición de cámara para enfocar el objeto seleccionado.
    bool  m_fitRequested = false;               ///< Petición de cámara para ajustar la vista a la escena.

    // Peticiones de historial y edición
    bool m_undoRequested = false;               ///< Petición para deshacer acción.
    bool m_redoRequested = false;               ///< Petición para rehacer acción.
    bool m_duplicateRequested = false;          ///< Petición para duplicar actor seleccionado.
    bool m_deleteRequested = false;             ///< Petición para eliminar actor seleccionado.
    bool m_copyRequested = false;               ///< Petición para copiar selección.
    bool m_pasteRequested = false;              ///< Petición para pegar selección.
    bool m_savePrefabRequested = false;         ///< Petición para guardar un Prefab.
    bool m_loadPrefabRequested = false;         ///< Petición para cargar un Prefab.

    // Creación/Generación de Assets
    std::string m_assetSpawnPath;               ///< Ruta del asset a instanciar en escena.
    bool m_assetSpawnRequested = false;         ///< Petición para instanciar un asset.

    /**
     * @brief Dibuja un contorno (outline) de selección visual para un objeto específico en el viewport.
     * @param cam Cámara activa.
     * @param localMin Mínimo de la caja delimitadora (AABB) local.
     * @param localMax Máximo de la caja delimitadora (AABB) local.
     * @param world Matriz de transformación del objeto en el mundo.
     */
    void drawSelectionOutline(Camera& cam, const EU::Vector3& localMin, const EU::Vector3& localMax, const XMMATRIX& world);

    // Banderas de creación de Luces
    bool m_createDirectionalLightRequested = false; ///< Petición para crear luz direccional.
    bool m_createPointLightRequested = false;       ///< Petición para crear luz puntual.
    bool m_createSpotLightRequested = false;        ///< Petición para crear luz focal (spot).

    /** @brief Consume y reinicia la petición de creación de Luz Direccional. @return true si se solicitó. */
    bool consumeCreateDirectionalLightRequest()
    {
        bool r = m_createDirectionalLightRequested;
        m_createDirectionalLightRequested = false;
        return r;
    }

    /** @brief Consume y reinicia la petición de creación de Luz Puntual. @return true si se solicitó. */
    bool consumeCreatePointLightRequest()
    {
        bool r = m_createPointLightRequested;
        m_createPointLightRequested = false;
        return r;
    }

    /** @brief Consume y reinicia la petición de creación de Luz Focal (Spot). @return true si se solicitó. */
    bool consumeCreateSpotLightRequest()
    {
        bool r = m_createSpotLightRequested;
        m_createSpotLightRequested = false;
        return r;
    }
};