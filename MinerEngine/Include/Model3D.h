#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @enum ModelType
 * @brief Define los formatos de modelo 3D soportados nativamente por la clase.
 */
enum ModelType {
    OBJ, ///< Formato estándar Wavefront OBJ.
    FBX  ///< Formato Autodesk FBX (Soporta jerarquías y materiales múltiples).
};

/**
 * @class Model3D
 * @brief Recurso que representa un modelo 3D cargado en memoria.
 * @details Hereda de IResource y se encarga de parsear e importar geometría (vértices, índices, normales, UVs)
 * desde archivos físicos (FBX u OBJ) o desde datos en crudo (como un Skybox).
 */
class Model3D : public IResource {
public:
    /**
     * @brief Constructor principal. Inicializa y carga un modelo 3D desde un archivo en disco.
     * @param name Nombre del recurso y ruta del archivo a cargar.
     * @param modelType Formato esperado del modelo (OBJ o FBX).
     */
    Model3D(const std::string& name, ModelType modelType)
        : IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
        SetType(ResourceType::Model3D);
        load(name);
    }

    /**
     * @brief Constructor especializado para crear un modelo de entorno (Skybox) a partir de vértices definidos en código.
     * @param name Nombre identificador del recurso.
     * @param vertices Arreglo de 8 vértices que componen el cubo del Skybox.
     * @param indices Arreglo de 36 índices que definen los triángulos del cubo.
     */
    Model3D(const std::string& name,
        const SkyboxVertex vertices[],
        const unsigned int indices[]) : IResource(name) {
        MeshComponent mesh;
        mesh.m_skyVertex.assign(vertices, vertices + 8);
        mesh.m_index.assign(indices, indices + 36);
        mesh.m_numIndex = mesh.m_index.size();
        SetType(ResourceType::Model3D);
        m_meshes.push_back(mesh);
    }

    ~Model3D() = default;

    /**
     * @brief Inicia el proceso de carga y parseo del archivo de modelo.
     * @param path Ruta del archivo del modelo.
     * @return true si la carga y el parseo fueron exitosos, false en caso de error.
     */
    bool load(const std::string& path) override;

    /**
     * @brief Inicializa los recursos gráficos necesarios para el modelo (como buffers de vértices en la GPU).
     * @return true si la inicialización de hardware fue exitosa.
     */
    bool init() override;

    /**
     * @brief Libera la memoria RAM estática y recursos dependientes de este modelo.
     */
    void unload() override;

    /**
     * @brief Calcula la huella de memoria aproximada de este recurso.
     * @return Tamaño total ocupado en bytes.
     */
    size_t getSizeInBytes() const override;

    /**
     * @brief Obtiene la colección de mallas que conforman este modelo.
     * @return Referencia constante al vector interno de MeshComponent.
     */
    const std::vector<MeshComponent>& GetMeshes() const { return m_meshes; }

    /* ==========================================
     * CARGADOR DE MODELOS FBX (Autodesk SDK)
     * ========================================== */

     /**
      * @brief Inicializa la instancia global del administrador y gestor de memoria del SDK de FBX.
      * @return true si el FbxManager se creó e inicializó correctamente.
      */
    bool InitializeFBXManager();

    /**
     * @brief Utiliza el SDK para leer e interpretar un archivo FBX, construyendo las estructuras internas de malla.
     * @param filePath Ruta del archivo .fbx a importar.
     * @return Vector de MeshComponent conteniendo toda la geometría extraída de la escena.
     */
    std::vector<MeshComponent> LoadFBXModel(const std::string& filePath);

    /**
     * @brief Analiza recursivamente un nodo dentro de la jerarquía de la escena FBX.
     * @param node Puntero al nodo FBX actual.
     */
    void ProcessFBXNode(FbxNode* node);

    /**
     * @brief Procesa el atributo de malla de un nodo, transformando los datos nativos de FBX a la estructura del motor (MeshComponent).
     * @param node Puntero al nodo FBX que contiene la malla.
     */
    void ProcessFBXMesh(FbxNode* node);

    /**
     * @brief Extrae los datos de materiales (texturas, colores base) vinculados a una superficie.
     * @param material Puntero al material de superficie FBX a inspeccionar.
     */
    void ProcessFBXMaterials(FbxSurfaceMaterial* material);

    /**
     * @brief Retorna las rutas o nombres de archivo de textura encontrados durante el parseo de los materiales.
     * @return Vector de cadenas con los nombres de las texturas referenciadas.
     */
    std::vector<std::string> GetTextureFileNames() const { return textureFileNames; }

private:
    FbxManager* lSdkManager;                    ///< Gestor principal del SDK de Autodesk FBX.
    FbxScene* lScene;                           ///< Contenedor de la jerarquía y los nodos de la escena FBX importada.
    std::vector<std::string> textureFileNames;  ///< Cache temporal de los nombres de textura descubiertos en el archivo.

public:
    ModelType m_modelType;                      ///< Bandera que indica el formato original mediante el que se cargó el modelo.
    std::vector<MeshComponent> m_meshes;        ///< Contenedor lineal de todas las partes geométricas (sub-mallas) extraídas del modelo.
};