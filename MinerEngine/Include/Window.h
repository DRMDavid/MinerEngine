#pragma once
#include "Prerequisites.h"

/**
 * @class Window
 * @brief Abstracci�n de una ventana de aplicaci�n Win32 utilizada por el motor gr�fico.
 * @details
 *  Esta clase encapsula la creaci�n, manejo y destrucci�n de una ventana en sistemas Windows.
 *  Sirve como contenedor de bajo nivel sobre la API Win32, proporcionando un punto de entrada
 *  para el renderizado de gr�ficos y la interacci�n con el sistema operativo.
 *
 *  Entre sus funciones principales se incluyen:
 *   - Creaci�n e inicializaci�n de la ventana.
 *   - Manejo del ciclo de vida (actualizaci�n, renderizado y cierre).
 *   - Exposici�n de informaci�n esencial (dimensiones, manejadores, etc.).
 */
class Window {
public:
  /**
   * @brief Constructor por defecto.
   * @details Inicializa la clase sin crear la ventana. Se requiere llamar a `init()` antes de su uso.
   */
  Window() = default;

  /**
   * @brief Destructor por defecto.
   * @note No destruye autom�ticamente la ventana del sistema operativo.
   *       Se recomienda llamar expl�citamente a `destroy()` para liberar recursos asociados.
   */
  ~Window() = default;

  /**
   * @brief Crea e inicializa la ventana principal de la aplicaci�n.
   * @param hInstance Identificador de la instancia de la aplicaci�n (proporcionado por WinMain).
   * @param nCmdShow  Comando que especifica c�mo debe mostrarse la ventana (ej.: `SW_SHOW`).
   * @param wndproc   Funci�n de callback que manejar� los mensajes de ventana (Win32 WNDPROC).
   * @return `S_OK` si la ventana fue creada con �xito; en caso contrario, un c�digo HRESULT con el error.
   * @pre Debe llamarse una sola vez durante la inicializaci�n del programa.
   * @post Si la creaci�n es exitosa, `m_hWnd` contendr� un handle v�lido a la ventana.
   */
  HRESULT init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

  /**
   * @brief Actualiza el estado de la ventana y procesa los mensajes del sistema.
   * @details
   *  Este m�todo debe llamarse en cada iteraci�n del bucle principal para que la ventana
   *  responda a eventos del sistema (entrada, redimensionamiento, cierre, etc.).
   */
  void update();

  /**
   * @brief Punto de entrada para operaciones de renderizado asociadas a la ventana.
   * @details
   *  No realiza dibujo directamente, pero puede servir como lugar para llamadas de
   *  preparaci�n del frame o sincronizaci�n con el swap chain.
   */
  void render();

  /**
   * @brief Libera los recursos asociados a la ventana y la destruye.
   * @details
   *  Cierra la ventana y limpia las estructuras internas. Este m�todo es seguro de llamar m�ltiples veces.
   * @post `m_hWnd` se establecer� en `nullptr` tras la destrucci�n.
   */
  void destroy();

public:
  /**
   * @brief Manejador de la ventana Win32.
   * @details Identificador �nico que representa la ventana creada. Se utiliza para interactuar
   *          con la API Win32 en operaciones posteriores (como mensajer�a o manipulaci�n de propiedades).
   */
  HWND m_hWnd = nullptr;

  /**
   * @brief Ancho actual de la ventana en p�xeles.
   * @details Se asigna tras la creaci�n y puede actualizarse si la ventana cambia de tama�o.
   */
  unsigned int m_width;

  /**
   * @brief Altura actual de la ventana en p�xeles.
   * @details Se asigna tras la creaci�n y puede variar durante la ejecuci�n.
   */
  unsigned int m_height;

private:
  /**
   * @brief Identificador de instancia de la aplicaci�n.
   * @details Proporcionado por el sistema operativo al iniciar la aplicaci�n.
   *          Se usa para registrar la clase de ventana y crearla.
   */
  HINSTANCE m_hInst = nullptr;

  /**
   * @brief Estructura que define el �rea cliente de la ventana.
   * @details Almacena coordenadas de posici�n y tama�o del �rea utilizable para renderizado.
   */
  RECT m_rect;

  /**
   * @brief Nombre por defecto de la ventana.
   * @details Se utiliza en el t�tulo de la barra superior al crear la ventana.
   *          Puede cambiarse para identificar diferentes ventanas del motor.
   */
  std::string m_windowName = "Miner Engine";
};
