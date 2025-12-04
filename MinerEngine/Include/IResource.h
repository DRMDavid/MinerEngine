#pragma once
#include "Prerequisites.h"

/**
 * @enum ResourceType
 * @brief Categorización taxonómica de los activos del motor.
 *
 * Define la naturaleza de los datos contenidos en el recurso para determinar
 * el pipeline de carga y procesamiento adecuado.
 */
enum class
	ResourceType {
	Unknown,  ///< Tipo no identificado o inicialización por defecto.
	Model3D,  ///< Malla geométrica (vértices, índices).
	Texture,  ///< Datos de imagen o mapas de bits.
	Sound,    ///< Clips de audio o flujos de sonido.
	Shader,   ///< Programas ejecutables en GPU.
	Material  ///< Definiciones de superficie y propiedades visuales.
};

/**
 * @enum ResourceState
 * @brief Máquina de estados del ciclo de vida del recurso.
 *
 * Indica la etapa actual de procesamiento en la que se encuentra el activo,
 * permitiendo la gestión asíncrona y el control de errores.
 */
enum class
	ResourceState {
	Unloaded, ///< El recurso existe como definición pero no ocupa memoria sustancial.
	Loading,  ///< El proceso de E/S o decodificación está en curso.
	Loaded,   ///< El recurso está completamente disponible en memoria y listo para usarse.
	Failed    ///< Ocurrió un error irrecuperable durante la carga o inicialización.
};

/**
 * @class IResource
 * @brief Contrato abstracto para la manipulación de recursos digitales.
 *
 * Establece la interfaz base que todos los activos gestionables (texturas, modelos, sonidos)
 * deben implementar. Encapsula metadatos comunes como rutas de archivo, identificadores
 * únicos y estado de carga, delegando la implementación específica de memoria a las clases derivadas.
 */
class IResource {
public:
	/**
	 * @brief Constructor de metadatos.
	 * * Inicializa la identidad del recurso sin realizar operaciones pesadas de E/S.
	 * @param name Etiqueta legible para identificar el recurso en herramientas de depuración.
	 */
	IResource(const std::string& name)
		: m_name(name)
		, m_filePath("")
		, m_type(ResourceType::Unknown)
		, m_state(ResourceState::Unloaded)
		, m_id(GenerateID())
	{
	}

	/**
	 * @brief Destructor virtual.
	 */
	virtual ~IResource() = default;

	/**
	 * @brief Reserva y configuración de recursos de hardware.
	 * * @pure Método abstracto encargado de crear las estructuras dependientes de la API gráfica (ej. buffers en VRAM).
	 * Debe invocarse después de que los datos crudos hayan sido cargados en RAM.
	 * @return True si la inicialización en el dispositivo fue exitosa.
	 */
	virtual bool init() = 0;

	/**
	 * @brief Ingesta de datos desde almacenamiento persistente.
	 * * @pure Método abstracto para leer el archivo desde disco, decodificarlo y almacenarlo en memoria del sistema.
	 * @param filename Ruta absoluta o relativa al archivo fuente.
	 * @return True si la lectura y parseo concluyeron sin errores.
	 */
	virtual bool load(const std::string& filename) = 0;

	/**
	 * @brief Purga de recursos.
	 * * @pure Método abstracto para liberar tanto la memoria del sistema (RAM) como la del dispositivo (VRAM),
	 * devolviendo el objeto a un estado 'Unloaded'.
	 */
	virtual void unload() = 0;

	/**
	 * @brief Auditoría de memoria.
	 * * @pure Método abstracto para consultar el peso del recurso.
	 * @return Cantidad estimada de bytes ocupados por este activo (útil para presupuestos de memoria).
	 */
	virtual size_t getSizeInBytes() const = 0;

	/**
	 * @brief Asigna la ruta de origen del archivo.
	 * @param path Cadena con la ubicación del activo en el sistema de archivos.
	 */
	void SetPath(const std::string& path) { m_filePath = path; }

	/**
	 * @brief Define la categoría del recurso.
	 * @param t Enumerador que clasifica este activo.
	 */
	void SetType(ResourceType t) { m_type = t; }

	/**
	 * @brief Actualiza el estado del ciclo de vida.
	 * @param s Nuevo estado operativo del recurso.
	 */
	void SetState(ResourceState s) { m_state = s; }

	/**
	 * @brief Recupera el nombre legible del recurso.
	 */
	const std::string& GetName() const { return m_name; }

	/**
	 * @brief Recupera la ruta de archivo asociada.
	 */
	const std::string& GetPath() const { return m_filePath; }

	/**
	 * @brief Consulta el tipo de activo.
	 */
	ResourceType GetType() const { return m_type; }

	/**
	 * @brief Consulta el estado actual de carga/disponibilidad.
	 */
	ResourceState GetState() const { return m_state; }

	/**
	 * @brief Obtiene el identificador único de instancia (UUID).
	 */
	uint64_t GetID() const { return m_id; }

protected:
	std::string m_name;      ///< Identificador textual para depuración.
	std::string m_filePath;  ///< Origen de datos en disco.
	ResourceType m_type;     ///< Clasificación del activo.
	ResourceState m_state;   ///< Estado actual en el flujo de carga.
	uint64_t m_id;           ///< ID numérico único generado en tiempo de ejecución.

private:
	/**
	 * @brief Generador secuencial de IDs.
	 * * Utiliza una variable estática para garantizar unicidad simple durante la sesión.
	 * @return Un entero de 64 bits único para cada instancia creada.
	 */
	static uint64_t GenerateID()
	{
		static uint64_t nextID = 1;
		return nextID++;
	}
};