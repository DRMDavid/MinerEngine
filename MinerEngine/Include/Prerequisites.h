#pragma once
/**
 * @file Prerequisites.h
 * @brief Configuración central y definiciones globales del motor.
 *
 * Este archivo agrupa las dependencias externas, macros de utilidad para depuración
 * y gestión de memoria, así como las estructuras de datos fundamentales compartidas
 * entre la CPU y la GPU (Shaders).
 */

 // Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>
#include <memory>
#include <unordered_map>
#include <type_traits>

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

// Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities\Memory\TSharedPointer.h"
#include "EngineUtilities\Memory\TWeakPointer.h"
#include "EngineUtilities\Memory\TStaticPtr.h"
#include "EngineUtilities\Memory\TUniquePtr.h"

// MACROS

/**
 * @def SAFE_RELEASE
 * @brief Mecanismo de seguridad para interfaces COM.
 *
 * Verifica si el puntero es válido antes de invocar el método Release(),
 * evitando violaciones de acceso y asegurando que el puntero se anule
 * tras la liberación para prevenir punteros colgantes.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @def MESSAGE
  * @brief Sistema de trazas de depuración informativo.
  *
  * Construye y emite una cadena de texto formateada hacia la ventana de salida
  * del depurador (Output Window), útil para rastrear el flujo de creación de recursos.
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @def ERROR
   * @brief Sistema de reporte de fallos críticos.
   *
   * Captura y formatea mensajes de error dentro de un bloque try-catch seguro,
   * garantizando que la excepción se registre en la consola de depuración
   * incluso si el formateo falla.
   */
#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        OutputDebugStringW(os_.str().c_str());                \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}

   //--------------------------------------------------------------------------------------
   // Structures
   //--------------------------------------------------------------------------------------

   /**
    * @struct SimpleVertex
    * @brief Estructura de entrada para el Input Assembler.
    *
    * Define el layout de memoria de un vértice individual tal como lo espera
    * el Vertex Shader.
    */
struct SimpleVertex
{
  XMFLOAT3 Pos; ///< Coordenadas espaciales (x, y, z).
  XMFLOAT2 Tex; ///< Coordenadas de mapeo de textura (u, v).

};

/**
 * @struct CBNeverChanges
 * @brief Buffer constante de frecuencia de actualización baja.
 *
 * Almacena datos que raramente cambian durante la ejecución de la escena,
 * como la matriz de Vista (Cámara), optimizando el ancho de banda del bus.
 */
struct CBNeverChanges
{
  XMMATRIX mView;
};

/**
 * @struct CBChangeOnResize
 * @brief Buffer constante dependiente de la ventana.
 *
 * Contiene matrices que solo requieren recalculo cuando cambian las dimensiones
 * del render target, típicamente la matriz de Proyección.
 */
struct CBChangeOnResize
{
  XMMATRIX mProjection;
};

/**
 * @struct CBChangesEveryFrame
 * @brief Buffer constante de alta frecuencia.
 *
 * Almacena datos que se actualizan en cada llamada de dibujo (DrawCall),
 * específicamente la matriz de Mundo del objeto y propiedades de material básicas.
 */
struct CBChangesEveryFrame
{
  XMMATRIX mWorld;
  XMFLOAT4 vMeshColor;
};

/**
 * @enum ExtensionType
 * @brief Formatos de imagen soportados por el cargador de texturas.
 */
enum ExtensionType {
  DDS = 0, ///< DirectDraw Surface (Formato nativo optimizado para GPU).
  PNG = 1, ///< Portable Network Graphics (Compresión sin pérdida).
  JPG = 2  ///< Joint Photographic Experts Group (Compresión con pérdida).
};

/**
 * @enum ShaderType
 * @brief Etapas programables del pipeline gráfico.
 */
enum ShaderType {
  VERTEX_SHADER = 0, ///< Etapa de procesamiento de geometría.
  PIXEL_SHADER = 1   ///< Etapa de rasterización y coloreado.
};

/**
 * @enum ComponentType
 * @brief Identificadores de RTTI para la arquitectura ECS.
 *
 * Etiquetas utilizadas para realizar casting seguro y búsquedas rápidas
 * dentro de los contenedores de componentes de las entidades.
 */
enum
  ComponentType {
  NONE = 0,      ///< Valor centinela o componente no inicializado.
  TRANSFORM = 1, ///< Datos de posición, rotación y escala.
  MESH = 2,      ///< Datos de topología geométrica.
  MATERIAL = 3   ///< Definición de shaders y texturas.
};