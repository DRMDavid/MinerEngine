#pragma once
#include <vector>
#include <memory>
#include <utility>

/**
 * @class ICommand
 * @brief Interfaz base para acciones reversibles (Command Pattern).
 * @details Define la estructura obligatoria para todos los comandos que admitan operaciones de deshacer (undo) y rehacer (redo).
 */
class ICommand {
public:
    /**
     * @brief Destructor virtual por defecto.
     * @details Garantiza la correcta destrucción de las clases derivadas a través de punteros a la interfaz.
     */
    virtual ~ICommand() = default;

    /**
     * @brief Revierte la acción realizada por el comando.
     */
    virtual void undo() = 0;

    /**
     * @brief Reejecuta la acción revertida previamente por el comando.
     */
    virtual void redo() = 0;

    /**
     * @brief Obtiene el nombre identificativo del comando.
     * @return Cadena de caracteres constantes con el nombre del comando.
     */
    virtual const char* name() const { return "Command"; }
};

/**
 * @class CommandManager
 * @brief Pila de undo/redo. Generico: no depende del motor.
 * @details Gestiona el historial de comandos mediante pilas de deshacer y rehacer, controlando el límite máximo de historial asignado.
 */
class CommandManager {
public:
    /**
     * @brief Registra y ejecuta un nuevo comando en el historial.
     * @details Limpia el historial de rehacer (redo), añade el comando a la pila de deshacer (undo) y descarta los comandos más antiguos si se excede el límite de profundidad (m_maxDepth).
     * @param cmd Puntero único (`std::unique_ptr`) al comando que se desea registrar.
     */
    void push(std::unique_ptr<ICommand> cmd) {
        if (!cmd) return;
        m_redo.clear();
        m_undo.push_back(std::move(cmd));
        if (m_undo.size() > m_maxDepth)
            m_undo.erase(m_undo.begin(), m_undo.begin() + (m_undo.size() - m_maxDepth));
    }

    /**
     * @brief Revisa y deshace el último comando ejecutado en la pila de deshacer.
     * @details Extrae el comando superior de la pila de deshacer, invoca su método `undo()` y lo traslada a la pila de rehacer.
     */
    void undo() {
        if (m_undo.empty()) return;
        std::unique_ptr<ICommand> c = std::move(m_undo.back());
        m_undo.pop_back();
        c->undo();
        m_redo.push_back(std::move(c));
    }

    /**
     * @brief Reejecuta el último comando revertido en la pila de rehacer.
     * @details Extrae el comando superior de la pila de rehacer, invoca su método `redo()` y lo traslada de vuelta a la pila de deshacer.
     */
    void redo() {
        if (m_redo.empty()) return;
        std::unique_ptr<ICommand> c = std::move(m_redo.back());
        m_redo.pop_back();
        c->redo();
        m_undo.push_back(std::move(c));
    }

    /**
     * @brief Comprueba si existen acciones disponibles para deshacer.
     * @return True si la pila de deshacer no está vacía, false en caso contrario.
     */
    bool canUndo() const { return !m_undo.empty(); }

    /**
     * @brief Comprueba si existen acciones disponibles para rehacer.
     * @return True si la pila de rehacer no está vacía, false en caso contrario.
     */
    bool canRedo() const { return !m_redo.empty(); }

    /**
     * @brief Vacía completamente los historiales de deshacer y rehacer.
     */
    void clear() { m_undo.clear(); m_redo.clear(); }

    /**
     * @brief Obtiene el número de acciones almacenadas en la pila de deshacer.
     * @return Cantidad de comandos almacenados en el historial activo de undo.
     */
    size_t undoCount() const { return m_undo.size(); }

private:
    std::vector<std::unique_ptr<ICommand>> m_undo; /**< Pila/Contenedor de comandos para deshacer. */
    std::vector<std::unique_ptr<ICommand>> m_redo; /**< Pila/Contenedor de comandos para rehacer. */
    size_t m_maxDepth = 100;                       /**< Límite máximo de comandos guardados en el historial (hasta 100 pasos). */
};