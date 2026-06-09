/**
 * @file RenderTypes.h
 * @brief Declara las estructuras, enums y constantes base dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/** @name Enumeraciones de Control de Pipeline */
///@{

/**
 * @enum MaterialDomain
 * @brief Define el dominio y el tratamiento de opacidad que tendrá el material.
 */
enum class MaterialDomain {
    Opaque = 0, ///< Material completamente sólido que bloquea la luz.
    Masked,     ///< Material perforado mediante texturas binarias (e.g., rejas, hojas) usando una prueba de descarte (Clip/Discard).
    Transparent ///< Material translúcido que requiere mezcla de color con el fondo (Alpha Blending).
};

/**
 * @enum BlendMode
 * @brief Modos de mezcla matemática de color para el rasterizador (Output Merger).
 */
enum class BlendMode {
    Opaque = 0,          ///< Sin mezcla. Reemplaza por completo el color del fondo.
    Alpha,               ///< Mezcla alfa tradicional (interpolación lineal basada en el canal Alpha).
    Additive,            ///< Suma de colores. Ideal para efectos como fuego, partículas de energía o láseres.
    PremultipliedAlpha   ///< Mezcla alfa con el canal de color pre-multiplicado por el alfa (evita artefactos de borde oscuros).
};

/**
 * @enum RenderPassType
 * @brief Identifica la fase o pasada de renderizado en la que se procesará la geometría.
 */
enum class RenderPassType {
    Shadow = 0, ///< Pasada de generación de mapas de sombras (Shadow Maps).
    Opaque,     ///< Pasada principal para objetos opacos.
    Skybox,     ///< Pasada exclusiva para dibujar el cielo/entorno.
    Transparent,///< Pasada de objetos translúcidos (ordenada por profundidad).
    Editor      ///< Pasada para renderizar gizmos, mallas de selección u otros elementos del motor.
};

/**
 * @enum LightType
 * @brief Define el modelo matemático del cálculo de iluminación para una fuente de luz.
 */
enum class LightType {
    Directional = 0, ///< Luz infinita sin posición, solo dirección (e.g., el Sol).
    Point,           ///< Luz omnidireccional puntual con radio de atenuación.
    Spot             ///< Luz en forma de cono proyectada desde un punto (e.g., una linterna).
};
///@}

/** @brief Límite máximo de luces concurrentes procesadas en los shaders por cada frame. */
constexpr int RMaxLights = 8;


/** @name Estructuras de Datos de Iluminación y Materiales */
///@{

/**
 * @struct LightData
 * @brief Contenedor de datos que define las propiedades físicas de una fuente de luz.
 */
struct LightData {
    LightType type = LightType::Directional;       ///< Tipo de iluminación.
    EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f); ///< Color de emisión de la luz (RGB).
    float intensity = 1.0f;                        ///< Factor de potencia lumínica.

    EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f); ///< Dirección del vector de luz.
    float range = 0.0f;                            ///< Radio máximo de alcance (inútil en direccionales).

    EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f); ///< Posición en el espacio tridimensional.
    float spotAngle = 0.0f;                        ///< Ángulo de apertura del cono de luz para fuentes tipo Spot.
};

/**
 * @struct MaterialParams
 * @brief Parámetros numéricos en la CPU para la configuración PBR del material.
 */
struct MaterialParams {
    XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); ///< Color base/Albedo por defecto (RGBA).
    float metallic = 1.0f;               ///< Grado de metalicidad (0.0 a 1.0).
    float roughness = 1.0f;              ///< Rugosidad de la superficie (0.0 a 1.0).
    float ao = 1.0f;                     ///< Oclusión ambiental escalar por defecto.
    float normalScale = 1.0f;            ///< Multiplicador de la intensidad del mapa de normales.
    float emissiveStrength = 1.0f;       ///< Multiplicador de potencia lumínica del mapa emisivo.
    float alphaCutoff = 0.5f;            ///< Umbral para descartar píxeles cuando el dominio es Masked.
};
///@}


/** @name Constant Buffers (Estructuras de Memoria alineadas para GPU) */
///@{

