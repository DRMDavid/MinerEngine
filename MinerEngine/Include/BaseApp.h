#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "ECS/Actor.h"
#include "GUI/GUI.h"
#include "SceneGraph\SceneGraph.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "EngineUtilities\Utilities\Skybox.h"
#include "EngineUtilities\Utilities\LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/RenderScene.h"
#include "CommandManager.h"
#include "PhysicsSystem.h"
#include "BehaviorSystem.h"
#include "AudioSystem.h"
#include "ParticleSystem.h"
#include "ParticleRenderer.h"
#include <string>   
#include <memory>


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @struct ActorClipboard
 * @brief Estructura que almacena los datos básicos de un actor copiado al portapapeles.
 */
struct ActorClipboard {
    std::string name;       ///< Nombre del actor copiado.
    EU::Vector3 position;   ///< Posición local guardada.
    EU::Vector3 rotation;   ///< Rotación local guardada (Euler).
    EU::Vector3 scale;      ///< Escala local guardada.
};

/**
 * @struct LoadedModel
 * @brief Estructura que encapsula todos los recursos asociados a un modelo 3D cargado en memoria.
 */
struct LoadedModel {
    Mesh mesh;                          ///< Geometría del modelo.
    Material material;                  ///< Material base del modelo.
    MaterialInstance materialInstance;  ///< Instancia específica del material.
    Texture albedo, normal, metallic, roughness, ao; ///< Texturas PBR del modelo.
    EU::Vector3 localMin;               ///< Coordenada mínima de la caja delimitadora (AABB) en espacio local.
    EU::Vector3 localMax;               ///< Coordenada máxima de la caja delimitadora (AABB) en espacio local.
};

/**
 * @struct GizmoEditState
 * @brief Captura el estado de transformación de un objeto antes o durante la edición con un Gizmo para el sistema de Deshacer/Rehacer.
 */
struct GizmoEditState {
    EU::Vector3 position; ///< Posición al momento de capturar el estado.
    EU::Vector3 rotation; ///< Rotación al momento de capturar el estado.
    EU::Vector3 scale;    ///< Escala al momento de capturar el estado.
};


/**
 * @enum EngineMode
 * @brief Representa el estado actual del motor.
 */
enum class EngineMode
{
    Edit = 0,
    Play,
    Paused
};

/**
 * @class BaseApp
 * @brief Clase base de la aplicación del motor.
 * Se encarga de inicializar los sistemas centrales (Ventana, DirectX, Escena, GUI),
 * gestionar el bucle principal, e implementar la lógica general del editor y la aplicación.
 */
class BaseApp {
public:

    BaseApp();
    ~BaseApp();
    //BaseApp() = default;
    //~BaseApp();
    //~BaseApp() { destroy(); }

    /**
     * @brief Preparación temprana antes de la inicialización completa.
     * @return Código HRESULT indicando el éxito o fallo de la operación.
     */
    HRESULT awake();

    /**
     * @brief Inicia el bucle principal de mensajes y ejecución de la aplicación.
     * @param hInst Instancia de la aplicación de Windows.
     * @param nCmdShow Modo de visualización de la ventana.
     * @return Código de salida de la aplicación al terminar el bucle.
     */
    int run(HINSTANCE hInst, int nCmdShow);

    /**
     * @brief Inicializa los subsistemas del motor: DirectX, GUI, Escena, Render Pipeline, etc.
     * @return Código HRESULT indicando el éxito o fallo de la inicialización.
     */
    HRESULT init();

    /**
     * @brief Actualiza la lógica de la aplicación y la escena.
     * @param deltaTime Tiempo transcurrido en segundos desde el último fotograma.
     */
    void update(float deltaTime);

    /**
     * @brief Renderiza el fotograma actual, procesando pases de render y la GUI.
     */
    void render();

    /**
     * @brief Limpia y libera todos los recursos inicializados (DirectX, memoria, etc.).
     */
    void destroy();

    /**
     * @brief Maneja el evento de redimensionamiento de la ventana principal.
     * @param newW Nuevo ancho de la ventana en píxeles.
     * @param newH Nuevo alto de la ventana en píxeles.
     */
    void onResize(unsigned int newW, unsigned int newH);

