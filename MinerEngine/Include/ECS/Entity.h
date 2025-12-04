#pragma once
#include "Prerequisites.h"
#include "Component.h"

class DeviceContext;

/**
 * @class Entity
 * @brief Arquitectura base para cualquier objeto interactivo en el motor.
 *
 * Actúa como un contenedor centralizado que agrupa lógica y datos a través de
 * la composición de componentes. Gestiona el ciclo de vida y la propagación
 * de eventos (actualización/renderizado) a sus módulos adjuntos.
 */
class
  Entity {
public:
  /**
   * @brief Constructor predeterminado.
   */
  Entity() = default;

  /**
   * @brief Destructor virtual para polimorfismo seguro.
   */
  virtual
    ~Entity() = default;

  /**
   * @brief Rutina de arranque.
   * * Método abstracto obligatorio para configurar el estado inicial de la entidad
   * antes de su inserción en el bucle principal.
   */
  virtual void
    init() = 0;

  /**
   * @brief Ciclo de simulación lógica.
   * * Define el comportamiento de la entidad en cada tick del reloj.
   * @param deltaTime Diferencia de tiempo (en segundos) respecto al frame anterior.
   * @param deviceContext Acceso al contexto gráfico si se requiere manipulación de recursos.
   */
  virtual void
    update(float deltaTime, DeviceContext& deviceContext) = 0;

  /**
   * @brief Ciclo de presentación visual.
   * * Encargado de enviar las instrucciones de dibujo al pipeline gráfico.
   * @param deviceContext Interfaz para la emisión de comandos de renderizado.
   */
  virtual void
    render(DeviceContext& deviceContext) = 0;

  /**
   * @brief Protocolo de finalización.
   * * Fuerza la limpieza de memoria y desconexión de recursos antes de la eliminación del objeto.
   */
  virtual void
    destroy() = 0;

  /**
   * @brief Vincula un nuevo módulo funcional a la entidad.
   * * Realiza una verificación en tiempo de compilación para asegurar que el tipo T
   * deriva de la clase base Component.
   * * @tparam T Clase concreta del componente a adjuntar.
   * @param component Puntero inteligente compartido al componente instanciado.
   */
  template <typename T> void
    addComponent(EU::TSharedPointer<T> component) {
    static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
    m_components.push_back(component.template dynamic_pointer_cast<Component>());
  }

  /**
   * @brief Recupera una referencia a un módulo específico.
   * * Busca dentro de la lista de componentes adjuntos y realiza un cast dinámico
   * para devolver el tipo solicitado.
   * * @tparam T Tipo de dato del componente buscado.
   * @return Puntero compartido al componente si existe; puntero nulo en caso contrario.
   */
  template<typename T>
  EU::TSharedPointer<T>
    getComponent() {
    for (auto& component : m_components) {
      EU::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
      if (specificComponent) {
        return specificComponent;
      }
    }
    return EU::TSharedPointer<T>();
  }
private:
protected:
  bool m_isActive;                                       
  int m_id;                                               
  std::vector<EU::TSharedPointer<Component>> m_components;
};