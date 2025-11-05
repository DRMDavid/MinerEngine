#include "ModelLoader.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>

// Definición de las estructuras internas para la re-indexación
// Esta estructura almacena un conjunto de índices (v/vt/vn) que 
// representan un "vértice único" para Direct3D.
struct VertexData
{
  unsigned int PosIndex;
  unsigned int TexIndex;
  unsigned int NormalIndex;

  // Operador de comparación para usar VertexData como clave en std::map
  // Si dos VertexData tienen los mismos índices v, vt, y vn, se consideran el mismo vértice.
  bool operator<(const VertexData& other) const {
    if (PosIndex != other.PosIndex) return PosIndex < other.PosIndex;
    if (TexIndex != other.TexIndex) return TexIndex < other.TexIndex;
    return NormalIndex < other.NormalIndex;
  }
};

HRESULT
ModelLoader::init(MeshComponent& mesh, const std::string& fileName) {
  if (fileName.empty()) {
    ERROR("ModelLoader", "init", "El nombre del archivo no puede estar vacío.");
    return E_INVALIDARG;
  }

  // Estructuras para los datos brutos del archivo OBJ
  std::vector<XMFLOAT3> temp_positions; // v
  std::vector<XMFLOAT2> temp_texcoords; // vt
  std::vector<XMFLOAT3> temp_normals;   // vn

  // Estructuras para la reconstrucción
  std::vector<VertexData> face_data; // Almacena los índices v/vt/vn de las caras trianguladas
  std::map<VertexData, unsigned int> index_map; // Mapa para la re-indexación final

  mesh.m_vertex.clear();
  mesh.m_index.clear();

  // ----------------------------------------------------------------------
  // 1. Lectura y parseo línea por línea
  // ----------------------------------------------------------------------
  std::ifstream file(fileName);
  if (!file.is_open()) {
    ERROR("ModelLoader", "init",
      ("Fallo al abrir el archivo de modelo. Verifique la ruta: " + fileName).c_str());
    return E_FAIL;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::stringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (prefix == "v") {
      XMFLOAT3 pos;
      ss >> pos.x >> pos.y >> pos.z;
      temp_positions.push_back(pos);
    }
    else if (prefix == "vt") {
      XMFLOAT2 tex;
      ss >> tex.x >> tex.y;
      // Invertir V (coordenada Y) para Direct3D
      tex.y = 1.0f - tex.y;
      temp_texcoords.push_back(tex);
    }
    else if (prefix == "vn") {
      XMFLOAT3 normal;
      ss >> normal.x >> normal.y >> normal.z;
      temp_normals.push_back(normal);
    }
    else if (prefix == "f") {
      std::vector<VertexData> face_indices;
      std::string segment;

      while (ss >> segment) {
        VertexData vd = { 0, 0, 0 };
        size_t pos_start = 0;
        size_t tex_start = segment.find('/');
        size_t norm_start = segment.find('/', tex_start + 1);

        try {
          // Posición (v)
          vd.PosIndex = std::stoul(segment.substr(pos_start, tex_start));

          // Textura (vt)
          if (tex_start != std::string::npos && norm_start != tex_start + 1) {
            vd.TexIndex = std::stoul(segment.substr(tex_start + 1, norm_start - tex_start - 1));
          }

          // Normal (vn)
          if (norm_start != std::string::npos) {
            vd.NormalIndex = std::stoul(segment.substr(norm_start + 1));
          }

          // Convertir de base 1 (OBJ) a base 0 (C++)
          vd.PosIndex--;
          vd.TexIndex = (vd.TexIndex > 0) ? vd.TexIndex - 1 : 0;
          vd.NormalIndex = (vd.NormalIndex > 0) ? vd.NormalIndex - 1 : 0;

          face_indices.push_back(vd);

        }
        catch (const std::exception& e) {
          ERROR("ModelLoader", "ParseFace",
            ("Error al parsear segmento de cara '" + segment + "'. Detalle: " + e.what()).c_str());
          file.close();
          return E_FAIL;
        }
      }

      // --------------------------------------------------------------
      // 2. Triangulación en Abanico (Fan Triangulation)
      // --------------------------------------------------------------
      for (size_t i = 1; i < face_indices.size() - 1; ++i) {
        // Genera un triángulo (índice 0, i, i+1)
        face_data.push_back(face_indices[0]);
        face_data.push_back(face_indices[i]);
        face_data.push_back(face_indices[i + 1]);
      }
    }
  }
  file.close();

  // ----------------------------------------------------------------------
  // 3. Reconstrucción del Mesh y Re-indexación
  // ----------------------------------------------------------------------
  for (const auto& vd : face_data) {
    if (index_map.find(vd) == index_map.end()) {
      // Vértice nuevo: crea un SimpleVertex y lo añade a la lista
      unsigned int new_index = static_cast<unsigned int>(mesh.m_vertex.size());
      index_map[vd] = new_index;

      SimpleVertex new_vertex = {};

      // Asignar Posición (v)
      if (vd.PosIndex < temp_positions.size()) {
        new_vertex.Pos = temp_positions[vd.PosIndex];
      }
      else {
        ERROR("ModelLoader", "Reconstruccion", "Error: Índice de posición inválido.");
        return E_FAIL;
      }

      // Asignar Coordenada de Textura (vt)
      if (vd.TexIndex < temp_texcoords.size()) {
        new_vertex.Tex = temp_texcoords[vd.TexIndex];
      }
      else {
        new_vertex.Tex = XMFLOAT2(0.0f, 0.0f);
      }

      // Asignar Normal (vn)
      if (vd.NormalIndex < temp_normals.size()) {
        new_vertex.Normal = temp_normals[vd.NormalIndex];
      }
      else {
        new_vertex.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
      }

      mesh.m_vertex.push_back(new_vertex);
      mesh.m_index.push_back(new_index);

    }
    else {
      // Vértice ya existe: solo añade su índice para reutilizar el dato
      mesh.m_index.push_back(index_map[vd]);
    }
  }

  // 4. Actualización de Metadatos
  mesh.m_numVertex = static_cast<int>(mesh.m_vertex.size());
  mesh.m_numIndex = static_cast<int>(mesh.m_index.size());

  MESSAGE("ModelLoader", "init", ("Carga y re-indexación exitosa de: " + fileName).c_str());
  MESSAGE("ModelLoader", "init", ("Vértices finales (después de re-indexación): " + std::to_string(mesh.m_numVertex)).c_str());
  MESSAGE("ModelLoader", "init", ("Índices finales: " + std::to_string(mesh.m_numIndex)).c_str());

  return S_OK;
}

// [CORRECCIÓN C2084] Implementación de funciones triviales (solo en el .cpp)
void ModelLoader::update() {
  /* Trivial */
}
void ModelLoader::render() {
  /* Trivial */
}
void ModelLoader::destroy() {
  /* Trivial */
}