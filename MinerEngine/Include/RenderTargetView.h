#pragma once
#include "Prerequisites.h"

// Declaraciones adelantadas de clases
class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/**
 * @class RenderTargetView
 * @brief Encapsula la creaci�n, gesti�n y uso de un Render Target View (RTV) en Direct3D 11.
 * @details
 *  Esta clase gestiona el ciclo de vida del RTV, desde su inicializaci�n hasta su destrucci�n,
 *  y permite configurarlo para operaciones de renderizado, ya sea desde el back buffer o desde
 *  texturas personalizadas.
 */
class RenderTargetView {
public:
  /**
   * @brief Crea un objeto vac�o de RenderTargetView.
   * @details No realiza ninguna inicializaci�n; se requiere llamar a `init()` antes de usarlo.
   */
  RenderTargetView() = default;

  /**
   * @brief Destructor trivial del objeto.
   * @note El recurso asociado no se libera autom�ticamente. Para liberar el RTV,
   *       es necesario invocar manualmente el m�todo `destroy()`.
   */
  ~RenderTargetView() = default;

  /**
   * @brief Construye un Render Target View a partir del back buffer.
   * @param device     Dispositivo Direct3D responsable de la creaci�n.
   * @param backBuffer Textura que representa el back buffer del swap chain.
   * @param Format     Formato en el que se definir� el RTV (por ejemplo, `DXGI_FORMAT_R8G8B8A8_UNORM`).
   * @return Devuelve `S_OK` en caso de �xito o un c�digo HRESULT en caso de fallo.
   * @post Si el resultado es satisfactorio, `m_renderTargetView` apuntar� a un recurso v�lido.
   */
  HRESULT init(Device& device, Texture& backBuffer, DXGI_FORMAT Format);

  /**
   * @brief Inicializa un RTV a partir de una textura arbitraria.
   * @param device        Dispositivo que gestiona la creaci�n del recurso.
   * @param inTex         Textura destino donde se dibujar� la salida del renderizado.
   * @param ViewDimension Dimensi�n de la vista (por ejemplo, `D3D11_RTV_DIMENSION_TEXTURE2D`).
   * @param Format        Formato deseado para la vista.
   * @return `S_OK` si la operaci�n se completa con �xito; de lo contrario, se devuelve un HRESULT de error.
   * @note Este m�todo es �til para render targets secundarios, como buffers diferidos o mapas de sombras.
   */
  HRESULT init(Device& device,
    Texture& inTex,
    D3D11_RTV_DIMENSION ViewDimension,
    DXGI_FORMAT Format);

  /**
   * @brief Punto de extensi�n para actualizar par�metros internos del RTV.
   * @details Actualmente no realiza ninguna acci�n, pero puede usarse para reconfigurar el RTV
   *          din�micamente en versiones futuras del motor.
   */
  void update();

  /**
   * @brief Aplica el RTV al pipeline de render y lo limpia con un color espec�fico.
   * @param deviceContext    Contexto de dispositivo donde se establecer� la vista.
   * @param depthStencilView Vista de Depth Stencil a enlazar junto al RTV.
   * @param numViews         N�mero de vistas a establecer (habitualmente 1).
   * @param ClearColor       Color RGBA que se usar� para limpiar el render target.
   * @pre Debe haberse invocado previamente `init()` con �xito.
   */
  void render(DeviceContext& deviceContext,
    DepthStencilView& depthStencilView,
    unsigned int numViews,
    const float ClearColor[4]);

  /**
   * @brief Asigna el RTV al pipeline sin realizar operaciones de limpieza.
   * @param deviceContext Contexto del dispositivo donde se establecer�.
   * @param numViews      N�mero de vistas a utilizar (generalmente 1).
   * @pre El RTV debe haberse creado correctamente mediante `init()`.
   */
  void render(DeviceContext& deviceContext, unsigned int numViews);

  /**
   * @brief Libera el recurso `ID3D11RenderTargetView` asociado.
   * @details Este m�todo es seguro de llamar m�ltiples veces; despu�s de la liberaci�n,
   *          el puntero interno se establecer� en `nullptr`.
   * @post `m_renderTargetView` quedar� en `nullptr` tras la llamada.
   */
  void destroy();

private:
  /**
   * @brief Puntero COM al recurso de vista de destino de renderizado en Direct3D 11.
   * @details Contiene la referencia al objeto RTV. Ser� v�lido tras una inicializaci�n exitosa
   *          y se liberar� cuando se invoque `destroy()`.
   */
  ID3D11RenderTargetView* m_renderTargetView = nullptr;
};
