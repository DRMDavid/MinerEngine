#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;

/**
 * @class DepthStencilView
 * @brief Clase encargada de gestionar la vista de profundidad/esténcil en Direct3D 11.
 *
 * Proporciona una interfaz para crear, aplicar y liberar un objeto
 * @c ID3D11DepthStencilView, el cual es fundamental para habilitar el uso
 * de un buffer de profundidad/esténcil dentro del pipeline gráfico
 * (fase de Output-Merger).
 *
 * @note Esta clase no se hace responsable de la administración de memoria
 * de los objetos @c Texture ni @c DeviceContext.
 */
class DepthStencilView {
public:
  /**
   * @brief Construye un objeto vacío sin asignar recursos.
   *
   * No se realiza ninguna operación de creación durante esta fase.
   */
  DepthStencilView() = default;

  /**
   * @brief Destructor trivial.
   *
   * No se liberan recursos automáticamente. Se recomienda invocar
   * @c destroy() explícitamente antes de la destrucción del objeto.
   */
  ~DepthStencilView() = default;

  /**
   * @brief Crea la vista de profundidad/esténcil a partir de una textura válida.
   *
   * Este método inicializa internamente un @c ID3D11DepthStencilView que se
   * asociará con el recurso proporcionado, siempre que este haya sido
   * creado con la bandera @c D3D11_BIND_DEPTH_STENCIL.
   *
   * @param device        Referencia al dispositivo gráfico responsable de la creación.
   * @param depthStencil  Textura que actuará como buffer de profundidad/esténcil.
   * @param format        Formato DXGI en el que se generará la vista
   *                      (ejemplo: @c DXGI_FORMAT_D24_UNORM_S8_UINT).
   * @return @c S_OK si la creación se realizó con éxito. En caso contrario,
   *         devuelve el código de error correspondiente.
   *
   * @post Si la función es exitosa, @c m_depthStencilView contendrá un puntero válido.
   */
  HRESULT init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

  /**
   * @brief Actualiza el estado de la vista de profundidad/esténcil.
   *
   * Método definido como marcador. Actualmente no realiza operaciones,
   * pero se deja como punto de extensión para futuras necesidades.
   */
  void update() {};

  /**
   * @brief Enlaza la vista de profundidad/esténcil al contexto de render.
   *
   * Invoca internamente a @c OMSetRenderTargets para adjuntar el
   * @c m_depthStencilView al @c DeviceContext indicado.
   *
   * @param deviceContext Contexto de dispositivo donde se activará la vista.
   *
   * @pre El objeto debe haber sido correctamente inicializado mediante @c init().
   */
  void render(DeviceContext& deviceContext);

  /**
   * @brief Libera la vista de profundidad/esténcil asociada.
   *
   * Este método es seguro de llamar múltiples veces. Una vez liberado,
   * @c m_depthStencilView se establece en @c nullptr.
   */
  void destroy();

public:
  /**
   * @brief Puntero al recurso @c ID3D11DepthStencilView.
   *
   * Se inicializa en @c nullptr por defecto. Solo contendrá una referencia
   * válida después de una creación exitosa mediante @c init().
   * Se restablece a @c nullptr tras llamar a @c destroy().
   */
  ID3D11DepthStencilView* m_depthStencilView = nullptr;
};
