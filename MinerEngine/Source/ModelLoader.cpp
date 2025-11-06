#include "ModelLoader.h"
#include <cstdio>

struct VertexData
{
  unsigned int PosIndex;
  unsigned int TexIndex;
  unsigned int NormalIndex;

  bool operator==(const VertexData& other) const {
    return PosIndex == other.PosIndex &&
      TexIndex == other.TexIndex &&
      NormalIndex == other.NormalIndex;
  }
};

HRESULT
ModelLoader::init(MeshComponent& mesh, const std::string& fileName) {
  if (fileName.empty()) {
    ERROR("ModelLoader", "init", "El nombre del archivo no puede estar vacío.");
    return E_INVALIDARG;
  }

  std::vector<XMFLOAT3> temp_positions;
  std::vector<XMFLOAT2> temp_texcoords;
  std::vector<XMFLOAT3> temp_normals;

  std::vector<VertexData> face_data;
  std::vector<VertexData> unique_vertices;

  mesh.m_vertex.clear();
  mesh.m_index.clear();

  FILE* file = nullptr;
  if (fopen_s(&file, fileName.c_str(), "r") != 0 || file == nullptr) {
    ERROR("ModelLoader", "init",
      ("Fallo al abrir el archivo de modelo. Verifique la ruta: " + fileName).c_str());
    return E_FAIL;
  }

  char lineBuffer[1024];
  while (std::fgets(lineBuffer, 1024, file) != nullptr) {
    std::string line(lineBuffer);
    if (line.empty() || line[0] == '#') continue;

    std::stringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (prefix == "v") {
      XMFLOAT3 pos;
      if (!(ss >> pos.x >> pos.y >> pos.z)) continue;
      temp_positions.push_back(pos);
    }
    else if (prefix == "vt") {
      XMFLOAT2 tex;
      if (!(ss >> tex.x >> tex.y)) continue;
      tex.y = 1.0f - tex.y;
      temp_texcoords.push_back(tex);
    }
    else if (prefix == "vn") {
      XMFLOAT3 normal;
      if (!(ss >> normal.x >> normal.y >> normal.z)) continue;
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
          vd.PosIndex = std::stoul(segment.substr(pos_start, tex_start));

          if (tex_start != std::string::npos && norm_start != tex_start + 1) {
            vd.TexIndex = std::stoul(segment.substr(tex_start + 1, norm_start - tex_start - 1));
          }

          if (norm_start != std::string::npos) {
            vd.NormalIndex = std::stoul(segment.substr(norm_start + 1));
          }

          vd.PosIndex--;
          vd.TexIndex = (vd.TexIndex > 0) ? vd.TexIndex - 1 : 0;
          vd.NormalIndex = (vd.NormalIndex > 0) ? vd.NormalIndex - 1 : 0;

          face_indices.push_back(vd);

        }
        catch (const std::exception& e) {
          ERROR("ModelLoader", "ParseFace",
            ("Error al parsear segmento de cara '" + segment + "'. Detalle: " + e.what()).c_str());
          std::fclose(file);
          return E_FAIL;
        }
      }

      for (size_t i = 1; i < face_indices.size() - 1; ++i) {
        face_data.push_back(face_indices[0]);
        face_data.push_back(face_indices[i]);
        face_data.push_back(face_indices[i + 1]);
      }
    }
  }
  std::fclose(file);

  for (const auto& vd : face_data) {
    bool found = false;
    unsigned int existing_index = 0;

    for (unsigned int i = 0; i < unique_vertices.size(); ++i) {
      if (unique_vertices[i] == vd) {
        found = true;
        existing_index = i;
        break;
      }
    }

    if (!found) {
      unsigned int new_index = static_cast<unsigned int>(mesh.m_vertex.size());
      unique_vertices.push_back(vd);

      SimpleVertex new_vertex = {};

      if (vd.PosIndex < temp_positions.size()) {
        new_vertex.Pos = temp_positions[vd.PosIndex];
      }
      else {
        ERROR("ModelLoader", "Reconstruccion", "Error: Índice de posición inválido.");
        return E_FAIL;
      }

      if (vd.TexIndex < temp_texcoords.size()) {
        new_vertex.Tex = temp_texcoords[vd.TexIndex];
      }
      else {
        new_vertex.Tex = XMFLOAT2(0.0f, 0.0f);
      }

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
      mesh.m_index.push_back(existing_index);
    }
  }

  mesh.m_numVertex = static_cast<int>(mesh.m_vertex.size());
  mesh.m_numIndex = static_cast<int>(mesh.m_index.size());

  MESSAGE("ModelLoader", "init", ("Carga y re-indexación exitosa de: " + fileName).c_str());
  MESSAGE("ModelLoader", "init", ("Vértices finales (después de re-indexación): " + std::to_string(mesh.m_numVertex)).c_str());
  MESSAGE("ModelLoader", "init", ("Índices finales: " + std::to_string(mesh.m_numIndex)).c_str());

  return S_OK;
}

void ModelLoader::update() {
}
void ModelLoader::render() {
}
void ModelLoader::destroy() {
}