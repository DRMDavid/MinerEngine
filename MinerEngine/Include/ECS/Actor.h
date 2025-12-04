#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h"
#include "SamplerState.h"
#include "ShaderProgram.h"


class Device;
class DeviceContext;
class MeshComponent;

/**
 * @class Actor
 * @brief Clase fundamental para objetos renderizables en el entorno 3D.
 *
 * Esta clase especializa a una Entidad para dotarla de capacidad visual. Gestiona
 * la geometría (mallas), apariencia (texturas) y los recursos de la API gráfica
 * (buffers, shaders) necesarios para su representación en pantalla y la proyección de sombras.
 */
class
  Actor : public Entity {
public:
  /**
   * @brief Instanciación básica del objeto.
   */
  Actor() = default;

  /**
   * @brief Crea un Actor y lo vincula a un contexto de dispositivo gráfico.
   * @param device Referencia al dispositivo encargado de la creación de recursos.
   */
  Actor(Device& device);

  /**
   * @brief Destructor virtual.
   */
  virtual
    ~Actor() = default;

  /**
   * @brief Configuración inicial del objeto.
   * * Implementación de la interfaz de Entity para preparar el estado inicial
   * del actor antes de su uso.
   */
  void
    init() override {}

  /**
   * @brief Ciclo de actualización lógica por fotograma.
   * * @param deltaTime Lapso de tiempo transcurrido desde el último frame.
   * @param deviceContext Contexto para realizar operaciones gráficas durante la actualización.
   * * @note Utilice este método para cálculos de movimiento o lógica de juego pre-renderizado.
   */
  void
    update(float deltaTime, DeviceContext& deviceContext) override;

  /**
   * @brief Envía la geometría del actor al pipeline de renderizado.
   * * Prepara los buffers, vincula los shaders correspondientes y emite
   * las llamadas de dibujo para visualizar el objeto en la escena.
   * * @param deviceContext Contexto utilizado para los comandos de dibujo.
   */
  void
    render(DeviceContext& deviceContext) override;

  /**
   * @brief Limpieza de recursos gráficos.
   * * Se encarga de liberar la memoria de buffers, texturas y estados asociados
   * para evitar fugas de memoria en la GPU.
   */
  void
    destroy();

  /**
   * @brief Asigna la geometría que definirá la forma del actor.
   * * Genera internamente los Vertex e Index Buffers necesarios basándose en
   * los datos de las mallas proporcionadas.
   * * @param device Dispositivo necesario para crear los buffers en GPU.
   * @param meshes Lista de componentes de malla con la información geométrica.
   */
  void
    setMesh(Device& device, std::vector<MeshComponent> meshes);

  /**
   * @brief Recupera el identificador textual del actor.
   * @return Cadena con el nombre asignado.
   */
  std::string
    getName() { return m_name; }

  /**
   * @brief Asigna un identificador textual al actor.
   * @param name Nuevo nombre para identificar la instancia.
   */
  void
    setName(const std::string& name) { m_name = name; }

  /**
   * @brief Carga la lista de materiales o imágenes para el actor.
   * @param textures Colección de texturas a aplicar sobre la malla.
   */
  void
    setTextures(std::vector<Texture> textures) { m_textures = textures; }

  /**
   * @brief Habilita o deshabilita la capacidad de generar sombras.
   * @param v True para activar la proyección de sombras, False para desactivarla.
   */
  void
    setCastShadow(bool v) { castShadow = v; }

  /**
   * @brief Consulta si el actor está configurado para ocluir luz.
   * @return Estado actual de la bandera de proyección de sombras.
   */
  bool
    canCastShadow() const { return castShadow; }

  /**
   * @brief Ejecuta el pase de renderizado para el mapa de sombras.
   * * Utiliza una configuración de shader simplificada para dibujar la profundidad
   * del objeto desde la perspectiva de la luz.
   * * @param deviceContext Contexto gráfico para el dibujo en el shadow map.
   */
  void
    renderShadow(DeviceContext& deviceContext);

private:
  std::vector<MeshComponent> m_meshes;    ///< Contenedor de la geometría del modelo.
  std::vector<Texture> m_textures;        ///< Recursos de imagen aplicados al modelo.
  std::vector<Buffer> m_vertexBuffers;    ///< Almacenamiento en GPU para vértices.
  std::vector<Buffer> m_indexBuffers;     ///< Almacenamiento en GPU para índices.


  SamplerState m_sampler;                 ///< Objeto para el filtrado de texturas.
  CBChangesEveryFrame m_model;            ///< Estructura de datos para constantes por frame.
  Buffer m_modelBuffer;                   ///< Buffer constante (CB) para la matriz de modelo.

  // Recursos para sombras
  ShaderProgram m_shaderShadow;           ///< Programa GLSL/HLSL para el pase de sombras.
  Buffer m_shaderBuffer;                  ///< Buffer auxiliar para parámetros del shader.
  
  CBChangesEveryFrame m_cbShadow;         ///< Constantes específicas para el cálculo de sombras.

  XMFLOAT4 m_LightPos;                    ///< Coordenadas de la fuente de luz principal.
  std::string m_name = "Actor";           ///< Etiqueta de depuración o identificación.
  bool castShadow = true;                 ///< Bandera de control para lógica de sombras.
};