#pragma once

/**
 * @file
 * @brief Definiciones b�sicas de estructuras, macros y utilidades para DirectX 11.
 * @details
 *  Este archivo declara v�rtices simples, buffers constantes para shaders,
 *  macros de liberaci�n/registro y un enumerado de extensiones de textura.
 *  Est� pensado para usarse en aplicaciones Windows con Direct3D 11.
 *
 *  Dependencias principales:
 *   - Windows SDK
 *   - DirectX 11 (d3d11, d3dx11, d3dcompiler)
 *   - XNA Math (xnamath) / DirectXMath (XMFLOAT/XMMATRIX)
 */

 //------------------------------------------
 // Librer�as est�ndar (STD)
 //------------------------------------------
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>

//------------------------------------------
// Librer�as DirectX
//------------------------------------------
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

//------------------------------------------
// Librer�as de terceros
//------------------------------------------
// (Agregar aqu� si aplica)

//------------------------------------------
// MACROS
//------------------------------------------

/**
 * @def SAFE_RELEASE(x)
 * @brief Libera con seguridad un objeto COM y lo pone a @c nullptr.
 * @param x Puntero a una interfaz COM (por ejemplo, @c ID3D11Buffer*, @c ID3D11Device*).
 * @warning El par�metro debe ser un puntero v�lido o @c nullptr. Tras la macro, @c x queda en @c nullptr.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @def MESSAGE(classObj, method, state)
  * @brief Registra un mensaje informativo en el depurador sobre la creaci�n de un recurso.
  * @param classObj Nombre de la clase (o identificador) que emite el mensaje.
  * @param method   M�todo o funci�n desde donde se emite el mensaje.
  * @param state    Estado o descripci�n adicional (p. ej., "OK", "FAILED", etc.).
  * @details
  *  Construye un mensaje con el formato:
  *  @code
  *    <classObj>::<method> : [CREATION OF RESOURCE : <state>]
  *  @endcode
  *  y lo env�a a la ventana de salida del depurador mediante @c OutputDebugStringW.
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @def ERROR(classObj, method, errorMSG)
   * @brief Registra un mensaje de error en el depurador con informaci�n contextual.
   * @param classObj Nombre de la clase (o identificador) donde ocurri� el error.
   * @param method   M�todo o funci�n que detect� el error.
   * @param errorMSG Mensaje de error detallado.
   * @details
   *  Ante cualquier excepci�n durante el formateo del mensaje, escribe
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
    * @brief Representa un v�rtice con posici�n y coordenadas de textura.
    * @var SimpleVertex::Pos
    *      Posici�n del v�rtice en espacio 3D (x, y, z).
    * @var SimpleVertex::Tex
    *      Coordenadas de textura (u, v) asociadas al v�rtice.
    */
struct SimpleVertex
{
  XMFLOAT3 Pos;  ///< Posici�n 3D del v�rtice.
  XMFLOAT2 Tex;  ///< Coordenadas (u, v) para mapeo de textura.
};

/**
 * @struct CBNeverChanges
 * @brief Buffer constante para datos que rara vez cambian.
 * @details
 *  Usualmente se enlaza como @c b0 en el pipeline de shaders y contiene la matriz de vista.
 *  Puede subirse una vez por frame o menos si la c�mara es est�tica.
 * @var CBNeverChanges::mView
 *      Matriz de vista (transforma del mundo a espacio de c�mara).
 */
struct CBNeverChanges
{
  XMMATRIX mView; ///< Matriz de vista.
};

/**
 * @struct CBChangeOnResize
 * @brief Buffer constante para datos que cambian al redimensionar la ventana.
 * @details
 *  Normalmente contiene la proyecci�n para ajustar el aspecto (aspect ratio)
 *  cuando cambia el tama�o del viewport o del swap chain.
 * @var CBChangeOnResize::mProjection
 *      Matriz de proyecci�n (perspectiva u ortogr�fica).
 */
struct CBChangeOnResize
{
  XMMATRIX mProjection; ///< Matriz de proyecci�n.
};

/**
 * @struct CBChangesEveryFrame
 * @brief Buffer constante para datos que cambian cada fotograma.
 * @details
 *  Usado t�picamente para animar transformaciones y/o variar el color del mesh por frame.
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
  DDS = 0, ///< Formato DDS (com�n en texturas de GPU).
  PNG = 1, ///< Formato PNG (sin p�rdida, soporta transparencia).
  JPG = 2  ///< Formato JPG/JPEG (con p�rdida).
};
