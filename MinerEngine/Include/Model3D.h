#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @enum ModelType
 * @brief Define los formatos de archivo de modelos 3D compatibles.
 */
enum ModelType {
    OBJ, ///< Formato Wavefront OBJ (Simple).
    FBX  ///< Formato Autodesk FBX (Complejo, soporta jerarquías y materiales).
};

/**
 * @class Model3D
 * @brief Clase que representa un recurso de modelo 3D en el motor.
 * * Se encarga de la carga, procesamiento y almacenamiento de mallas (meshes) y materiales.
 * Utiliza el SDK de FBX para extraer datos geométricos y de textura de archivos complejos.
 */
class Model3D : public IResource {
public:
    /**
     * @brief Constructor que inicializa el recurso y dispara la carga del archivo.
     * @param name Nombre del recurso.
     * @param modelType Tipo de archivo (OBJ o FBX).
     */
    Model3D(const std::string& name, ModelType modelType)
        : IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
        SetType(ResourceType::Model3D);
        load(name);
    }

    /** @brief Destructor virtual. */
    ~Model3D() = default;

    /**
     * @brief Implementación de la carga desde disco.
     * @param path Ruta del archivo de modelo.
     * @return true si el archivo se leyó y procesó correctamente.
     */
    bool load(const std::string& path) override;

    /**
     * @brief Inicializa los recursos de GPU necesarios para las mallas cargadas.
     * @return true si la inicialización en GPU fue exitosa.
     */
    bool init() override;

    /**
     * @brief Libera los datos de las mallas y los objetos del SDK de FBX.
     */
    void unload() override;

    /**
     * @brief Calcula el tamaño total en bytes de todas las mallas cargadas.
     * @return Tamaño en bytes para profiling.
     */
    size_t getSizeInBytes() const override;

    /**
     * @brief Retorna la lista de mallas que componen el modelo.
     * @return Referencia constante al vector de MeshComponents.
     */
    const std::vector<MeshComponent>& GetMeshes() const { return m_meshes; }

    /** @name FBX Model Loader
     * Funciones específicas para el manejo del SDK de Autodesk FBX.
     */
     ///@{

     /**
      * @brief Inicializa el gestor del SDK de FBX (FbxManager).
      * @return true si el SDK está listo para usarse.
      */
    bool InitializeFBXManager();

    /**
     * @brief Orquestador de la carga de archivos FBX.
     * @param filePath Ruta del archivo .fbx.
     * @return Vector de MeshComponents extraídos del archivo.
     */
    std::vector<MeshComponent> LoadFBXModel(const std::string& filePath);

    /**
     * @brief Recorre de forma recursiva los nodos del archivo FBX.
     * @param node Nodo actual a procesar.
     */
    void ProcessFBXNode(FbxNode* node);

    /**
     * @brief Extrae la información geométrica (vértices, índices, normales) de un nodo.
     * @param node Nodo que contiene un atributo de tipo FbxMesh.
     */
    void ProcessFBXMesh(FbxNode* node);

    /**
     * @brief Extrae los datos de materiales vinculados a la malla.
     * @param material Puntero al material del SDK de FBX.
     */
    void ProcessFBXMaterials(FbxSurfaceMaterial* material);

    /**
     * @brief Obtiene las rutas de las texturas encontradas en el modelo.
     * @return Vector de strings con los nombres de archivos de textura.
     */
    std::vector<std::string> GetTextureFileNames() const { return textureFileNames; }
    ///@}

private:
    FbxManager* lSdkManager; ///< Gestor de memoria del SDK de FBX.
    FbxScene* lScene;       ///< Representación de la escena cargada desde el archivo.
    std::vector<std::string> textureFileNames; ///< Lista temporal de texturas necesarias.

public:
    ModelType m_modelType;          ///< Formato original del modelo.
    std::vector<MeshComponent> m_meshes; ///< Contenedor final de las sub-mallas procesadas.
};