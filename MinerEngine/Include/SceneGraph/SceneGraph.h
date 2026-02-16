#pragma once
#include "Prerequisites.h"

class Entity;
class DeviceContext;

/**
 * @class SceneGraph
 * @brief Sistema encargado de gestionar la jerarquía y el ciclo de vida de las entidades en la escena.
 * * El SceneGraph permite organizar las entidades en una estructura de árbol, encargándose de:
 * 1. Validar relaciones para evitar ciclos (bucles infinitos).
 * 2. Propagar transformaciones espaciales (World Matrices) de padres a hijos.
 * 3. Gestionar el registro centralizado de todas las entidades activas.
 */
class SceneGraph {
public:
    /** @brief Constructor por defecto. */
    SceneGraph() = default;

    /** @brief Destructor por defecto. */
    ~SceneGraph() = default;

    /**
     * @brief Inicializa los estados internos del grafo de escena.
     */
    void init();

    /**
     * @brief Registra una nueva entidad en el grafo para que sea procesada.
     * @param e Puntero a la entidad a registrar.
     */
    void addEntity(Entity* e);

    /**
     * @brief Elimina una entidad del grafo y limpia sus vínculos jerárquicos.
     * @param e Puntero a la entidad a eliminar.
     */
    void removeEntity(Entity* e);

    /**
     * @brief Comprueba si una entidad es ancestro (padre, abuelo, etc.) de otra.
     * @note Crucial para evitar que un padre intente ser hijo de su propio hijo.
     * @param possibleAncestor La entidad que podría ser el ancestro.
     * @param node La entidad desde la cual se empieza a buscar hacia arriba.
     * @return true si existe una relación de ancestro.
     */
    bool isAncestor(Entity* possibleAncestor, Entity* node) const;

    /**
     * @brief Crea un vínculo jerárquico entre dos entidades.
     * @param child Entidad que será subordinada.
     * @param parent Entidad que actuará como padre.
     * @return true si el vínculo se realizó con éxito (y no creó ciclos).
     */
    bool attach(Entity* child, Entity* parent);

    /**
     * @brief Desvincula a un hijo de su padre actual, moviéndolo de nuevo al nivel raíz.
     * @param child Entidad a desvincular.
     * @return true si la operación fue exitosa.
     */
    bool detach(Entity* child);

    /**
     * @brief Actualiza todas las entidades y recalcula las matrices de transformación.
     * @param deltaTime Tiempo transcurrido desde el último frame.
     * @param deviceContext Contexto del dispositivo para operaciones lógicas.
     */
    void update(float deltaTime, DeviceContext& deviceContext);

    /**
     * @brief Dispara el renderizado de todas las entidades registradas.
     * @param deviceContext Contexto del dispositivo para el pipeline de gráficos.
     */
    void render(DeviceContext& deviceContext);

    /**
     * @brief Limpia el grafo y libera las referencias a las entidades.
     */
    void destroy();

private:
    /**
     * @brief Función recursiva para calcular la matriz de mundo (World Matrix).
     * * Multiplica la matriz local del @p node por la matriz @p parentWorld para
     * obtener la posición global real en el espacio 3D.
     * * @param node Entidad actual a procesar.
     * @param parentWorld Matriz de transformación acumulada del padre.
     */
    void updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld);

    /**
     * @brief Verifica si una entidad no tiene padre asignado.
     * @param e Entidad a consultar.
     */
    bool isRoot(Entity* e) const;

    /**
     * @brief Verifica si la entidad ya se encuentra en la lista maestra de entidades.
     * @param e Entidad a consultar.
     */
    bool isRegistered(Entity* e) const;

private:
    //std::vector<EU::TSharedPointer<Entity>> m_entities;

public:
    /** @brief Lista maestra de todas las entidades registradas en la escena. */
    std::vector<Entity*> m_entities;
};