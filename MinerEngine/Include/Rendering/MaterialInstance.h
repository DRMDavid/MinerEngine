#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Material;
class DeviceContext;
class Texture;

/**
 * @class MaterialInstance
 * @brief Representa una instancia específica de un Material con recursos de texturas y parámetros únicos.
 * * La instancia permite reutilizar la lógica de un Material base (Shaders, Render States)
 * aplicando diferentes mapas de texturas (PBR) y constantes (MaterialParams) para cada objeto.
 */
class MaterialInstance {
public:
    /** @name Gestión de Material Base */
    ///@{

    /**
     * @brief Asigna el material base que define el pipeline de renderizado.
     * @param material Puntero al Material base.
     */
    void setMaterial(Material* material) { m_material = material; }

    /** @brief Obtiene el material base vinculado. @return Puntero al Material. */
    Material* getMaterial() const { return m_material; }
    ///@}

    /** @name Configuración de Mapas PBR */
    ///@{

    /** @brief Asigna el mapa de Albedo (Color base). @param texture Puntero a la textura. */
    void setAlbedo(Texture* texture) { m_albedo = texture; }

    /** @brief Asigna el mapa de Normales. @param texture Puntero a la textura de normales. */
    void setNormal(Texture* texture) { m_normal = texture; }

    /** @brief Asigna el mapa de Metalicidad. @param texture Puntero a la textura de metalicidad. */
    void setMetallic(Texture* texture) { m_metallic = texture; }

    /** @brief Asigna el mapa de Rugosidad. @param texture Puntero a la textura de roughness. */
    void setRoughness(Texture* texture) { m_roughness = texture; }

    /** @brief Asigna el mapa de Oclusión Ambiental (AO). @param texture Puntero a la textura de AO. */
    void setAO(Texture* texture) { m_ao = texture; }

    /** @brief Asigna el mapa de Emisión. @param texture Puntero a la textura emissive. */
    void setEmissive(Texture* texture) { m_emissive = texture; }
    ///@}

    /** @name Getters de Texturas */
    ///@{
    Texture* getAlbedo() const { return m_albedo; }
    Texture* getNormal() const { return m_normal; }
    Texture* getMetallic() const { return m_metallic; }
    Texture* getRoughness() const { return m_roughness; }
    Texture* getAO() const { return m_ao; }
    Texture* getEmissive() const { return m_emissive; }
    ///@}

    /** @name Gestión de Parámetros */
    ///@{

    /**
     * @brief Accede a los parámetros numéricos del material (escalares, colores, etc.).
     * @return Referencia a la estructura MaterialParams.
     */
    MaterialParams& getParams() { return m_params; }

    /**
     * @brief Accede a los parámetros numéricos del material (versión constante).
     * @return Referencia constante a la estructura MaterialParams.
     */
    const MaterialParams& getParams() const { return m_params; }
    ///@}

    /**
     * @brief Vincula todas las texturas activas al pipeline de la GPU.
     * @param deviceContext Contexto del dispositivo encargado de las llamadas de dibujo (DirectX/Vulkan).
     */
    void bindTextures(DeviceContext& deviceContext) const;

private:
    /// Puntero al material maestro del cual depende esta instancia.
    Material* m_material = nullptr;

    /** @name Recursos de Texturas (PBR Workflow) */
    ///@{
    Texture* m_albedo = nullptr;    ///< Mapa de color base o difuso.
    Texture* m_normal = nullptr;    ///< Mapa para detalles de superficie (Normal mapping).
    Texture* m_metallic = nullptr;  ///< Define qué áreas son metálicas.
    Texture* m_roughness = nullptr; ///< Define la micro-rugosidad de la superficie.
    Texture* m_ao = nullptr;        ///< Mapa de Ambient Occlusion para sombras suaves.
    Texture* m_emissive = nullptr;  ///< Mapa para superficies que emiten luz.
    ///@}

    /// Estructura que contiene valores constantes (vectores, floats) para el shader.
    MaterialParams m_params;
};