/**
 * @struct CBPerFrame
 * @brief Constant Buffer que almacena variables globales compartidas por toda la escena en el frame actual.
 * @note Cumple con la alineación estricta de 16 bytes requerida por shaders (HLSL/GLSL).
 */
struct CBPerFrame {
    XMFLOAT4X4 View{};                    ///< Matriz de Vista de la cámara principal.
    XMFLOAT4X4 Projection{};              ///< Matriz de Proyección de la cámara principal.
    XMFLOAT4X4 LightViewProjection{};     ///< Matriz de Vista-Proyección de la luz para cálculo de sombras.
    EU::Vector3 CameraPos{};              ///< Posición global de la cámara en el espacio de mundo.
    float pad0 = 0.0f;                    ///< Relleno para alineación de 16 bytes.
    EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f); ///< Dirección de la luz principal.
    float pad1 = 0.0f;                    ///< Relleno para alineación de 16 bytes.
    EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f); ///< Color de la luz principal.
    float ligthRange = 10.0f;             ///< Rango o alcance de la luz. (Nota: contiene typo "ligth" nativo).
    EU::Vector3 LightPosition = EU::Vector3(0.0f, 5.0f, 0.0f); ///< Posición de la luz principal.
    int LightCount = 0;                   ///< Número de luces activas a iterar en el shader.
    XMFLOAT3 pad2 = XMFLOAT3(0.0f, 0.0f, 0.0f); ///< Relleno para que el buffer final sea múltiplo de 16 bytes.
};

/**
 * @struct CBPerObject
 * @brief Constant Buffer que actualiza las transformaciones espaciales individuales de cada objeto antes de su dibujado.
 */
struct CBPerObject {
    XMFLOAT4X4 World{}; ///< Matriz de Transformación de Mundo (Transformación local a global).
};

/**
 * @struct CBPerMaterial
 * @brief Constant Buffer optimizado para enviar las propiedades del material PBR directamente al Pixel Shader.
 * @note Totalmente alineado mediante floats de padding redundantes para evitar discrepancias de memoria en la GPU.
 */
struct CBPerMaterial {
    XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); ///< Color base (16 bytes).
    float Metallic = 1.0f;          ///< Metalicidad (4 bytes).
    float Roughness = 1.0f;         ///< Rugosidad (4 bytes).
    float AO = 1.0f;                ///< Oclusión ambiental (4 bytes).
    float NormalScale = 1.0f;       ///< Factor de escala de normales (4 bytes) -> Total 16 bytes.
    float EmissiveStrength = 1.0f;  ///< Multiplicador de emisión (4 bytes).
    float AlphaCutoff = 0.0f;       ///< Umbral de recorte alfa (4 bytes).

    // Relleno de seguridad para completar la estructura en bloques uniformes en registros float4 de GPU
    float pad0 = 0.0f;
    float pad1 = 0.0f;
    float pad2 = 0.0f;
    float pad3 = 0.0f;
    float pad4 = 0.0f;
    float pad5 = 0.0f;
};
///@}


/**
 * @struct RenderObject
 * @brief Define una unidad fundamental de dibujo (Draw Command) enviada al backend gráfico.
 *
 * Esta estructura empaqueta el "qué" se va a dibujar (Mesh), el "cómo" se va a ver
 * (MaterialInstance), la transformación espacial actual (world) y datos de control para ordenamiento
 * y optimización (culling, sombras y profundidad).
 */
struct RenderObject {
    Mesh* mesh = nullptr;                               ///< Puntero a la geometría geométrica del objeto.
    MaterialInstance* materialInstance = nullptr;       ///< Instancia de material principal asignada por defecto.
    std::vector<MaterialInstance*> materialInstances;   ///< Mapeo de materiales múltiples indexados si la malla contiene varias submallas.
    XMMATRIX world = XMMatrixIdentity();                ///< Matriz de transformación de mundo de alta precisión (SIMD).

    bool castShadow = true;                             ///< Define si este objeto proyectará silueta en el mapa de sombras.
    bool transparent = false;                           ///< Bandera rápida que define si el objeto requiere procesamiento en la cola transparente.
    float distanceToCamera = 0.0f;                      ///< Distancia actual a la cámara (útil para ordenar objetos transparentes de atrás hacia adelante).
};