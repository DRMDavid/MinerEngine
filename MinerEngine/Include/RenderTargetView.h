#pragma once
#include "Prerequisites.h"

// Declaraciones adelantadas de clases
class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/**
 * @class RenderTargetView
 * @brief Encapsula la creación, gestión y uso de un Render Target View (RTV) en Direct3D 11.
 * @details
 *  Esta clase gestiona el ciclo de vida del RTV, desde su inicialización hasta su destrucción,
 *  y permite configurarlo para operaciones de renderizado, ya sea desde el back buffer o desde
 *  texturas personalizadas.
 */
class RenderTargetView {
public:
  /**
   * @brief Crea un objeto vacío de RenderTargetView.
   * @details No realiza ninguna inicialización; se requiere llamar a `init()` antes de usarlo.
   */
  RenderTargetView() = default;

  /**
   * @brief Destructor trivial del objeto.
   * @note El recurso asociado no se libera automáticamente. Para liberar el RTV,
   *       es necesario invocar manualmente el método `destroy()`.
   */
  ~RenderTargetView() = default;

  /**
   * @brief Construye un Render Target View a partir del back buffer.
   * @param device     Dispositivo Direct3D responsable de la creación.
   * @param backBuffer Textura que representa el back buffer del swap chain.
   * @param Format     Formato en el que se definirá el RTV (por ejemplo, `DXGI_FORMAT_R8G8B8A8_UNORM`).
   * @return Devuelve `S_OK` en caso de éxito o un código HRESULT en caso de fallo.
   * @post Si el resultado es satisfactorio, `m_renderTargetView` apuntará a un recurso válido.
   */
  HRESULT init(Device& device, Texture& backBuffer, DXGI_FORMAT Format);

  /**
   * @brief Inicializa un RTV a partir de una textura arbitraria.
   * @param device        Dispositivo que gestiona la creación del recurso.
   * @param inTex         Textura destino donde se dibujará la salida del renderizado.
   * @param ViewDimension Dimensión de la vista (por ejemplo, `D3D11_RTV_DIMENSION_TEXTURE2D`).
   * @param Format        Formato deseado para la vista.
   * @return `S_OK` si la operación se completa con éxito; de lo contrario, se devuelve un HRESULT de error.
   * @note Este método es útil para render targets secundarios, como buffers diferidos o mapas de sombras.
   */
  HRESULT init(Device& device,
    Texture& inTex,
    D3D11_RTV_DIMENSION ViewDimension,
    DXGI_FORMAT Format);

  /**
   * @brief Punto de extensión para actualizar parámetros internos del RTV.
   * @details Actualmente no realiza ninguna acción, pero puede usarse para reconfigurar el RTV
   *          dinámicamente en versiones futuras del motor.
   */
  void update();

  /**
   * @brief Aplica el RTV al pipeline de render y lo limpia con un color específico.
   * @param deviceContext    Contexto de dispositivo donde se establecerá la vista.
   * @param depthStencilView Vista de Depth Stencil a enlazar junto al RTV.
   * @param numViews         Número de vistas a establecer (habitualmente 1).
   * @param ClearColor       Color RGBA que se usará para limpiar el render target.
   * @pre Debe haberse invocado previamente `init()` con éxito.
   */
  void render(DeviceContext& deviceContext,
    DepthStencilView& depthStencilView,
    unsigned int numViews,
    const float ClearColor[4]);

  /**
   * @brief Asigna el RTV al pipeline sin realizar operaciones de limpieza.
   * @param deviceContext Contexto del dispositivo donde se establecerá.
   * @param numViews      Número de vistas a utilizar (generalmente 1).
   * @pre El RTV debe haberse creado correctamente mediante `init()`.
   */
  void render(DeviceContext& deviceContext, unsigned int numViews);

  /**
   * @brief Libera el recurso `ID3D11RenderTargetView` asociado.
   * @details Este método es seguro de llamar múltiples veces; después de la liberación,
   *          el puntero interno se establecerá en `nullptr`.
   * @post `m_renderTargetView` quedará en `nullptr` tras la llamada.
   */
  void destroy();

private:
  /**
   * @brief Puntero COM al recurso de vista de destino de renderizado en Direct3D 11.
   * @details Contiene la referencia al objeto RTV. Será válido tras una inicialización exitosa
   *          y se liberará cuando se invoque `destroy()`.
   */
  ID3D11RenderTargetView* m_renderTargetView = nullptr;
};
