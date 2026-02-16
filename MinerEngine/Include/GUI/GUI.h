#pragma once
#include "Prerequisites.h"
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

/**
 * @class GUI
 * @brief Clase encargada de gestionar la interfaz de usuario (UI) del motor.
 * * Esta clase centraliza la inicialización, actualización y renderizado de ImGui,
 * así como la gestión de herramientas de edición como el Inspector y Gizmos.
 */
class GUI {
public:
    /** @brief Constructor por defecto. */
    GUI() = default;

    /** @brief Destructor por defecto. */
    ~GUI() = default;

    /**
     * @brief Realiza la lógica previa a la inicialización de los componentes.
     */
    void awake();

    /**
     * @brief Inicializa el contexto de ImGui y los backends para Win32 y DirectX 11.
     * @param window Referencia a la ventana de la aplicación.
     * @param device Referencia al dispositivo de DirectX 11.
     * @param deviceContext Referencia al contexto del dispositivo de DirectX 11.
     */
    void init(Window& window, Device& device, DeviceContext& deviceContext);

    /**
     * @brief Prepara el nuevo frame de la UI.
     * @param viewport Referencia al viewport actual.
     * @param window Referencia a la ventana para cálculos de dimensiones.
     */
    void update(Viewport& viewport, Window& window);

    /**
     * @brief Renderiza los datos de ImGui en la pantalla.
     */
    void render();

    /**
     * @brief Libera los recursos de ImGui y destruye el contexto.
     */
    void destroy();

    /**
     * @brief Dibuja la barra de herramientas superior (ToolBar).
     */
    void ToolBar();

    /**
     * @brief Gestiona el cierre de la aplicación a través de la interfaz.
     */
    void closeApp();

    /**
     * @brief Muestra información o descripciones emergentes (tooltips).
     */
    void toolTipData();

    /**
     * @brief Aplica un estilo visual personalizado inspirado en Apple/Liquid.
     * @param opacity Nivel de opacidad de las ventanas (0.0f a 1.0f).
     * @param accent Color de acento de la interfaz en formato ImVec4.
     */
    void appleLiquidStyle(float opacity /*0..1f*/, ImVec4 accent /*=#0A84FF*/);

    /**
     * @brief Crea un control personalizado para editar vectores de 3 componentes (Vector3).
     * @param label Etiqueta identificadora del control.
     * @param values Puntero al arreglo de floats que se modificará.
     * @param resetValues Valor por defecto al presionar el botón de reset.
     * @param columnWidth Ancho de la columna de la etiqueta.
     */
    void vec3Control(const std::string& label,
        float* values,
        float resetValues = 0.0f,
        float columnWidth = 100.0f);

    /**
     * @brief Dibuja la ventana del Inspector General para un Actor seleccionado.
     * @param actor Puntero inteligente al actor que se desea inspeccionar.
     */
    void inspectorGeneral(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja el contenedor de propiedades específicas del actor.
     * @param actor Puntero inteligente al actor.
     */
    void inspectorContainer(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Muestra la jerarquía de objetos (Outliner) en la escena.
     * @param actors Vector de actores presentes en la escena actual.
     */
    void outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    /**
     * @brief Gestiona la manipulación de transformaciones mediante Gizmos (ImGuizmo).
     * @param view Matriz de vista de la cámara.
     * @param projection Matriz de proyección de la cámara.
     * @param actor Actor cuyas transformaciones serán editadas.
     */
    void editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor);

    /**
     * @brief Dibuja la barra de herramientas específica para los Gizmos (Traslación, Rotación, Escala).
     */
    void drawGizmoToolbar();

    /**
     * @brief Función auxiliar para convertir una XMMATRIX a un arreglo de floats.
     * * Necesaria para la compatibilidad con ImGuizmo, que requiere arreglos de tipo float[16].
     * @param mat Matriz de DirectX Math a convertir.
     * @param dest Puntero al destino donde se copiarán los 16 valores flotantes.
     */
    void ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

private:
    bool checkboxValue = true;       ///< Valor de prueba para checkbox.
    bool checkboxValue2 = false;     ///< Segundo valor de prueba para checkbox.
    std::vector<const char*> m_objectsNames; ///< Lista de nombres de objetos para la interfaz.
    std::vector<const char*> m_tooltips;     ///< Almacén de textos para tooltips.

    bool show_exit_popup = false;    ///< Estado del popup de confirmación de salida.

public:
    int selectedActorIndex = -1;     ///< Índice del actor seleccionado actualmente en el Outliner.
};