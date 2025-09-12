#pragma once
#include "Prerequisites.h"

class
	DeviceContext {

public:
	DeviceContext() = default;
	~DeviceContext() = default;

	void
		init();

	void
		update();

	void
		render();

	void
		destroy();

void 
OMSetRenderTargets(unsigned int NumViews,
		               ID3D11RenderTargetView* const* ppRenderTargetViews,
		               ID3D11DepthStencilView* pDepthStencilView);


void 
RSSetViewports(unsigned int NumViewports,
               const D3D11_VIEWPORT* pViewports);



private:

	ID3D11DeviceContext* m_deviceContext = nullptr;
};