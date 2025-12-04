#pragma once
#include "Prerequisites.h"
class DeviceContext;

/**
 * @class Component
 * @brief Interfaz polimórfica para módulos de comportamiento.
 *
 * Define el contrato de ciclo de vida que deben cumplir todos los elementos
 * funcionales agregados a una entidad. Obliga a la implementación de rutinas
 * de inicialización, simulación, dibujo y liberación de memoria.
 */
class
  Component {
public:
  /**
   * @brief Constructor base.
   */
  Component() = default;

  /**
   * @brief Constructor de inicialización por categoría.
   * @param type Identificador que clasifica la naturaleza del componente.
   */
  Component(const ComponentType type) : m_type(type) {}

  /**
   * @brief Destructor virtual para herencia segura.
   * * Garantiza que los destructores de las clases derivadas sean invocados correctamente
   * al eliminar el objeto a través de un puntero base.
   */
  virtual
    ~Component() = default;

  /**
   * @brief Rutina de configuración inicial.
   * * @pure Método abstracto que debe preparar el estado interno del componente
   * antes de entrar al bucle principal.
   */
  virtual void
    init() = 0;

  /**
   * @brief Ejecución de lógica de simulación.
   * * @pure Método abstracto invocado en cada iteración del bucle de juego para
   * procesar cambios dependientes del tiempo.
   * @param deltaTime Intervalo de tiempo en segundos desde el fotograma anterior.
   */
  virtual void
    update(float deltaTime) = 0;

  /**
   * @brief Interacción con el pipeline gráfico.
   * * @pure Método abstracto encargado de enviar comandos de dibujo o actualizar
   * estados en la GPU si el componente tiene representación visual.
   * @param deviceContext Referencia a la interfaz de comandos del dispositivo gráfico.
   */
  virtual void
    render(DeviceContext& deviceContext) = 0;

  /**
   * @brief Protocolo de finalización.
   * * @pure Método abstracto para la liberación manual de recursos o desconexión
   * de eventos antes de la destrucción del objeto.
   */
  virtual void
    destroy() = 0;

  /**
   * @brief Accesor de identidad del componente.
   * @return El enumerador que define a qué familia de componentes pertenece esta instancia.
   */
  ComponentType
    getType() const { return m_type; }
protected:
  ComponentType m_type; 
};