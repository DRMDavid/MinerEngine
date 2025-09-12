#include "DeviceContext.h"

void DeviceContext::OMSetRenderTargets(
  unsigned int NumViews,
  ID3D11RenderTargetView* const* ppRenderTargetViews,
  ID3D11DepthStencilView* pDepthStencilView)
{
  // Validar los parámetros de entrada
  if (!ppRenderTargetViews && !pDepthStencilView) {
    ERROR("DeviceContext", "OMSetRenderTargets",
      "Both ppRenderTargetViews and pDepthStencilView are nullptr");
    return;
  }

  if (NumViews > 0 && !ppRenderTargetViews) {
    ERROR("DeviceContext", "OMSetRenderTargets",
      "ppRenderTargetViews is nullptr, but NumViews > 0");
    return;
  }

  // Asignar los render targets y el depth stencil
  m_deviceContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
}

void DeviceContext::RSSetViewports(unsigned int NumViewports,
                                   const D3D11_VIEWPORT* pViewports)
{
  // Validar los parámetros de entrada
  if (NumViewports == 0) {
    ERROR("DeviceContext", "RSSetViewports", "NumViewports is 0");
    return;
  }
  if (!pViewports) {
    ERROR("DeviceContext", "RSSetViewports", "pViewports is nullptr");
    return;
  }
  // Asignar los viewports
  m_deviceContext->RSSetViewports(NumViewports, pViewports);
}
