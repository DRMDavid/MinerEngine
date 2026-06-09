#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

/**
 * @struct Submesh
 * @brief Representa una sección independiente o sub-sección geométrica de una malla principal.
 *
 * Una malla compleja suele dividirse en múltiples submallas. Esto permite que diferentes
 * partes de un mismo objeto 3D utilicen diferentes materiales o configuraciones de buffers
 * en un solo comando de dibujado por submalla.
 */
struct Submesh {
    Buffer vertexBuffer;         ///< Buffer de GPU que almacena los vértices (posiciones, normales, UVs, etc.).
    Buffer indexBuffer;          ///< Buffer de GPU que almacena los índices para el dibujado indexado.

    unsigned int indexCount = 0;   ///< Cantidad de índices que componen esta submalla.
    unsigned int startIndex = 0;   ///< Desplazamiento (offset) inicial dentro del Buffer de Índices.
    unsigned int materialSlot = 0; ///< Índice del material asignado a esta submalla dentro de la instancia.
};

/**
 * @class Mesh
 * @brief Contenedor principal que agrupa y gestiona la geometría de un objeto 3D.
 *
 * La clase Mesh actúa como la estructura contenedora de alto nivel que administra
 * la memoria de GPU para todas las submallas que componen el modelo tridimensional.
 */
class Mesh {
public:
    /** @name Acceso a Datos */
    ///@{

    /**
     * @brief Obtiene una referencia modificable al vector de submallas.
     * @return std::vector<Submesh>& Referencia al contenedor de submallas.
     */
    std::vector<Submesh>& getSubmeshes() { return m_submeshes; }

    /**
     * @brief Obtiene una referencia de solo lectura al vector de submallas.
     * @return const std::vector<Submesh>& Referencia constante al contenedor de submallas.
     */
    const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }
    ///}

    /**
     * @brief Libera todos los recursos de hardware (GPU) asociados a la malla.
     * * Recorre cada submalla para destruir sus respectivos Vertex y Index Buffers,
     * evitando fugas de memoria en la VRAM, y finalmente limpia el contenedor.
     */
    void destroy() {
        for (Submesh& submesh : m_submeshes) {
            submesh.vertexBuffer.destroy();
            submesh.indexBuffer.destroy();
        }
        m_submeshes.clear();
    }

private:
    std::vector<Submesh> m_submeshes; ///< Lista de submallas que componen este objeto 3D.
};