    /**
     * @brief Gestiona el redimensionamiento del panel del Viewport del editor (ImGui).
     */
    void handleEditorViewportResize();

    /**
     * @brief Guarda el estado actual de la escena en un archivo.
     * @param path Ruta del archivo donde se guardará la escena.
     * @return true si se guardó con éxito, false en caso contrario.
     */
    bool saveScene(const std::string& path);

    /**
     * @brief Carga una escena desde un archivo especificado.
     * @param path Ruta del archivo a cargar.
     * @return true si se cargó con éxito, false en caso contrario.
     */
    bool loadScene(const std::string& path);

    /**
     * @brief Obtiene la ruta por defecto donde se almacenan las escenas.
     * @return Cadena de texto con la ruta.
     */
    std::string getDefaultScenePath() const;

    /**
     * @brief Añade un nuevo actor a la jerarquía de la escena.
     * @param actor Puntero compartido al actor a añadir.
     */
    void addActorToScene(const EU::TSharedPointer<Actor>& actor);

    /**
     * @brief Remueve un actor existente de la escena.
     * @param actor Puntero compartido al actor a remover.
     */
    void removeActorFromScene(const EU::TSharedPointer<Actor>& actor);

private:

   
    /**
     * @brief Función de callback principal para gestionar mensajes de Windows.
     * @param hWnd Handle de la ventana.
     * @param message Mensaje recibido.
     * @param wParam Parámetro adicional del mensaje.
     * @param lParam Parámetro adicional del mensaje.
     * @return Resultado del procesamiento del mensaje.
     */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // Core Graphics e Infraestructura
    Window              m_window;               ///< Instancia de la ventana de la aplicación.
    Device              m_device;               ///< Dispositivo gráfico de DirectX.
    DeviceContext       m_deviceContext;        ///< Contexto del dispositivo para emitir comandos.
    SwapChain           m_swapChain;            ///< Swap chain para el intercambio de buffers.
    Texture             m_backBuffer;           ///< Textura del buffer trasero.
    RenderTargetView    m_renderTargetView;     ///< Vista de render target del back buffer.
    Texture             m_depthStencil;         ///< Textura para el buffer de profundidad y stencil.
    DepthStencilView    m_depthStencilView;     ///< Vista del buffer de profundidad.
    Viewport            m_viewport;             ///< Viewport principal asociado a la ventana.
    ShaderProgram       m_shaderProgram;        ///< Programa de shaders principal o por defecto.
    bool                m_d3dReady = false;     ///< Indica si DirectX se inicializó correctamente.
    Buffer              m_constantBuffer;       ///< Buffer constante principal de DirectX.
    CBMain              m_constantBufferStruct; ///< Estructura de datos del buffer constante principal.
    PhysicsSystem       m_physicsSystem;
    BehaviorSystem      m_behaviorSystem;
    AudioSystem         m_audioSystem;
    ParticleSystem      m_particleSystem;
    ParticleRenderer    m_particleRenderer;
    

    EngineMode m_engineMode = EngineMode::Edit;

    /**
     * @struct InitialTransform
     * @brief Captura los parámetros de transformación iniciales para una posible restauración.
     */
    struct InitialTransform {
        EU::Vector3 position;
        EU::Vector3 rotation;
        EU::Vector3 scale;
    };

    // Estado inicial y control de cámara
    bool m_initialStateCaptured = false;                  ///< Bandera que indica si el estado inicial fue capturado.
    EU::Vector3 m_initialLightDir;                        ///< Dirección inicial de la luz.
    EU::Vector3 m_initialLightColor;                      ///< Color inicial de la luz.
    EU::Vector3 m_initialCameraPos;                       ///< Posición inicial de la cámara.
    std::vector<InitialTransform> m_initialTransforms;    ///< Transformaciones originales de los actores.

    /** @brief Almacena el estado inicial de la escena para permitir su reseteo. */
    void captureInitialState();
    /** @brief Restaura la escena a los valores iniciales capturados. */
    void resetSceneToDefaults();
    void startPlayMode();
    void stopPlayMode();
    void togglePauseMode();
    void rotateActorInPlay(int actorIndex, float deltaTime);

