/**
 * @file DeferredRenderer.cpp
 * @brief Implementa la logica de DeferredRenderer dentro del subsistema Rendering.
 * @ingroup rendering
 */

#include "Rendering/DeferredRenderer.h"
#include <algorithm>
#include <cmath>
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"

namespace {
    constexpr unsigned int kGBufferTargetCount = 4;

    struct RenderTargetViewAccess {
        ID3D11RenderTargetView* m_renderTargetView = nullptr;
    };

    struct EditorViewportPassAccess {
        Texture m_colorTexture;
        Texture m_colorSRV;
        RenderTargetView m_rtv;
        Texture m_depthTexture;
        DepthStencilView m_dsv;
        unsigned int m_width = 1;
        unsigned int m_height = 1;
    };

    ID3D11RenderTargetView* ResolveViewportRTV(EditorViewportPass& pass) {
        EditorViewportPassAccess* access = reinterpret_cast<EditorViewportPassAccess*>(&pass);
        RenderTargetViewAccess* rtvAccess = reinterpret_cast<RenderTargetViewAccess*>(&access->m_rtv);
        return rtvAccess->m_renderTargetView;
    }

    ID3D11DepthStencilView* ResolveViewportDSV(EditorViewportPass& pass) {
        EditorViewportPassAccess* access = reinterpret_cast<EditorViewportPassAccess*>(&pass);
        return access->m_dsv.m_depthStencilView;
    }

    ID3D11RenderTargetView* ResolveRTV(RenderTargetView& view) {
        RenderTargetViewAccess* access = reinterpret_cast<RenderTargetViewAccess*>(&view);
        return access->m_renderTargetView;
    }
}

const LightData*
findPrimaryShadowLight(const RenderScene& scene) {
    for (const LightData& light : scene.directionalLights) {
        if (light.type == LightType::Directional) {
            return &light;
        }
    }

    return scene.directionalLights.empty() ? nullptr : &scene.directionalLights.front();
}

void
writeLightToFrameBuffer(CBPerFrame& buffer, int lightIndex, const LightData& light) {
    const float range = light.range > 0.0f ? light.range : 10.0f;
    const EU::Vector3 lightColor = light.color * light.intensity;
    buffer.LightPositionsRanges[lightIndex] = XMFLOAT4(light.position.x, light.position.y, light.position.z, range);
    buffer.LightColorsTypes[lightIndex] = XMFLOAT4(lightColor.x, lightColor.y, lightColor.z, static_cast<float>(static_cast<int>(light.type)));
    buffer.LightDirectionsIntensities[lightIndex] = XMFLOAT4(light.direction.x, light.direction.y, light.direction.z, light.intensity);
}

HRESULT
DeferredRenderer::init(Device& device) {
    HRESULT hr = m_perFrameBuffer.init(device, sizeof(CBPerFrame));
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_perObjectBuffer.init(device, sizeof(CBPerObject));
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_disabledDepthStencil.init(device,
        false,
        D3D11_DEPTH_WRITE_MASK_ZERO,
        D3D11_COMPARISON_ALWAYS);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_shadowDepthStencil.init(device,
        true,
        D3D11_DEPTH_WRITE_MASK_ALL,
        D3D11_COMPARISON_LESS);
    if (FAILED(hr)) {
        return hr;

        hr = createShadowResources(device);
        if (FAILED(hr)) {
            return hr;
        }

        hr = m_preShadowDebugPass.init(device, m_renderWidth, m_renderHeight);
        if (FAILED(hr)) {
            return hr;
        }

        hr = createGBufferResources(device, m_renderWidth, m_renderHeight);
        if (FAILED(hr)) {
            return hr;
        }

        hr = createLightingResources(device);
        if (FAILED(hr)) {
            return hr;
        }