#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"
#include "Rendering/RenderTypes.h"

// Forward declaration
class DeviceContext;

/**
 * @class LightComponent
 * @brief Componente del sistema ECS (Entity-Component-System) que dota a un actor de las propiedades de una fuente de luz.
 */
class LightComponent : public Component {
public:
    /**
     * @brief Constructor por defecto. Inicializa el componente.
     */
    LightComponent()
        : Component(ComponentType::NONE) {
    }

    /**
     * @brief Inicializa los recursos o estados del componente.
     */
    void init() override {}

    /**
     * @brief Actualiza la lógica del componente.
     * @param deltaTime Tiempo transcurrido desde el último fotograma.
     */
    void update(float deltaTime) override {}

    /**
     * @brief Renderiza información o elementos visuales de depuración asociados al componente.
     * @param deviceContext Contexto del dispositivo gráfico utilizado para emitir comandos de dibujado.
     */
    void render(DeviceContext& deviceContext) override {}

    /**
     * @brief Libera los recursos asignados por el componente antes de su destrucción.
     */
    void destroy() override {}

    /**
     * @brief Obtiene una referencia modificable a los datos estructurales de la luz.
     * @return Referencia a la estructura LightData.
     */
    LightData& getLightData() { return m_light; }

    /**
     * @brief Obtiene una referencia de solo lectura a los datos estructurales de la luz.
     * @return Referencia constante a la estructura LightData.
     */
    const LightData& getLightData() const { return m_light; }

    /**
     * @brief Activa o desactiva la proyección de sombras para esta luz.
     * @param value true para habilitar la proyección de sombras, false para deshabilitarla.
     */
    void setCastShadow(bool value) { m_castShadow = value; }

    /**
     * @brief Verifica si la luz está configurada para proyectar sombras.
     * @return true si la luz proyecta sombras, false en caso contrario.
     */
    bool canCastShadow() const { return m_castShadow; }

private:
    LightData m_light;          ///< Estructura que contiene los parámetros principales de la luz (color, intensidad, tipo, etc.).
    bool m_castShadow = false;  ///< Bandera que indica si esta luz debe contribuir a la generación de mapas de sombras.
};