    bool isPlaying() const;
    bool isPaused() const;
    bool isEditing() const;
    /** @brief Mueve y ajusta la cámara para enfocar el actor especificado. */
    void focusCameraOnActor(const EU::TSharedPointer<Actor>& actor);
    /** @brief Ajusta la cámara para visualizar todos los elementos de la escena. */
    void fitCameraToScene();

    unsigned int m_lastDrawCalls = 0; ///< Contador de draw calls del último fotograma.

    // Variables de Picking (Selección de objetos)
    EU::Vector3 m_modelLocalMin; ///< AABB local mínima temporal para validación de picking.
    EU::Vector3 m_modelLocalMax; ///< AABB local máxima temporal para validación de picking.

    /** @brief Ejecuta la lógica de Raycasting para seleccionar un actor usando el cursor en el viewport. */
    void pickActorFromMouse();

    // Comandos y Gizmos (Deshacer / Rehacer)
    CommandManager m_commands;            ///< Gestor para historial de comandos (Undo/Redo).
    bool m_prevGizmoUsing = false;        ///< Estado previo del uso del Gizmo (Frame anterior).
    bool m_gizmoEditing = false;          ///< Bandera activa durante una modificación con Gizmo.
    int  m_gizmoEditActorIndex = -1;      ///< Índice del actor actualmente editado por el Gizmo.
    GizmoEditState m_gizmoBefore;         ///< Estado guardado justo antes de empezar a usar el Gizmo.

    /**
     * @brief Captura la transformación actual de un actor por su índice.
     * @return true si el actor existe y se capturó correctamente.
     */
    bool captureGizmoState(int index, GizmoEditState& out);

    // Sistema de Portapapeles (Copiar / Pegar / Prefabs)
    ActorClipboard m_clipboard;     ///< Portapapeles interno de la aplicación.
    bool m_hasClipboard = false;    ///< Bandera que indica si hay datos en el portapapeles.

    /** @brief Instancia un modelo "Pistola" de prueba en la escena. */
    EU::TSharedPointer<Actor> spawnPistol(const std::string& name, const EU::Vector3& pos, const EU::Vector3& rot, const EU::Vector3& scale);
    void duplicateSelected();       ///< Duplica el actor actualmente seleccionado.
    void deleteSelected();          ///< Elimina el actor seleccionado de la escena.
    void copySelected();            ///< Copia los datos básicos del actor seleccionado al portapapeles.
    void pasteClipboard();          ///< Pega y crea un nuevo actor basado en los datos del portapapeles.
    void savePrefabSelected();      ///< Guarda el actor seleccionado como un archivo prefabricado (Prefab).
    void loadPrefab();              ///< Carga y anexa un archivo de Prefab a la escena actual.

    // Gestión de Recursos Dinámicos (Modelos y Texturas)
    std::vector<std::unique_ptr<LoadedModel>> m_loadedModels;   ///< Lista de modelos en memoria.
    std::vector<Texture> m_thumbTextures;                       ///< Texturas usadas como miniaturas en UI.
    std::vector<AssetThumb> m_thumbnails;                       ///< Datos para renderizar miniaturas en UI.

    /** @brief Carga un modelo 3D desde un archivo, generando recursos y un nuevo actor. */
    EU::TSharedPointer<Actor> loadModelActor(const std::string& modelPath);
    /** @brief Carga todas las texturas asociadas (PBR) para un modelo en una carpeta dada. */
    void loadModelTextures(LoadedModel& lm, const std::string& folder);
    /** @brief Genera o carga los recursos visuales que representarán los assets en el Content Browser. */
    void buildTextureThumbnails();

    /**
     * @brief Obtiene el AABB local combinado de los MeshRenderer del actor indicado.
     * @return true si se encontró al menos un componente con AABB.
     */
    bool getActorAABB(const EU::TSharedPointer<Actor>& actor, EU::Vector3& outMin, EU::Vector3& outMax);

