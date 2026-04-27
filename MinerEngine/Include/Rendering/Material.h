#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class ShaderProgram;
class RasterizerState;
class DepthStencilState;
class SamplerState;

/**
 * @class Material
 * @brief Clase que encapsula el estado de renderizado y el pipeline de sombreado para un objeto.
 * * La clase Material actúa como un contenedor que vincula un ShaderProgram con sus estados
 * correspondientes de rasterización, profundidad y muestreo, además de definir el dominio y modo de mezcla.
 */
class Material {
public:
    /** @name Setters */
    ///@{

    /**
     * @brief Asigna el programa de sombreado (shader) al material.
     * @param shader Puntero al ShaderProgram que se utilizará para el renderizado.
     */
    void setShader(ShaderProgram* shader) { m_shader = shader; }

    /**
     * @brief Establece el estado del rasterizador.
     * @param state Puntero al RasterizerState (Cull mode, Fill mode, etc.).
     */
    void setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }

    /**
     * @brief Establece el estado de profundidad y stencil.
     * @param state Puntero al DepthStencilState que define pruebas de profundidad y máscaras.
     */
    void setDepthStencilState(DepthStencilState* state) { m_depthStencilState = state; }

    /**
     * @brief Establece el estado del sampler de texturas.
     * @param state Puntero al SamplerState para el filtrado y direccionamiento de texturas.
     */
    void setSamplerState(SamplerState* state) { m_samplerState = state; }

    /**
     * @brief Define el dominio del material (e.g., Superficie, Post-procesado).
     * @param domain El dominio de material de la enumeración MaterialDomain.
     */
    void setDomain(MaterialDomain domain) { m_domain = domain; }

    /**
     * @brief Establece el modo de mezcla (Blending) del material.
     * @param blendMode El modo de mezcla (Opaque, Masked, Translucent).
     */
    void setBlendMode(BlendMode blendMode) { m_blendMode = blendMode; }
    ///@}

    /** @name Getters */
    ///@{

    /** @brief Obtiene el programa de sombreado actual. @return Puntero al ShaderProgram. */
    ShaderProgram* getShader() const { return m_shader; }

    /** @brief Obtiene el estado del rasterizador. @return Puntero al RasterizerState. */
    RasterizerState* getRasterizerState() const { return m_rasterizerState; }

    /** @brief Obtiene el estado de profundidad y stencil. @return Puntero al DepthStencilState. */
    DepthStencilState* getDepthStencilState() const { return m_depthStencilState; }

    /** @brief Obtiene el estado del sampler. @return Puntero al SamplerState. */
    SamplerState* getSamplerState() const { return m_samplerState; }

    /** @brief Obtiene el dominio del material. @return El MaterialDomain actual. */
    MaterialDomain getDomain() const { return m_domain; }

    /** @brief Obtiene el modo de mezcla. @return El BlendMode actual. */
    BlendMode getBlendMode() const { return m_blendMode; }
    ///@}

private:
    /// Puntero al programa de sombreado vinculado.
    ShaderProgram* m_shader = nullptr;

    /// Estado que define cómo se rasterizan las primitivas.
    RasterizerState* m_rasterizerState = nullptr;

    /// Estado que controla las pruebas de profundidad (Z-buffer).
    DepthStencilState* m_depthStencilState = nullptr;

    /// Estado que define cómo se leen las texturas en el shader.
    SamplerState* m_samplerState = nullptr;

    /// Clasificación del material dentro del pipeline.
    MaterialDomain m_domain = MaterialDomain::Opaque;

    /// Modo de mezcla utilizado para el renderizado de este material.
    BlendMode m_blendMode = BlendMode::Opaque;
};