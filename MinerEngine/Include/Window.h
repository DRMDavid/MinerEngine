#pragma once
#include "Prerequisites.h"

/**
 * @class Window
 * @brief Abstracción de una ventana de aplicación Win32 utilizada por el motor gráfico.
 * @details
 *  Esta clase encapsula la creación, manejo y destrucción de una ventana en sistemas Windows.
 *  Sirve como contenedor de bajo nivel sobre la API Win32, proporcionando un punto de entrada
 *  para el renderizado de gráficos y la interacción con el sistema operativo.
 *
 *  Entre sus funciones principales se incluyen:
 *   - Creación e inicialización de la ventana.
 *   - Manejo del ciclo de vida (actualización, renderizado y cierre).
 *   - Exposición de información esencial (dimensiones, manejadores, etc.).
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
   * @note No destruye automáticamente la ventana del sistema operativo.
   *       Se recomienda llamar explícitamente a `destroy()` para liberar recursos asociados.
   */
  ~Window() = default;

  /**
   * @brief Crea e inicializa la ventana principal de la aplicación.
   * @param hInstance Identificador de la instancia de la aplicación (proporcionado por WinMain).
   * @param nCmdShow  Comando que especifica cómo debe mostrarse la ventana (ej.: `SW_SHOW`).
   * @param wndproc   Función de callback que manejará los mensajes de ventana (Win32 WNDPROC).
   * @return `S_OK` si la ventana fue creada con éxito; en caso contrario, un código HRESULT con el error.
   * @pre Debe llamarse una sola vez durante la inicialización del programa.
   * @post Si la creación es exitosa, `m_hWnd` contendrá un handle válido a la ventana.
   */
  HRESULT init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

  /**
   * @brief Actualiza el estado de la ventana y procesa los mensajes del sistema.
   * @details
   *  Este método debe llamarse en cada iteración del bucle principal para que la ventana
   *  responda a eventos del sistema (entrada, redimensionamiento, cierre, etc.).
   */
  void update();

  /**
   * @brief Punto de entrada para operaciones de renderizado asociadas a la ventana.
   * @details
   *  No realiza dibujo directamente, pero puede servir como lugar para llamadas de
   *  preparación del frame o sincronización con el swap chain.
   */
  void render();

  /**
   * @brief Libera los recursos asociados a la ventana y la destruye.
   * @details
   *  Cierra la ventana y limpia las estructuras internas. Este método es seguro de llamar múltiples veces.
   * @post `m_hWnd` se establecerá en `nullptr` tras la destrucción.
   */
  void destroy();

public:
  /**
   * @brief Manejador de la ventana Win32.
   * @details Identificador único que representa la ventana creada. Se utiliza para interactuar
   *          con la API Win32 en operaciones posteriores (como mensajería o manipulación de propiedades).
   */
  HWND m_hWnd = nullptr;

  /**
   * @brief Ancho actual de la ventana en píxeles.
   * @details Se asigna tras la creación y puede actualizarse si la ventana cambia de tamaño.
   */
  unsigned int m_width;

  /**
   * @brief Altura actual de la ventana en píxeles.
   * @details Se asigna tras la creación y puede variar durante la ejecución.
   */
  unsigned int m_height;

private:
  /**
   * @brief Identificador de instancia de la aplicación.
   * @details Proporcionado por el sistema operativo al iniciar la aplicación.
   *          Se usa para registrar la clase de ventana y crearla.
   */
  HINSTANCE m_hInst = nullptr;

  /**
   * @brief Estructura que define el área cliente de la ventana.
   * @details Almacena coordenadas de posición y tamaño del área utilizable para renderizado.
   */
  RECT m_rect;

  /**
   * @brief Nombre por defecto de la ventana.
   * @details Se utiliza en el título de la barra superior al crear la ventana.
   *          Puede cambiarse para identificar diferentes ventanas del motor.
   */
  std::string m_windowName = "Miner Engine";
};
