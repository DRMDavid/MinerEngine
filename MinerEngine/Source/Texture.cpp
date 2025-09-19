#include "Texture.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT 
Texture::init(Device& device, 
              const std::string& textureName, 
              ExtensionType extensionType) {
  return E_NOTIMPL;
}

HRESULT 
Texture::init(Device& device, 
              unsigned int width, 
              unsigned int height, 
              DXGI_FORMAT Format, 
              unsigned int BindFlags, 
              unsigned int sampleCount, 
              unsigned int qualityLevels) {
  if (!device.m_device) {
    ERROR("Texture", "init", "Device is null.");
    return E_POINTER;
  }
  if (width == 0 || height == 0) {
    ERROR("Texture", "init", "Width and height must be greater than 0");
    E_INVALIDARG;
  }

  // Config the texture
  D3D11_TEXTURE2D_DESC desc;
  memset(&desc, 0, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT hr = device.CreateTexture2D(&desc, nullptr, &m_texture);

  if (FAILED(hr)) {
    ERROR("Texture", "init",
      ("Failed to create texture with specified params. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}
