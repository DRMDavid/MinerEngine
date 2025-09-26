#pragma once
#include "Prerequisites.h"

// Declaraciones adelantadas
class Device;
class DeviceContext;

/**
 * @class Texture
 * @brief Clase encargada de la creación, manejo y uso de texturas en Direct3D 11.
 * @details
 *  Esta clase abstrae las operaciones más comunes relacionadas con recursos de textura:
 *  - Carga desde archivos de imagen.
 *  - Creación dinámica en memoria (RTT, G-Buffer, etc.).
 *  - Creación a partir de otras texturas existentes.
 *
 *  También gestiona la creación de vistas de recursos asociadas y su liberación segura.
 */
class Texture {
public:
  /**
   * @brief Constructor por defecto.
   * @details Inicializa la instancia sin asignar recursos. Es necesario llamar a `init()` para su uso.
   */
  Texture() = default;

  /**
   * @brief Destructor por defecto.
   * @note Los recursos Direct3D no se liberan automáticamente. Es recomendable llamar a `destroy()`
   *       antes de destruir el objeto para evitar fugas de memoria.
   */
  ~Texture() = default;

  /**
   * @brief Carga una textura desde un archivo de imagen.
   * @param device        Dispositivo Direct3D encargado de la creación del recurso.
   * @param textureName   Ruta o nombre del archivo de textura.
   * @param extensionType Tipo de formato de la imagen (por ejemplo, DDS, PNG o JPG).
   * @return `S_OK` si la textura se cargó correctamente, o un código HRESULT en caso de error.
   * @pre El archivo debe existir y ser accesible desde la ruta especificada.
   * @post `m_texture` y `m_textureFromImg` apuntarán a recursos válidos si la operación fue exitosa.
   */
  HRESULT init(Device& device,
    const std::string& textureName,
    ExtensionType extensionType);

  /**
   * @brief Crea una textura 2D en memoria sin depender de archivos externos.
   * @param device        Dispositivo Direct3D responsable de la creación.
   * @param width         Ancho de la textura en píxeles.
   * @param height        Alto de la textura en píxeles.
   * @param Format        Formato de la textura (por ejemplo, `DXGI_FORMAT_R8G8B8A8_UNORM`).
   * @param BindFlags     Indicadores de uso (shader resource, render target, depth stencil, etc.).
   * @param sampleCount   Número de muestras para MSAA (por defecto 1).
   * @param qualityLevels Niveles de calidad para el multisampling (por defecto 0).
   * @return `S_OK` si la creación fue exitosa; en caso contrario, un código HRESULT de error.
   * @note Este método se usa comúnmente para texturas renderizables o buffers auxiliares.
   */
  HRESULT init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT Format,
    unsigned int BindFlags,
    unsigned int sampleCount = 1,
    unsigned int qualityLevels = 0);

  /**
   * @brief Crea una nueva textura copiando la configuración de otra existente.
   * @param device     Dispositivo Direct3D para la creación.
   * @param textureRef Textura fuente de la cual se tomarán parámetros.
   * @param format     Formato del nuevo recurso.
   * @return `S_OK` si se crea correctamente; en caso contrario, HRESULT con el error.
   * @details
   *  Esta variante es útil para generar texturas derivadas (por ejemplo, para post-procesamiento
   *  o versiones con distinto formato a partir de un mismo recurso base).
   */
  HRESULT init(Device& device, Texture& textureRef, DXGI_FORMAT format);

  /**
   * @brief Punto de extensión para actualizar el estado interno de la textura.
   * @details Actualmente no tiene implementación, pero puede ser utilizado en el futuro
   *          para actualizar datos o regenerar el recurso dinámicamente.
   */
  void update();

  /**
   * @brief Vincula la textura al pipeline gráfico como recurso de shader.
   * @param deviceContext Contexto del dispositivo donde se establecerá el recurso.
   * @param StartSlot     Slot inicial en el que se asignará la vista de textura.
   * @param NumViews      Número de vistas consecutivas a enlazar.
   * @pre La textura debe haberse inicializado correctamente con `init()`.
   * @note Suele llamarse antes de ejecutar un shader que consuma esta textura.
   */
  void render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews);

  /**
   * @brief Libera todos los recursos asociados a la textura.
   * @details
   *  Destruye tanto el recurso base (`ID3D11Texture2D`) como su vista de recurso (`ID3D11ShaderResourceView`).
   *  El método es seguro de llamar múltiples veces.
   * @post Los punteros internos se establecerán en `nullptr`.
   */
  void destroy();

public:
  /**
   * @brief Puntero COM al recurso de textura 2D en GPU.
   * @details Representa el objeto real en memoria gráfica. Es válido tras una inicialización exitosa.
   */
  ID3D11Texture2D* m_texture = nullptr;

  /**
   * @brief Vista de recurso asociada a la textura.
   * @details Permite que los shaders accedan a la textura durante el pipeline gráfico.
   */
  ID3D11ShaderResourceView* m_textureFromImg = nullptr;

  /**
   * @brief Nombre o ruta del recurso de textura.
   * @details Almacena el identificador de la textura original cargada desde disco.
   */
  std::string m_textureName;
};
