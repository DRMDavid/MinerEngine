#pragma once
#include "Prerequisites.h"
#include "ECS\Component.h"
class DeviceContext;

/**
 * @class MeshComponent
 * @brief Contenedor de topología geométrica para el pipeline de renderizado.
 *
 * Esta clase actúa como el almacenamiento principal de los datos crudos (raw data)
 * que definen la forma tridimensional de una entidad. Almacena los buffers de
 * atributos de vértices y la información de conectividad (índices) necesaria
 * para que la GPU ensamble las primitivas.
 */
class
  MeshComponent : public Component {
public:
  /**
   * @brief Instanciación de contenedor vacío.
   *
   * Configura el componente con contadores a cero y establece su identidad
   * dentro del sistema ECS bajo la categoría de Malla (MESH).
   */
  MeshComponent() : m_numVertex(0), m_numIndex(0), Component(ComponentType::MESH) {}

  /**
   * @brief Destructor virtual.
   */
  virtual
    ~MeshComponent() = default;

  /**
   * @brief Rutina de inicialización diferida.
   * * @note Implementación vacía. Reservado para la carga asíncrona de recursos
   * o generación procedimental post-construcción.
   */
  void
    init() override {};

  /**
   * @brief Hook de actualización por fotograma.
   * * @note Implementación vacía. Puede utilizarse para animaciones de malla por CPU,
   * morph targets o deformaciones dinámicas antes del envío a GPU.
   * @param deltaTime Tiempo delta para interpolaciones.
   */
  void
    update(float deltaTime) override {};

  /**
   * @brief Hook de envío al pipeline.
   * * @note Implementación vacía. Generalmente, el sistema de renderizado (Renderer System)
   * accede a los datos públicos de esta clase en lugar de que el componente se dibuje a sí mismo.
   * @param deviceContext Interfaz de comandos gráficos.
   */
  void
    render(DeviceContext& deviceContext) override {};

  /**
   * @brief Liberación de memoria.
   * * @note Implementación vacía. La memoria de los vectores STL se gestiona automáticamente,
   * a menos que se integre gestión manual de VRAM.
   */
  void
    destroy() override {};

public:
  /**
   * @brief Identificador de recurso o etiqueta de depuración.
   */
  std::string m_name;

  /**
   * @brief Buffer de atributos de vértices (VBO Data).
   * * Contiene la información por punto (Posición, Normal, UV, etc.) requerida
   * por el Vertex Shader.
   */
  std::vector<SimpleVertex> m_vertex;

  /**
   * @brief Buffer de ordenamiento de índices (IBO Data).
   * * Define la secuencia en la que los vértices se conectan para formar
   * triángulos o líneas, permitiendo la reutilización de vértices.
   */
  std::vector<unsigned int> m_index;

  /**
   * @brief Cantidad de elementos en el buffer de vértices.
   * * Utilizado para llamadas de dibujo (DrawCalls).
   */
  int m_numVertex;

  /**
   * @brief Cantidad de elementos en el buffer de índices.
   * * Determina el conteo para llamadas de dibujo indexadas (DrawIndexed).
   */
  int m_numIndex;
};