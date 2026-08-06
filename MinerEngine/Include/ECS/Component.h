#pragma once
#include "Prerequisites.h"

class DeviceContext;

/**
 * @class Component
 * @brief Clase base abstracta para todos los componentes del motor.
 *
 * Todos los componentes del ECS (Entity Component System) heredan de esta
 * clase base. Define la interfaz común para inicialización, actualización,
 * renderizado y destrucción de recursos.
 *
 * Además, incorpora un estado de activación que permite habilitar o
 * deshabilitar cualquier componente desde el editor sin eliminarlo del actor.
 */
class Component {
public:

	/**
	 * @brief Constructor por defecto.
	 */
	Component() = default;

	/**
	 * @brief Constructor que inicializa el tipo del componente.
	 * @param type Tipo del componente dentro del ECS.
	 */
	Component(const ComponentType type)
		: m_type(type) {
	}

	/**
	 * @brief Destructor virtual.
	 */
	virtual
		~Component() = default;

	/**
	 * @brief Inicializa el componente.
	 */
	virtual void
		init() = 0;

	/**
	 * @brief Actualiza la lógica del componente.
	 * @param deltaTime Tiempo transcurrido desde el último frame.
	 */
	virtual void
		update(float deltaTime) = 0;

	/**
	 * @brief Renderiza el componente.
	 * @param deviceContext Contexto del dispositivo gráfico.
	 */
	virtual void
		render(DeviceContext& deviceContext) = 0;

	/**
	 * @brief Libera los recursos del componente.
	 */
	virtual void
		destroy() = 0;

	/**
	 * @brief Obtiene el tipo del componente.
	 * @return Tipo del componente.
	 */
	ComponentType
		getType() const {
		return m_type;
	}

	/**
	 * @brief Indica si el componente está habilitado.
	 *
	 * Un componente deshabilitado permanece agregado al Actor,
	 * pero los sistemas del motor pueden ignorarlo durante su
	 * actualización.
	 *
	 * @return true si el componente está activo.
	 */
	bool
		isEnabled() const {
		return m_enabled;
	}

	/**
	 * @brief Activa o desactiva el componente.
	 *
	 * Esta función será utilizada desde el Inspector del editor
	 * mediante una casilla de verificación (checkbox).
	 *
	 * @param enabled Nuevo estado del componente.
	 */
	void
		setEnabled(bool enabled) {
		m_enabled = enabled;
	}

protected:

	/**
	 * @brief Tipo del componente.
	 */
	ComponentType m_type = ComponentType::NONE;

	/**
	 * @brief Estado de activación del componente.
	 *
	 * Si es false, el componente continúa existiendo dentro del
	 * Actor, pero los sistemas del motor (Física, Audio,
	 * Animación, etc.) pueden ignorarlo durante la actualización.
	 */
	bool m_enabled = true;
};