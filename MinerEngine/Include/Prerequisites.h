#pragma once

/**
 * @file
 * @brief Definiciones básicas de estructuras, macros y utilidades para DirectX 11.
 * @details
 *  Este archivo declara vértices simples, buffers constantes para shaders,
 *  macros de liberación/registro y un enumerado de extensiones de textura.
 *  Está pensado para usarse en aplicaciones Windows con Direct3D 11.
 *
 *  Dependencias principales:
 *   - Windows SDK
 *   - DirectX 11 (d3d11, d3dx11, d3dcompiler)
 *   - XNA Math (xnamath) / DirectXMath (XMFLOAT/XMMATRIX)
 */

 //------------------------------------------
 // Librerías estándar (STD)
 //------------------------------------------
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>

//------------------------------------------
// Librerías DirectX
//------------------------------------------
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

//------------------------------------------
// Librerías de terceros
//------------------------------------------
// (Agregar aquí si aplica)

//------------------------------------------
// MACROS
//------------------------------------------

/**
 * @def SAFE_RELEASE(x)
 * @brief Libera con seguridad un objeto COM y lo pone a @c nullptr.
 * @param x Puntero a una interfaz COM (por ejemplo, @c ID3D11Buffer*, @c ID3D11Device*).
 * @warning El parámetro debe ser un puntero válido o @c nullptr. Tras la macro, @c x queda en @c nullptr.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @def MESSAGE(classObj, method, state)
  * @brief Registra un mensaje informativo en el depurador sobre la creación de un recurso.
  * @param classObj Nombre de la clase (o identificador) que emite el mensaje.
  * @param method   Método o función desde donde se emite el mensaje.
  * @param state    Estado o descripción adicional (p. ej., "OK", "FAILED", etc.).
  * @details
  *  Construye un mensaje con el formato:
  *  @code
  *    <classObj>::<method> : [CREATION OF RESOURCE : <state>]
  *  @endcode
  *  y lo envía a la ventana de salida del depurador mediante @c OutputDebugStringW.
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @def ERROR(classObj, method, errorMSG)
   * @brief Registra un mensaje de error en el depurador con información contextual.
   * @param classObj Nombre de la clase (o identificador) donde ocurrió el error.
   * @param method   Método o función que detectó el error.
   * @param errorMSG Mensaje de error detallado.
   * @details
   *  Ante cualquier excepción durante el formateo del mensaje, escribe
   *  @c "Failed to log error message." para evitar que el registro falle silenciosamente.
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
   // Estructuras
   //--------------------------------------------------------------------------------------

   /**
    * @struct SimpleVertex
    * @brief Representa un vértice con posición y coordenadas de textura.
    * @var SimpleVertex::Pos
    *      Posición del vértice en espacio 3D (x, y, z).
    * @var SimpleVertex::Tex
    *      Coordenadas de textura (u, v) asociadas al vértice.
    */
struct SimpleVertex
{
  XMFLOAT3 Pos;  ///< Posición 3D del vértice.
  XMFLOAT2 Tex;  ///< Coordenadas (u, v) para mapeo de textura.
};

/**
 * @struct CBNeverChanges
 * @brief Buffer constante para datos que rara vez cambian.
 * @details
 *  Usualmente se enlaza como @c b0 en el pipeline de shaders y contiene la matriz de vista.
 *  Puede subirse una vez por frame o menos si la cámara es estática.
 * @var CBNeverChanges::mView
 *      Matriz de vista (transforma del mundo a espacio de cámara).
 */
struct CBNeverChanges
{
  XMMATRIX mView; ///< Matriz de vista.
};

/**
 * @struct CBChangeOnResize
 * @brief Buffer constante para datos que cambian al redimensionar la ventana.
 * @details
 *  Normalmente contiene la proyección para ajustar el aspecto (aspect ratio)
 *  cuando cambia el tamaño del viewport o del swap chain.
 * @var CBChangeOnResize::mProjection
 *      Matriz de proyección (perspectiva u ortográfica).
 */
struct CBChangeOnResize
{
  XMMATRIX mProjection; ///< Matriz de proyección.
};

/**
 * @struct CBChangesEveryFrame
 * @brief Buffer constante para datos que cambian cada fotograma.
 * @details
 *  Usado típicamente para animar transformaciones y/o variar el color del mesh por frame.
 * @var CBChangesEveryFrame::mWorld
 *      Matriz de mundo (transforma de espacio local a espacio mundial).
 * @var CBChangesEveryFrame::vMeshColor
 *      Color del mesh como vector (rgba).
 */
struct CBChangesEveryFrame
{
  XMMATRIX mWorld;      ///< Matriz de mundo por objeto.
  XMFLOAT4 vMeshColor;  ///< Color RGBA del mesh.
};

/**
 * @enum ExtensionType
 * @brief Tipos de extensiones de imagen soportadas.
 * @var ExtensionType::DDS
 *      Textura en formato DirectDraw Surface (DDS).
 * @var ExtensionType::PNG
 *      Imagen en formato Portable Network Graphics (PNG).
 * @var ExtensionType::JPG
 *      Imagen en formato JPEG (JPG).
 */
enum ExtensionType {
  DDS = 0, ///< Formato DDS (común en texturas de GPU).
  PNG = 1, ///< Formato PNG (sin pérdida, soporta transparencia).
  JPG = 2  ///< Formato JPG/JPEG (con pérdida).
};

enum ShaderType {
  VERTEX_SHADER = 0,
  PIXEL_SHADER = 1
};
