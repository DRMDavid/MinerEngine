#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

/**
 * @struct Submesh
 * @brief Representa una sección individual de una malla que comparte el mismo material.
 * * Una Submesh contiene sus propios buffers de vértices e índices. Esto permite que una
 * sola malla (Mesh) tenga diferentes secciones con distintos materiales o propiedades
 * de renderizado.
 */
struct Submesh {
    /// Buffer de vértices que contiene los datos de posición, normales, UVs, etc.
    Buffer vertexBuffer;

    /// Buffer de índices para el dibujado indexado (Indexed Rendering).
    Buffer indexBuffer;

    /// Número total de índices a dibujar en esta sección.
    unsigned int indexCount = 0;

    /// Índice de inicio en el buffer (offset) para el comando de dibujo.
    unsigned int startIndex = 0;

    /// Índice del slot de material que debe usar esta sub-malla en la instancia del material.
    unsigned int materialSlot = 0;
};

/**
 * @class Mesh
 * @brief Clase contenedor para una malla poligonal completa.
 * * Una Mesh está compuesta por una o varias Submeshes. Actúa como el recurso principal
 * de geometría que puede ser instanciado en la escena.
 */
class Mesh {
public:
    /** @name Gestión de Geometría */
    ///@{

    /**
     * @brief Obtiene la lista de sub-mallas que componen esta malla.
     * @return Referencia al vector de Submeshes.
     */
    std::vector<Submesh>& getSubmeshes() { return m_submeshes; }

    /**
     * @brief Obtiene la lista de sub-mallas (versión constante).
     * @return Referencia constante al vector de Submeshes.
     */
    const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }
    ///@}

    /**
     * @brief Libera los recursos de la GPU de todas las sub-mallas.
     * * Itera por cada sub-malla, destruye sus Vertex y Index Buffers,
     * y finalmente limpia el contenedor.
     */
    void destroy() {
        for (Submesh& submesh : m_submeshes) {
            submesh.vertexBuffer.destroy();
            submesh.indexBuffer.destroy();
        }
        m_submeshes.clear();
    }

private:
    /// Listado de secciones geométricas que forman la malla completa.
    std::vector<Submesh> m_submeshes;
};