    // Recursos "Hardcoded" (Materiales y Texturas base)
    Texture m_AlbedoSRV;
    Texture m_MetallicSRV;
    Texture m_RoughnessSRV;
    Texture m_AOSRV;
    Texture m_NormalSRV;
    Texture m_EmissiveSRV;
    Texture m_drakefireAlbedoSRV;
    Texture m_drakefireNormalSRV;
    Texture m_drakefireMetallicSRV;
    Texture m_drakefireRoughnessSRV;
    Texture m_drakefireAOSRV;

    Camera  m_camera;   ///< Cámara principal del Editor/Escena.

    // Elementos de la Escena
    SceneGraph m_sceneGraph;                                    ///< Árbol de la escena que gestiona jerarquías.
    std::vector<EU::TSharedPointer<Actor>> m_actors;            ///< Colección lineal de actores raíz en la escena.
    EU::TSharedPointer<Actor> m_cyberGun;                       ///< Puntero de referencia rápida al actor de prueba.
    EU::TSharedPointer<Actor> m_drakefirePistol;                ///< Puntero de referencia rápida a segundo actor.
    EU::TSharedPointer<Actor> m_directionalLightActor;          ///< Puntero al actor que contiene la luz direccional principal.
    /**
    * @brief Actor invisible utilizado como suelo físico de la escena.
    */
    EU::TSharedPointer<Actor> m_groundActor;

    Model3D* m_model;                   ///< Puntero temporal o de compatibilidad a malla de modelo 1.
    Model3D* m_drakefireModel = nullptr;///< Puntero temporal a malla de modelo 2.

    GUI     m_gui;                      ///< Módulo encargado de gestionar y renderizar la interfaz de usuario.
    bool    m_guiInitialized = false;   ///< Indica si la GUI fue inicializada correctamente.
    EU::Vector3 m_cameraPos;            ///< Copia local de la posición de la cámara (para UI o accesos rápidos).

    // Sistemas Gráficos y Render
    Skybox m_skybox;                            ///< Entorno panorámico (Skybox)
    Texture m_skyboxTex;                        ///< Textura cúbica del Skybox.
    RasterizerState m_defaultRasterizer;        ///< Estado de rasterización base.
    DepthStencilState m_defaultDepthStencil;    ///< Estado de Depth/Stencil base.
    SamplerState m_defaultSampler;              ///< Estado de muestreado de texturas base.
    Mesh m_cyberGunRenderMesh;                  ///< Contenedor temporal de malla.
    Mesh m_drakefireRenderMesh;                 ///< Contenedor temporal de malla.
    Material m_pbrMaterial;                     ///< Archivo/Definición del material PBR base opaco.
    Material m_transparentPbrMaterial;          ///< Archivo/Definición del material PBR base con transparencia.
    MaterialInstance m_cyberGunMaterial;        ///< Instancia de material vinculada al modelo CyberGun.
    MaterialInstance m_drakefireMaterial;       ///< Instancia de material vinculada al modelo Drakefire.

    // Render Pipeline y Viewport Avanzado
    EditorViewportPass m_editorViewportPass;    ///< Pase de render específico que encapsula el dibujado hacia el panel de la GUI.
    RenderPipeline m_renderPipeline;            ///< Sistema coordinador de los pases de render (Deferred, Forward, Sombras, etc).
    RenderScene m_renderScene;                  ///< Repositorio optimizado de datos enviados desde la jerarquía hacia el render pipeline.

    // Control asíncrono de redimensionamiento
    bool m_editorViewportResizePending = false;     ///< Bandera que activa un Resize del viewport in-editor.
    unsigned int m_pendingViewportWidth = 1;        ///< Ancho solicitado para el viewport.
    unsigned int m_pendingViewportHeight = 1;       ///< Alto solicitado para el viewport.
    unsigned int m_lastRequestedViewportWidth = 1;  ///< Último ancho estable de viewport.
    unsigned int m_lastRequestedViewportHeight = 1; ///< Último alto estable de viewport.
    int m_viewportResizeStableFrames = 0;           ///< Contador para aplicar el resize de manera fluida y sin flicker.
};