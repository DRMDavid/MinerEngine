#pragma once
#include "Prerequisites.h"
#include "IResource.h"

/**
 * @class ResourceManager
 * @brief Gestor centralizado para el ciclo de vida de activos.
 *
 * Implementa el patrón Singleton para ofrecer un punto de acceso global
 * al inventario de recursos. Actúa como una caché inteligente,
 * garantizando que cada archivo en disco se cargue una única vez en memoria,
 * compartiendo punteros a dicha instancia para optimizar el rendimiento (Flyweight).
 */
class
	ResourceManager {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	ResourceManager() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~ResourceManager() = default;

	/**
	 * @brief Acceso a la instancia única.
	 * * Utiliza la inicialización estática local (thread-safe en C++11) para
	 * garantizar que el gestor exista antes de su primer uso.
	 * @return Referencia al objeto singleton.
	 */
	static ResourceManager& getInstance() {
		static ResourceManager instance;
		return instance;
	}

	/**
	 * @brief Eliminación de constructor de copia.
	 * * Previene la duplicación de la instancia singleton.
	 */
	ResourceManager(const ResourceManager&) = delete;

	/**
	 * @brief Eliminación del operador de asignación.
	 * * Previene la asignación de la instancia singleton.
	 */
	ResourceManager& operator=(const ResourceManager&) = delete;

	/**
	 * @brief Orquestador de adquisición de recursos.
	 * * Implementa una estrategia de "búsqueda o creación". Verifica si el recurso ya existe
	 * en el registro interno (mapa hash); si existe y es válido, devuelve la instancia existente.
	 * Si no, instancia el objeto, dispara su carga desde disco, inicializa sus recursos en GPU
	 * y lo registra para uso futuro.
	 * * @tparam T Tipo concreto del recurso (Debe heredar de IResource).
	 * @tparam Args Argumentos variádicos para el constructor del recurso.
	 * @param key Identificador único (hash key) para almacenar el recurso.
	 * @param filename Ruta del archivo fuente para cargar los datos.
	 * @param args Lista de argumentos que se reenviarán al constructor de T.
	 * @return Puntero compartido al recurso listo para usar, o nullptr si hubo error.
	 */
	template<typename T, typename... Args>
	std::shared_ptr<T> GetOrLoad(const std::string& key,
		const std::string& filename,
		Args&&... args) {
		static_assert(std::is_base_of<IResource, T>::value,
			"T debe heredar de IResource");

		// 1. Verificación de existencia en caché (Hit)
		auto it = m_resources.find(key);
		if (it != m_resources.end()) {
			// Validación de tipo y estado
			auto existing = std::dynamic_pointer_cast<T>(it->second);
			if (existing && existing->GetState() == ResourceState::Loaded) {
				return existing; // Retorno de instancia compartida
			}
		}

		// 2. Creación por ausencia en caché (Miss)
		std::shared_ptr<T> resource = std::make_shared<T>(key, std::forward<Args>(args)...);

		// Ejecución de pipeline de carga (Disco -> RAM)
		if (!resource->load(filename)) {
			// Falla en lectura de archivo
			return nullptr;
		}

		// Ejecución de pipeline de inicialización (RAM -> VRAM/API)
		if (!resource->init()) {
			return nullptr;
		}

		// 3. Registro y retorno
		m_resources[key] = resource;
		return resource;
	}

	/**
	 * @brief Búsqueda pasiva de recursos.
	 * * Intenta recuperar un recurso sin desencadenar procesos de carga si no existe.
	 * Útil para consultas de solo lectura.
	 * @tparam T Tipo al cual castear el recurso encontrado.
	 * @param key Clave de identificación del recurso.
	 * @return Puntero compartido al recurso si existe, nullptr en caso contrario.
	 */
	template<typename T>
	std::shared_ptr<T> Get(const std::string& key) const
	{
		auto it = m_resources.find(key);
		if (it == m_resources.end()) return nullptr;

		return std::dynamic_pointer_cast<T>(it->second);
	}

	/**
	 * @brief Liberación puntual de memoria.
	 * * Invoca el método unload() del recurso y lo elimina del registro,
	 * decrementando su contador de referencias.
	 * @param key Clave del recurso a eliminar.
	 */
	void Unload(const std::string& key)
	{
		auto it = m_resources.find(key);
		if (it != m_resources.end()) {
			it->second->unload();
			m_resources.erase(it);
		}
	}

	/**
	 * @brief Purga total del sistema.
	 * * Itera sobre todo el inventario invocando la liberación de recursos.
	 * Debe llamarse al cerrar la aplicación o cambiar de nivel/escena masiva.
	 */
	void UnloadAll()
	{
		for (auto& [key, res] : m_resources) {
			if (res) {
				res->unload();
			}
		}
		m_resources.clear();
	}

private:
	/**
	 * @brief Contenedor asociativo (Caché).
	 * * Mapea identificadores de cadena (Strings) a punteros polimórficos de recursos.
	 */
	std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;
};