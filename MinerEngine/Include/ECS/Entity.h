#pragma once
#include "Prerequisites.h"
#include "Component.h"

class DeviceContext;

class
    Entity {
public:
<<<<<<< Updated upstream
  Entity() = default;

  /**
=======
    Entity() = default;

    /**
>>>>>>> Stashed changes
   * @brief Destructor virtual.
   */
    virtual
        ~Entity() = default;

<<<<<<< Updated upstream
  /**
   * @brief Initialize the entity with a device context.
   * @param deviceContext The device context to initialize with.
   * @return True if initialization is successful, false otherwise.
   */
  virtual void
    init() = 0;

  /**
   * @brief Método virtual puro para actualizar el componente.
   * @param deltaTime El tiempo transcurrido desde la última actualización.
   */
  virtual void
    update(float deltaTime, DeviceContext& deviceContext) = 0;

  /**
   * @brief Método virtual puro para renderizar el componente.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  virtual void
    render(DeviceContext& deviceContext) = 0;

  /**
   * @brief Método virtual puro para destruir el componente.
   * Libera los recursos asociados al componente.
   */
  virtual void
    destroy() = 0;

  /**
   * @brief Agrega un componente a la entidad.
   * @tparam T Tipo del componente, debe derivar de Component.
   * @param component Puntero compartido al componente que se va a agregar.
   */
  template <typename T> void
    addComponent(EU::TSharedPointer<T> component) {
    static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
    m_components.push_back(component.template dynamic_pointer_cast<Component>());
  }

  /**
   * @brief Obtiene un componente de la entidad por su tipo.
   * @tparam T Tipo del componente a obtener.
   * @return Puntero compartido al componente si se encuentra, nullptr en caso contrario.
   */
  template<typename T>
  EU::TSharedPointer<T>
    getComponent() {
    for (auto& component : m_components) {
      EU::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
      if (specificComponent) {
        return specificComponent;
      }
=======
    virtual void
        awake() = 0;

    /**
     * @brief Initialize the entity with a device context.
     * @param deviceContext The device context to initialize with.
     * @return True if initialization is successful, false otherwise.
       */
    virtual void
        init() = 0;

    /**
     * @brief Método virtual puro para actualizar el componente.
     * @param deltaTime El tiempo transcurrido desde la última actualización.
     */
    virtual void
        update(float deltaTime, DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para renderizar el componente.
     * @param deviceContext Contexto del dispositivo para operaciones gráficas.
     */
    virtual void
        render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para destruir el componente.
     * Libera los recursos asociados al componente.
       */
    virtual void
        destroy() = 0;

    /**
     * @brief Agrega un componente a la entidad.
     * @tparam T Tipo del componente, debe derivar de Component.
     * @param component Puntero compartido al componente que se va a agregar.
     */
    template <typename T> void
        addComponent(EU::TSharedPointer<T> component) {
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
        m_components.push_back(component.template dynamic_pointer_cast<Component>());
    }

    /**
     * @brief Obtiene un componente de la entidad por su tipo.
     * @tparam T Tipo del componente a obtener.
     * @return Puntero compartido al componente si se encuentra, nullptr en caso contrario.
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
>>>>>>> Stashed changes
    }
private:
protected:
<<<<<<< Updated upstream
  bool m_isActive;
  int m_id;
  std::vector<EU::TSharedPointer<Component>> m_components;
=======
    bool m_isActive;
    int m_id;
    std::vector<EU::TSharedPointer<Component>> m_components;
>>>>>>> Stashed changes
};