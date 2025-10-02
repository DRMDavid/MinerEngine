#pragma once
#include "Prerequisites.h"

/**
 * @class Device
 * @brief Clase responsable de administrar el dispositivo gr�fico Direct3D 11.
 *
 * Facilita la creaci�n y control de un objeto @c ID3D11Device, permitiendo
 * inicializar, actualizar y destruir los recursos asociados. Tambi�n expone
 * m�todos de conveniencia para generar vistas, texturas, shaders, buffers
 * y otros elementos utilizados en la etapa de renderizado.
 */
class Device {
public:
  /**
   * @brief Construye un objeto sin inicializar.
   *
   * No se crea ning�n recurso en este punto.
   */
  Device() = default;

  /**
   * @brief Destructor trivial.
   *
   * No libera autom�ticamente los recursos.
   * Se recomienda invocar @c destroy() antes de la destrucci�n del objeto.
   */
  ~Device() = default;

  /**
   * @brief Configura el dispositivo Direct3D 11.
   *
   * Realiza las operaciones necesarias para la creaci�n del @c ID3D11Device
   * y recursos b�sicos requeridos.
   */
  void init();

  /**
   * @brief Aplica cambios en el estado del dispositivo.
   *
   * Este m�todo puede usarse para actualizar configuraciones
   * relacionadas con los recursos gr�ficos.
   */
  void update();

  /**
   * @brief Lanza las rutinas de renderizado asociadas al dispositivo.
   */
  void render();

  /**
   * @brief Libera todos los recursos creados y cierra el dispositivo.
   *
   * Una vez invocado, @c m_device se establece en @c nullptr.
   */
  void destroy();

  /**
   * @brief Crea una vista de render target.
   *
   * @param pResource Recurso de origen.
   * @param pDesc     Descriptor opcional para la vista.
   * @param ppRTView  Referencia de salida para la interfaz creada.
   * @return @c S_OK si la creaci�n fue exitosa; c�digo de error en caso contrario.
   */
  HRESULT CreateRenderTargetView(ID3D11Resource* pResource,
                                 const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
                                 ID3D11RenderTargetView** ppRTView);

  /**
   * @brief Crea una textura 2D en memoria de GPU.
   *
   * @param pDesc        Estructura con las caracter�sticas de la textura.
   * @param pInitialData Datos iniciales opcionales.
   * @param ppTexture2D  Referencia de salida para la textura resultante.
   * @return @c HRESULT indicando el estado de la operaci�n.
   */
  HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
                          const D3D11_SUBRESOURCE_DATA* pInitialData,
                          ID3D11Texture2D** ppTexture2D);

  /**
   * @brief Crea una vista de Depth Stencil.
   *
   * @param pResource           Recurso base.
   * @param pDesc               Descriptor opcional de la vista.
   * @param ppDepthStencilView  Referencia de salida para la vista resultante.
   * @return @c HRESULT con el resultado de la operaci�n.
   */
  HRESULT CreateDepthStencilView(ID3D11Resource* pResource,
                                 const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
                                 ID3D11DepthStencilView** ppDepthStencilView);

  /// @brief Crea un shader de v�rtices desde bytecode compilado.
  HRESULT CreateVertexShader(const void* pShaderBytecode,
                             SIZE_T BytecodeLength,
                             ID3D11ClassLinkage* pClassLinkage,
                             ID3D11VertexShader** ppVertexShader);

  /// @brief Genera un layout de entrada para la etapa de ensamblado de v�rtices.
  HRESULT CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
                            UINT NumElements,
                            const void* pShaderBytecodeWithInputSignature,
                            SIZE_T BytecodeLength,
                            ID3D11InputLayout** ppInputLayout);

  /// @brief Crea un shader de p�xeles desde bytecode compilado.
  HRESULT CreatePixelShader(const void* pShaderBytecode,
                            SIZE_T BytecodeLength,
                            ID3D11ClassLinkage* pClassLinkage,
                            ID3D11PixelShader** ppPixelShader);

  /// @brief Reserva un buffer en GPU para v�rtices, �ndices u otros datos.
  HRESULT CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
                       const D3D11_SUBRESOURCE_DATA* pInitialData,
                       ID3D11Buffer** ppBuffer);

  /// @brief Crea un estado de muestreo de texturas.
  HRESULT CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
                             ID3D11SamplerState** ppSamplerState);

public:
  /**
   * @brief Puntero al objeto @c ID3D11Device subyacente.
   *
   * Inicializado en @c init() y liberado en @c destroy().
   */
  ID3D11Device* m_device = nullptr;
};
