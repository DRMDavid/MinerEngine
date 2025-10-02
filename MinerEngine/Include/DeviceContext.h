#pragma once
#include "Prerequisites.h"

/**
 * @class DeviceContext
 * @brief Administra el contexto inmediato de Direct3D 11.
 *
 * Proporciona funciones para configurar el pipeline de renderizado,
 * asignar recursos, limpiar buffers y ejecutar comandos de dibujo.
 * Sirve como interfaz entre el CPU y la GPU para la emisión de comandos.
 */
class DeviceContext {
public:
  /**
   * @brief Construye un contexto vacío.
   *
   * No se inicializa ningún recurso hasta llamar a @c init().
   */
  DeviceContext() = default;

  /**
   * @brief Destructor trivial.
   *
   * No libera recursos automáticamente; se recomienda llamar a @c destroy().
   */
  ~DeviceContext() = default;

  /** @brief Inicializa el contexto inmediato de Direct3D 11. */
  void init();

  /** @brief Actualiza parámetros internos (placeholder para futuras funciones). */
  void update();

  /** @brief Ejecuta rutinas de renderizado (actualmente placeholder). */
  void render();

  /** @brief Libera el contexto y establece @c m_deviceContext en nullptr. */
  void destroy();

  /** @brief Configura uno o varios viewports en la etapa de rasterización. */
  void RSSetViewports(unsigned int NumViewports,
                      const D3D11_VIEWPORT* pViewports);

  /** @brief Asigna Shader Resource Views a la etapa de Pixel Shader. */
  void PSSetShaderResources(unsigned int StartSlot,
                            unsigned int NumViews,
                            ID3D11ShaderResourceView* const* ppShaderResourceViews);

  /** @brief Define el Input Layout activo para la etapa de ensamblado de entrada. */
  void IASetInputLayout(ID3D11InputLayout* pInputLayout);

  /** @brief Asigna un Vertex Shader al pipeline. */
  void VSSetShader(ID3D11VertexShader* pVertexShader,
                   ID3D11ClassInstance* const* ppClassInstances,
                   unsigned int NumClassInstances);

  /** @brief Asigna un Pixel Shader al pipeline. */
  void PSSetShader(ID3D11PixelShader* pPixelShader,
                   ID3D11ClassInstance* const* ppClassInstances,
                   unsigned int NumClassInstances);

  /** @brief Copia datos desde CPU a un recurso en GPU. */
  void UpdateSubresource(ID3D11Resource* pDstResource,
                         unsigned int DstSubresource,
                         const D3D11_BOX* pDstBox,
                         const void* pSrcData,
                         unsigned int SrcRowPitch,
                         unsigned int SrcDepthPitch);

  /** @brief Asigna buffers de vértices al Input Assembler. */
  void IASetVertexBuffers(unsigned int StartSlot,
                          unsigned int NumBuffers,
                          ID3D11Buffer* const* ppVertexBuffers,
                          const unsigned int* pStrides,
                          const unsigned int* pOffsets);

  /** @brief Asigna un Index Buffer al Input Assembler. */
  void IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
                        DXGI_FORMAT Format,
                        unsigned int Offset);

  /** @brief Asigna Sampler States a la etapa de Pixel Shader. */
  void PSSetSamplers(unsigned int StartSlot,
                     unsigned int NumSamplers,
                     ID3D11SamplerState* const* ppSamplers);

  /** @brief Configura el Rasterizer State actual. */
  void RSSetState(ID3D11RasterizerState* pRasterizerState);

  /** @brief Asigna un Blend State al Output Merger. */
  void OMSetBlendState(ID3D11BlendState* pBlendState,
    const float BlendFactor[4],
    unsigned int SampleMask);

  /** @brief Configura los Render Targets y Depth Stencil Views. */
  void OMSetRenderTargets(unsigned int NumViews,
                          ID3D11RenderTargetView* const* ppRenderTargetViews,
                          ID3D11DepthStencilView* pDepthStencilView);

  /** @brief Define la topología de primitivas para el Input Assembler. */
  void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

  /** @brief Limpia un Render Target con un color especificado. */
  void ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
                             const float ColorRGBA[4]);

  /** @brief Limpia un Depth Stencil View. */
  void ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
                             unsigned int ClearFlags,
                             float Depth,
                             UINT8 Stencil);

  /** @brief Asigna constant buffers al Vertex Shader. */
  void VSSetConstantBuffers(unsigned int StartSlot,
                            unsigned int NumBuffers,
                            ID3D11Buffer* const* ppConstantBuffers);

  /** @brief Asigna constant buffers al Pixel Shader. */
  void PSSetConstantBuffers(unsigned int StartSlot,
                            unsigned int NumBuffers,
                            ID3D11Buffer* const* ppConstantBuffers);

  /** @brief Envía un comando de dibujado de primitivas indexadas. */
  void DrawIndexed(unsigned int IndexCount,
                   unsigned int StartIndexLocation,
                   int BaseVertexLocation);

public:
  /** @brief Puntero al contexto inmediato de Direct3D 11. */
  ID3D11DeviceContext* m_deviceContext = nullptr;
};
