#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;

/**
 * @class DepthStencilView
 * @brief Clase encargada de gestionar la vista de profundidad/est�ncil en Direct3D 11.
 *
 * Proporciona una interfaz para crear, aplicar y liberar un objeto
 * @c ID3D11DepthStencilView, el cual es fundamental para habilitar el uso
 * de un buffer de profundidad/est�ncil dentro del pipeline gr�fico
 * (fase de Output-Merger).
 *
 * @note Esta clase no se hace responsable de la administraci�n de memoria
 * de los objetos @c Texture ni @c DeviceContext.
 */
class DepthStencilView {
public:
  /**
   * @brief Construye un objeto vac�o sin asignar recursos.
   *
   * No se realiza ninguna operaci�n de creaci�n durante esta fase.
   */
  DepthStencilView() = default;

  /**
   * @brief Destructor trivial.
   *
   * No se liberan recursos autom�ticamente. Se recomienda invocar
   * @c destroy() expl�citamente antes de la destrucci�n del objeto.
   */
  ~DepthStencilView() = default;

  /**
   * @brief Crea la vista de profundidad/est�ncil a partir de una textura v�lida.
   *
   * Este m�todo inicializa internamente un @c ID3D11DepthStencilView que se
   * asociar� con el recurso proporcionado, siempre que este haya sido
   * creado con la bandera @c D3D11_BIND_DEPTH_STENCIL.
   *
   * @param device        Referencia al dispositivo gr�fico responsable de la creaci�n.
   * @param depthStencil  Textura que actuar� como buffer de profundidad/est�ncil.
   * @param format        Formato DXGI en el que se generar� la vista
   *                      (ejemplo: @c DXGI_FORMAT_D24_UNORM_S8_UINT).
   * @return @c S_OK si la creaci�n se realiz� con �xito. En caso contrario,
   *         devuelve el c�digo de error correspondiente.
   *
   * @post Si la funci�n es exitosa, @c m_depthStencilView contendr� un puntero v�lido.
   */
  HRESULT init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

  /**
   * @brief Actualiza el estado de la vista de profundidad/est�ncil.
   *
   * M�todo definido como marcador. Actualmente no realiza operaciones,
   * pero se deja como punto de extensi�n para futuras necesidades.
   */
  void update() {};

  /**
   * @brief Enlaza la vista de profundidad/est�ncil al contexto de render.
   *
   * Invoca internamente a @c OMSetRenderTargets para adjuntar el
   * @c m_depthStencilView al @c DeviceContext indicado.
   *
   * @param deviceContext Contexto de dispositivo donde se activar� la vista.
   *
   * @pre El objeto debe haber sido correctamente inicializado mediante @c init().
   */
  void render(DeviceContext& deviceContext);

  /**
   * @brief Libera la vista de profundidad/est�ncil asociada.
   *
   * Este m�todo es seguro de llamar m�ltiples veces. Una vez liberado,
   * @c m_depthStencilView se establece en @c nullptr.
   */
  void destroy();

public:
  /**
   * @brief Puntero al recurso @c ID3D11DepthStencilView.
   *
   * Se inicializa en @c nullptr por defecto. Solo contendr� una referencia
   * v�lida despu�s de una creaci�n exitosa mediante @c init().
   * Se restablece a @c nullptr tras llamar a @c destroy().
   */
  ID3D11DepthStencilView* m_depthStencilView = nullptr;
};
