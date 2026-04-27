#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

/**
 * @class RenderScene
 * @brief Estructura de datos que contiene todos los elementos visibles de la escena para un frame.
 * * RenderScene actúa como un "escenario" temporal. Durante el ciclo de actualización, el motor
 * recolecta objetos y luces, los clasifica y los almacena aquí para que el Pipeline de Renderizado
 * pueda dibujarlos de manera eficiente (por ejemplo, manejando el orden de transparencia).
 */
class RenderScene {
public:
    /**
     * @brief Limpia todos los contenedores de la escena.
     * * Se debe llamar al inicio de cada frame para descartar los datos del frame anterior
     * y prepararse para una nueva recolección de objetos.
     */
    void clear();

public:
    /** @name Listas de Renderizado */
    ///@{

    /** * @brief Objetos opacos que se renderizan primero.
     * * Generalmente se dibujan de adelante hacia atrás para aprovechar el Early-Z testing.
     */
    std::vector<RenderObject> opaqueObjects;

    /** * @brief Objetos con transparencia que se renderizan después de los opacos.
     * * Deben ordenarse de atrás hacia adelante para un mezclado (Blending) correcto.
     */
    std::vector<RenderObject> transparentObjects;
    ///@}

    /** @name Iluminación y Entorno */
    ///@{

    /** @brief Lista de luces direccionales activas en la escena. */
    std::vector<LightData> directionalLights;

    /** * @brief Puntero al Skybox (cubemap) de la escena.
     * * Si es nulo, se utilizará un color de fondo sólido definido por el motor.
     */
    Skybox* skybox = nullptr;
    ///@}
};