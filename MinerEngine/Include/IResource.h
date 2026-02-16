#pragma once
#include "Prerequisites.h"

/**
 * @enum ResourceType
 * @brief Define los tipos de recursos soportados por el motor.
 */
enum class ResourceType {
    Unknown,  ///< Tipo no definido o error.
    Model3D,  ///< Mallas y geometrías 3D (e.g., .obj, .fbx).
    Texture,  ///< Mapas de bits (e.g., .png, .tga, .dds).
    Sound,    ///< Archivos de audio.
    Shader,   ///< Programas de sombreado (HLSL/GLSL).
    Material  ///< Definiciones de propiedades de superficie.
};

/**
 * @enum ResourceState
 * @brief Representa el ciclo de vida o estado actual de carga de un recurso.
 */
enum class ResourceState {
    Unloaded, ///< El recurso existe en memoria pero no tiene datos cargados.
    Loading,  ///< El recurso está en proceso de lectura o transferencia a GPU.
    Loaded,   ///< El recurso está listo para ser utilizado.
    Failed    ///< Ocurrió un error durante la carga.
};

/**
 * @class IResource
 * @brief Interfaz base para todos los recursos del motor.
 * * Esta clase define el contrato que deben seguir todos los assets (texturas, modelos, etc.)
 * para ser gestionados por un Resource Manager. Incluye manejo de estados y un ID único.
 */
class IResource {
public:
    /**
     * @brief Constructor que inicializa el recurso con un nombre.
     * @param name Nombre identificativo del recurso.
     */
    IResource(const std::string& name)
        : m_name(name)
        , m_filePath("")
        , m_type(ResourceType::Unknown)
        , m_state(ResourceState::Unloaded)
        , m_id(GenerateID())
    {
    }

    /** @brief Destructor virtual para asegurar la limpieza correcta en clases derivadas. */
    virtual ~IResource() = default;

    /**
     * @brief Inicializa los componentes de GPU del recurso.
     * @return true si la creación en GPU fue exitosa, false en caso contrario.
     */
    virtual bool init() = 0;

    /**
     * @brief Carga los datos del recurso desde el disco duro.
     * @param filename Ruta completa o relativa del archivo.
     * @return true si la lectura fue exitosa.
     */
    virtual bool load(const std::string& filename) = 0;

    /**
     * @brief Libera la memoria (tanto RAM como VRAM) del recurso.
     */
    virtual void unload() = 0;

    /**
     * @brief Obtiene el tamaño que ocupa el recurso en memoria.
     * @return Tamaño en bytes. Util para herramientas de profiling.
     */
    virtual size_t getSizeInBytes() const = 0;

    /** @name Setters */
    ///@{
    void SetPath(const std::string& path) { m_filePath = path; }
    void SetType(ResourceType t) { m_type = t; }
    void SetState(ResourceState s) { m_state = s; }
    ///@}

    /** @name Getters */
    ///@{
    const std::string& GetName() const { return m_name; }
    const std::string& GetPath() const { return m_filePath; }
    ResourceType GetType() const { return m_type; }
    ResourceState GetState() const { return m_state; }
    uint64_t GetID() const { return m_id; }
    ///@}

protected:
    std::string m_name;       ///< Nombre interno del recurso.
    std::string m_filePath;   ///< Ruta de origen en el sistema de archivos.
    ResourceType m_type;      ///< Categoría del recurso.
    ResourceState m_state;    ///< Estado actual de disponibilidad.
    uint64_t m_id;            ///< Identificador único generado en tiempo de ejecución.

private:
    /**
     * @brief Genera un ID incremental de forma atómica/estática.
     * @return Un nuevo ID único para la instancia actual de la aplicación.
     */
    static uint64_t GenerateID()
    {
        static uint64_t nextID = 1;
        return nextID++;
    }
};