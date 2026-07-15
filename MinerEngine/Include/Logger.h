#pragma once
#include <string>
#include <vector>
#include <mutex>

/**
 * @enum LogLevel
 * @brief Define los niveles de severidad para los mensajes de registro (log).
 */
enum class LogLevel {
    Info = 0,    ///< Mensaje informativo general.
    Warning = 1, ///< Advertencia sobre un problema potencial o comportamiento inesperado.
    Error = 2    ///< Error crítico o fallo en una operación.
};

/**
 * @struct LogEntry
 * @brief Representa una entrada individual dentro del registro.
 */
struct LogEntry {
    LogLevel level;         ///< Nivel de severidad del mensaje.
    std::string message;    ///< Contenido del mensaje de registro.
};

/**
 * @class Logger
 * @brief Buffer central de logs para la consola del editor (Singleton).
 * @details Esta clase es segura para subprocesos (thread-safe) y mantiene un historial
 * circular para evitar desbordamientos de memoria, eliminando entradas antiguas cuando
 * se alcanza el límite.
 */
class Logger {
public:
    /**
     * @brief Obtiene la instancia única (Singleton) del Logger.
     * @return Referencia a la instancia estática de Logger.
     */
    static Logger& get() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Añade un nuevo mensaje de registro estándar.
     * @details Si el buffer supera las 8000 entradas, se eliminan las 2000 más antiguas
     * para liberar espacio de manera eficiente.
     * @param level Nivel de severidad del mensaje.
     * @param msg Cadena de texto con el mensaje.
     */
    void add(LogLevel level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_entries.size() > 8000) {
            m_entries.erase(m_entries.begin(), m_entries.begin() + 2000);
        }
        m_entries.push_back({ level, msg });
        m_dirty = true;
    }

    /**
     * @brief Añade un nuevo mensaje de registro desde una cadena ancha (wstring).
     * @param level Nivel de severidad del mensaje.
     * @param wmsg Cadena de texto ancha con el mensaje.
     */
    void addW(LogLevel level, const std::wstring& wmsg) {
        add(level, narrow(wmsg));
    }

    /**
     * @brief Obtiene una copia segura de todas las entradas de registro actuales.
     * @return Vector que contiene todas las entradas de LogEntry.
     */
    std::vector<LogEntry> snapshot() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_entries;
    }

    /**
     * @brief Vacía completamente el historial de registros.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
    }

    /**
     * @brief Consume la bandera de actualización y la reinicia.
     * @details Útil para que la interfaz de usuario sepa si debe redibujar o hacer auto-scroll.
     * @return true si se añadieron nuevos registros desde la última vez que se llamó a este método.
     */
    bool consumeDirty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool d = m_dirty;
        m_dirty = false;
        return d;
    }

private:
    /**
     * @brief Constructor privado para aplicar el patrón Singleton.
     */
    Logger() = default;

    /**
     * @brief Convierte una cadena ancha (wstring) en una cadena estándar (string).
     * @details Reemplaza caracteres no imprimibles por espacios y saltos de línea por puntos,
     * además de limpiar los espacios finales.
     * @param w Cadena ancha de entrada.
     * @return Cadena convertida y sanitizada.
     */
    static std::string narrow(const std::wstring& w) {
        std::string s;
        s.reserve(w.size());
        for (wchar_t c : w) {
            s.push_back((c >= 32 && c < 127) ? (char)c : (c == L'\n' ? '.' : ' '));
        }
        // quita salto de linea final o espacios
        while (!s.empty() && (s.back() == '\n' || s.back() == ' ')) s.pop_back();
        return s;
    }

    std::vector<LogEntry> m_entries; ///< Contenedor interno de las entradas de registro.
    std::mutex m_mutex;              ///< Mutex para garantizar la seguridad en entornos multihilo.
    bool m_dirty = false;            ///< Bandera que indica si hay nuevas entradas sin leer por la UI.
};