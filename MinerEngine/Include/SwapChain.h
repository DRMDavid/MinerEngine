#pragma once
#include "Prerequisites.h"

// Declaraciones adelantadas de clases
class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class SwapChain
 * @brief Gestiona el ciclo de vida de la cadena de intercambio (Swap Chain) en Direct3D 11.
 * @details
 *  La clase encapsula la creaci�n, configuraci�n y presentaci�n de buffers en pantalla.
 *  Su funci�n principal es coordinar el back buffer con el dispositivo gr�fico y la ventana,
 *  facilitando el renderizado continuo en aplicaciones en tiempo real.
 */
class SwapChain {
public:
  /**
   * @brief Constructor por defecto.
   * @details No realiza inicializaci�n; se debe llamar a `init()` antes de usar el objeto.
   */
  SwapChain() = default;

  /**
   * @brief Destructor por defecto.
   * @note No libera recursos autom�ticamente. Se recomienda llamar a `destroy()`
   *       antes de la destrucci�n para evitar fugas de memoria.
   */
  ~SwapChain() = default;

  /**
   * @brief Inicializa la cadena de intercambio y la asocia a una ventana.
   * @param device        Dispositivo Direct3D responsable de la creaci�n.
   * @param deviceContext Contexto de dispositivo que gestionar� las operaciones de renderizado.
   * @param backBuffer    Textura que se utilizar� como back buffer.
   * @param window        Ventana destino donde se presentar� el contenido renderizado.
   * @return `S_OK` si la inicializaci�n fue exitosa; en caso contrario, un c�digo `HRESULT` de error.
   * @pre El dispositivo y el contexto deben estar correctamente creados.
   * @post `m_swapChain` apuntar� a un objeto v�lido si la funci�n tiene �xito.
   */
  HRESULT init(Device& device,
               DeviceContext& deviceContext,
               Texture& backBuffer,
               Window window);

  /**
   * @brief Punto de entrada para actualizaciones din�micas de la cadena de intercambio.
   * @details
   *  Actualmente es un marcador de posici�n sin implementaci�n. Puede usarse
   *  en el futuro para reconfigurar par�metros del swap chain en tiempo de ejecuci�n,
   *  como el tama�o del buffer o el formato de color.
   */
  void update();

  /**
   * @brief Realiza tareas relacionadas con el renderizado previas a la presentaci�n.
   * @details
   *  Este m�todo puede emplearse para preparar el back buffer o ejecutar operaciones
   *  necesarias antes de mostrar el contenido final en pantalla.
   */
  void render();

  /**
   * @brief Libera todos los recursos asociados al swap chain.
   * @details
   *  Incluye la liberaci�n del objeto `IDXGISwapChain` y cualquier recurso relacionado
   *  con el DXGI (dispositivo, adaptador, f�brica, etc.).
   * @post Todos los punteros internos se establecen en `nullptr` tras la ejecuci�n.
   */
  void destroy();

  /**
   * @brief Presenta el contenido del back buffer en la ventana asociada.
   * @details
   *  Llama internamente a `IDXGISwapChain::Present()`, mostrando el resultado del
   *  renderizado en pantalla. Es el �ltimo paso del pipeline de render.
   * @pre La cadena de intercambio debe estar correctamente inicializada.
   */
  void present();

public:
  /**
   * @brief Puntero COM al objeto de la cadena de intercambio de DXGI.
   * @details Representa el enlace entre el dispositivo gr�fico y el sistema de presentaci�n.
   */
  IDXGISwapChain* m_swapChain = nullptr;

  /**
   * @brief Tipo de controlador utilizado por Direct3D.
   * @details Define si el dispositivo est� basado en hardware, software, referencia, etc.
   *          Por defecto, se inicializa en `D3D_DRIVER_TYPE_NULL`.
   */
  D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
  /**
   * @brief Nivel de caracter�sticas de Direct3D utilizado por el dispositivo.
   * @details Establece la versi�n m�nima de caracter�sticas soportadas.
   *          El valor por defecto es `D3D_FEATURE_LEVEL_11_0`.
   */
  D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

  /**
   * @brief N�mero de muestras utilizadas para antialiasing (MSAA).
   * @details Controla la calidad del suavizado de bordes.
   *          Valores mayores mejoran la calidad a costa de rendimiento.
   */
  unsigned int m_sampleCount;

  /**
   * @brief Cantidad de niveles de calidad disponibles para MSAA.
   * @details Determinado durante la creaci�n del dispositivo. Afecta al nivel de suavizado posible.
   */
  unsigned int m_qualityLevels;

  /**
   * @brief Puntero al objeto `IDXGIDevice` asociado al dispositivo Direct3D.
   * @details Permite acceder a informaci�n sobre el hardware de GPU subyacente.
   */
  IDXGIDevice* m_dxgiDevice = nullptr;

  /**
   * @brief Adaptador DXGI en uso por el dispositivo gr�fico.
   * @details Representa la tarjeta gr�fica o GPU utilizada para renderizar.
   */
  IDXGIAdapter* m_dxgiAdapter = nullptr;

  /**
   * @brief F�brica DXGI responsable de crear el swap chain.
   * @details Gestiona la interacci�n entre el adaptador, el dispositivo y la ventana.
   */
  IDXGIFactory* m_dxgiFactory = nullptr;
};
