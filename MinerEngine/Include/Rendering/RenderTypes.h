#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/** @enum MaterialDomain
 * @brief Define el dominio conceptual del material para determinar su posición en el pipeline.
 */
enum class MaterialDomain {
    Opaque = 0,    ///< Materiales sólidos sin transparencia.
    Masked,        ///< Materiales con recorte binario (Alpha Testing/Cutoff).
    Transparent    ///< Materiales con mezcla alfa (Alpha Blending).
};

/** @enum BlendMode
 * @brief Modos de mezcla de color (Pixel Blending) para la etapa de Output Merger.
 */
enum class BlendMode {
    Opaque = 0,             ///< Reemplaza el color del buffer.
    Alpha,                  ///< Mezcla basada en el canal alfa (SrcAlpha, InvSrcAlpha).
    Additive,               ///< Suma de colores (útil para efectos de partículas).
    PremultipliedAlpha      ///< Mezcla con alfa pre-multiplicado.
};

/** @enum RenderPassType
 * @brief Identifica el tipo de pase de renderizado que se está ejecutando.
 */
enum class RenderPassType {
    Shadow = 0,    ///< Generación de mapas de sombras (Depth only).
    Opaque,        ///< Pase principal de geometría opaca.
    Skybox,        ///< Renderizado del fondo/entorno.
    Transparent,   ///< Pase de objetos con mezcla alfa.
    Editor         ///< Renderizado de elementos de utilidad (Gizmos, Wireframe).
};

/** @enum LightType
 * @brief Tipos de fuentes de luz soportadas por el motor.
 */
enum class LightType {
    Directional = 0, ///< Luz infinita y paralela (Sol).
    Point,           ///< Luz puntual con decaimiento por radio.
    Spot             ///< Luz focal con forma de cono.
};

/** @struct LightData
 * @brief Contiene todos los parámetros necesarios para los cálculos de iluminación.
 */
struct LightData {
    LightType type = LightType::Directional;    ///< Tipo de luz.
    EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f); ///< Color de la luz (RGB).
    float intensity = 1.0f;                     ///< Factor de brillo.

    EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f); ///< Dirección (para Directional/Spot).
    float range = 0.0f;                         ///< Alcance máximo (para Point/Spot).

    EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f); ///< Ubicación en el mundo.
    float spotAngle = 0.0f;                     ///< Ángulo del cono en radianes (para Spot).
};

/** @struct MaterialParams
 * @brief Parámetros de superficie para el modelo PBR.
 */
struct MaterialParams {
    XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); ///< Color base o Albedo.
    float metallic = 1.0f;           ///< Grado de metalicidad [0-1].
    float roughness = 1.0f;          ///< Rugosidad de la superficie [0-1].
    float ao = 1.0f;                 ///< Factor de oclusión ambiental.
    float normalScale = 1.0f;        ///< Intensidad del Normal Map.
    float emissiveStrength = 1.0f;   ///< Multiplicador de luz emitida.
    float alphaCutoff = 0.5f;        ///< Umbral para materiales Masked.
};

/** @struct CBPerFrame
 * @brief Constant Buffer global actualizado una vez por frame.
 * @note Alineado a 16 bytes para compatibilidad con HLSL.
 */
struct CBPerFrame {
    XMFLOAT4X4 View{};           ///< Matriz de vista de la cámara.
    XMFLOAT4X4 Projection{};     ///< Matriz de proyección.
    EU::Vector3 CameraPos{};     ///< Posición de la cámara para cálculos especulares.
    float pad0 = 0.0f;           ///< Padding para alineación.
    EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f); ///< Dirección de la luz principal.
    float pad1 = 0.0f;
    EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f); ///< Color de la luz principal.
    float pad2 = 0.0f;
};

/** @struct CBPerObject
 * @brief Constant Buffer actualizado por cada objeto renderizado.
 */
struct CBPerObject {
    XMFLOAT4X4 World{};          ///< Matriz de transformación de mundo.
};

/** @struct CBPerMaterial
 * @brief Constant Buffer que envía los parámetros del material al Shader.
 * @note Se recomienda mantener los paddings para asegurar que cada float4 esté alineado.
 */
struct CBPerMaterial {
    XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    float Metallic = 1.0f;
    float Roughness = 1.0f;
    float AO = 1.0f;
    float NormalScale = 1.0f;
    float EmissiveStrength = 1.0f;
    float AlphaCutoff = 0.0f;
    // Paddings necesarios para completar registros de 16 bytes en la GPU
    float pad0 = 0.0f; float pad1 = 0.0f; float pad2 = 0.0f;
    float pad3 = 0.0f; float pad4 = 0.0f; float pad5 = 0.0f;
};

/** @struct RenderObject
 * @brief Representa un comando de dibujo con todos sus datos asociados.
 * * Esta estructura es la que se almacena en las listas de RenderScene.
 */
struct RenderObject {
    Mesh* mesh = nullptr;                      ///< Geometría a dibujar.
    MaterialInstance* materialInstance = nullptr; ///< Material principal.
    std::vector<MaterialInstance*> materialInstances; ///< Soporte para múltiples materiales por sub-malla.
    XMMATRIX world = XMMatrixIdentity();       ///< Matriz de transformación final.
    bool castShadow = true;                    ///< Indica si el objeto genera sombras.
    bool transparent = false;                  ///< Flag de optimización para pases de renderizado.
    float distanceToCamera = 0.0f;             ///< Usado para el ordenamiento (Depth Sorting).
};