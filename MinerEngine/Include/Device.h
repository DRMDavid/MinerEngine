#pragma once
#include "Prerequisites.h"

/**
 * @class Device
 * @brief Encapsula la creación, gestión y destrucción de un dispositivo Direct3D 11.
 *
 * Esta clase proporciona métodos para inicializar, actualizar, renderizar y destruir
 * un dispositivo Direct3D 11. Además, expone funciones de ayuda para crear recursos
 * gráficos como Render Target Views, Textures y Depth Stencil Views.
 */
class Device {
public:
  /**
   * @brief Constructor por defecto de la clase Device.
   */
  Device() = default;

  /**
   * @brief Destructor por defecto de la clase Device.
   */
  ~Device() = default;

  /**
   * @brief Inicializa el dispositivo Direct3D 11 y los recursos necesarios.
   */
  void init();

  /**
   * @brief Actualiza el estado del dispositivo o los recursos asociados.
   */
  void update();

  /**
   * @brief Ejecuta las rutinas de renderizado usando el dispositivo Direct3D 11.
   */
  void render();

  /**
   * @brief Libera y limpia todos los recursos asociados al dispositivo.
   */
  void destroy();

  /**
   * @brief Crea una vista de destino de renderizado (Render Target View).
   *
   * @param pResource Recurso de Direct3D desde el cual se creará la vista.
   * @param pDesc Descripción de la vista de destino de renderizado.
   * @param ppRTView Puntero que recibe la interfaz de la vista creada.
   * @return HRESULT Resultado de la operación (S_OK en caso de éxito).
   */
  HRESULT CreateRenderTargetView(
    ID3D11Resource* pResource,
    const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
    ID3D11RenderTargetView** ppRTView);

  /**
   * @brief Crea una textura 2D en el dispositivo.
   *
   * @param pDesc Descripción de la textura 2D a crear.
   * @param pInitialData Datos iniciales opcionales para poblar la textura.
   * @param ppTexture2D Puntero que recibe la interfaz de la textura creada.
   * @return HRESULT Resultado de la operación (S_OK en caso de éxito).
   */
  HRESULT CreateTexture2D(
    const D3D11_TEXTURE2D_DESC* pDesc,
    const D3D11_SUBRESOURCE_DATA* pInitialData,
    ID3D11Texture2D** ppTexture2D);

  /**
   * @brief Crea una vista de Depth Stencil a partir de un recurso.
   *
   * @param pResource Recurso de Direct3D del cual se derivará la vista.
   * @param pDesc Descripción de la vista de Depth Stencil.
   * @param ppDepthStencilView Puntero que recibe la interfaz de la vista creada.
   * @return HRESULT Resultado de la operación (S_OK en caso de éxito).
   */
  HRESULT CreateDepthStencilView(
    ID3D11Resource* pResource,
    const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
    ID3D11DepthStencilView** ppDepthStencilView);

  HRESULT
    CreateVertexShader(const void* pShaderBytecode,
      SIZE_T BytecodeLength,
      ID3D11ClassLinkage* pClassLinkage,
      ID3D11VertexShader** ppVertexShader);

  HRESULT
    CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
      UINT NumElements,
      const void* pShaderBytecodeWithInputSignature,
      SIZE_T BytecodeLength,
      ID3D11InputLayout** ppInputLayout);

  HRESULT
    CreatePixelShader(const void* pShaderBytecode,
      SIZE_T BytecodeLength,
      ID3D11ClassLinkage* pClassLinkage,
      ID3D11PixelShader** ppPixelShader);

  HRESULT
    CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
      const D3D11_SUBRESOURCE_DATA* pInitialData,
      ID3D11Buffer** ppBuffer);

  HRESULT
    CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
      ID3D11SamplerState** ppSamplerState);

public:
  /**
   * @brief Puntero al dispositivo Direct3D 11.
   * @details Se inicializa en init() y se libera en destroy().
   */
  ID3D11Device* m_device = nullptr;
};
