#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @enum ModelType
 * @brief Formatos de intercambio de archivos soportados por el importador.
 */
enum
	ModelType {
	OBJ, ///< Formato Wavefront OBJ (Geometría estática simple).
	FBX  ///< Formato Autodesk FBX (Soporte para escenas complejas y materiales).
};

/**
 * @class Model3D
 * @brief Adaptador de recursos para la importación de geometría externa.
 *
 * Esta clase especializa IResource para gestionar la carga, interpretación y conversión
 * de archivos de modelos 3D. Su función principal es descomponer escenas complejas
 * provenientes de herramientas DCC (como Maya o Blender) en una lista de componentes
 * de malla (MeshComponents) nativos del motor.
 */
class
	Model3D : public IResource {
public:
	/**
	 * @brief Constructor de recurso.
	 * * Inicializa el gestor de memoria del SDK y configura el tipo de importación.
	 * @param name Identificador o ruta del archivo a cargar.
	 * @param modelType Especificación del algoritmo de parsing a utilizar.
	 */
	Model3D(const std::string& name, ModelType modelType)
		: IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
		SetType(ResourceType::Model3D);
		load(name);
	}

	/**
	 * @brief Destructor por defecto.
	 */
	~Model3D() = default;

	/**
	 * @brief Ejecución del pipeline de importación.
	 * * Implementación de IResource. Abre el archivo en disco e invoca al parser
	 * correspondiente (FBX/OBJ) para poblar la lista de mallas.
	 * @param path Ruta absoluta o relativa al archivo fuente.
	 * @return True si el archivo fue parseado y convertido exitosamente.
	 */
	bool
		load(const std::string& path) override;

	/**
	 * @brief Validación de recursos.
	 * * Implementación de IResource. Verifica que la geometría cargada sea válida
	 * para su uso en GPU.
	 * @return True si el modelo contiene al menos una malla válida.
	 */
	bool
		init() override;

	/**
	 * @brief Limpieza del entorno de importación.
	 * * Implementación de IResource. Destruye la escena FBX y libera los gestores
	 * de memoria del SDK externo.
	 */
	void
		unload() override;

	/**
	 * @brief Cálculo de huella de memoria.
	 * * @return Estimación de bytes ocupados por los vértices e índices de todas las mallas.
	 */
	size_t
		getSizeInBytes() const override;

	/**
	 * @brief Acceso a la geometría procesada.
	 * @return Referencia de solo lectura al contenedor de mallas generadas.
	 */
	const std::vector<MeshComponent>&
		GetMeshes() const { return m_meshes; }

	/* FBX MODEL LOADER */

	/**
	 * @brief Configuración del entorno Autodesk FBX SDK.
	 * * Inicializa los objetos FbxManager y FbxIOSettings necesarios para
	 * las operaciones de E/S.
	 * @return True si el SDK se inicializó correctamente.
	 */
	bool
		InitializeFBXManager();

	/**
	 * @brief Orquestador de la carga FBX.
	 * * Importa la escena completa del archivo y comienza el recorrido del grafo.
	 * @param filePath Ubicación del archivo .fbx.
	 * @return Lista de componentes de malla resultantes de la conversión.
	 */
	std::vector<MeshComponent>
		LoadFBXModel(const std::string& filePath);

	/**
	 * @brief Recorrido recursivo del grafo de escena.
	 * * Analiza un nodo del árbol FBX y, si contiene atributos de malla,
	 * delega el procesamiento; posteriormente visita a los hijos del nodo.
	 * @param node Puntero al nodo actual en la jerarquía FBX.
	 */
	void
		ProcessFBXNode(FbxNode* node);

	/**
	 * @brief Extracción de topología y datos.
	 * * Convierte los datos de control (Puntos, Normales, UVs) del formato FBX
	 * al formato de vértices nativo del motor (SimpleVertex).
	 * @param node Nodo que contiene el atributo de geometría.
	 */
	void
		ProcessFBXMesh(FbxNode* node);

	/**
	 * @brief Análisis de propiedades de superficie.
	 * * Extrae información relevante de los materiales vinculados, como rutas
	 * a texturas difusas.
	 * @param material Puntero al material FBX asociado a la malla.
	 */
	void
		ProcessFBXMaterials(FbxSurfaceMaterial* material);

	/**
	 * @brief Recuperación de dependencias de imagen.
	 * @return Lista de nombres de archivos de textura detectados durante el parsing.
	 */
	std::vector<std::string>
		GetTextureFileNames() const { return textureFileNames; }
private:
	FbxManager* lSdkManager;  ///< Gestor de memoria principal del SDK de FBX.
	FbxScene* lScene;         ///< Contenedor raíz de la jerarquía de objetos importados.
	std::vector<std::string> textureFileNames; ///< Caché de rutas de texturas encontradas.
public:
	ModelType m_modelType;              ///< Formato del archivo origen.
	std::vector<MeshComponent> m_meshes;///< Colección de sub-mallas que componen el modelo.
};