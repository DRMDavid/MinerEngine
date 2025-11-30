#include "BaseApp.h"
#include "ResourceManager.h"

// --- IMGUI INCLUDES ---
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

// Handler externo
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// --- ESTILO VISUAL ---
void SetupStyle() {
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 5.0f;
  style.FramePadding = ImVec2(6, 4);
  style.ItemSpacing = ImVec2(8, 6);

  // Tema oscuro estilo "Slate"
  ImVec4* colors = style.Colors;
  colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
  if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) return 0;
  if (FAILED(init())) return 0;

  MSG msg = {};
  LARGE_INTEGER freq, prev;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&prev);

  while (WM_QUIT != msg.message) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else {
      LARGE_INTEGER curr;
      QueryPerformanceCounter(&curr);
      float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
      prev = curr;
      update(deltaTime);
      render();
    }
  }
  return (int)msg.wParam;
}

HRESULT BaseApp::init() {
  HRESULT hr = S_OK;

  // 1. Inicializar Sistemas
  hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window); if (FAILED(hr)) return hr;
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM); if (FAILED(hr)) return hr;
  hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0); if (FAILED(hr)) return hr;
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT); if (FAILED(hr)) return hr;
  hr = m_viewport.init(m_window); if (FAILED(hr)) return hr;

  // 2. Cargar Actor
  m_Printstream = EU::MakeShared<Actor>(m_device);
  if (!m_Printstream.isNull()) {
    m_model = new Model3D("Assets/Desert.fbx", ModelType::FBX);
    m_Printstream->setMesh(m_device, m_model->GetMeshes());

    hr = m_PrintstreamAlbedo.init(m_device, "Assets/texture_16px 173", ExtensionType::PNG);
    std::vector<Texture> textures; textures.push_back(m_PrintstreamAlbedo);
    m_Printstream->setTextures(textures);

    m_Printstream->setName("Printstream");
    m_actors.push_back(m_Printstream);

    // --- VALORES INICIALES EXACTOS ---
    auto transform = m_Printstream->getComponent<Transform>();
    if (transform) {
      // Posición: -3.200, -4.000, 5.500
      transform->setPosition(EU::Vector3(-3.200f, -4.000f, 5.500f));

      // Rotación: -0.040, -4.660, 0.000
      transform->setRotation(EU::Vector3(-0.040f, -4.660f, 0.000f));

      // Escala: 1, 1, 1
      transform->setScale(EU::Vector3(1.0f, 1.0f, 1.0f));
    }
  }

  std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
  Layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
  Layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

  hr = m_shaderProgram.init(m_device, "MinerEngine.fx", Layout); if (FAILED(hr)) return hr;

  m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
  m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));

  XMVECTOR Eye = XMVectorSet(0.0f, 5.0f, -10.0f, 0.0f);
  XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  m_View = XMMatrixLookAtLH(Eye, At, Up);

  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

  // 3. Inicializar ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  SetupStyle();
  ImGui_ImplWin32_Init(m_window.m_hWnd);
  ImGui_ImplDX11_Init(m_device.m_device, m_deviceContext.m_deviceContext);

  return S_OK;
}

void BaseApp::update(float deltaTime) {
  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
  m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

  for (auto& actor : m_actors) {
    actor->update(deltaTime, m_deviceContext);
  }
}

// --- CONTROL PERSONALIZADO CORREGIDO (Sin funciones internas) ---
void DrawVec3Control(const std::string& label, float* values, float resetValue = 0.0f, float columnWidth = 100.0f) {
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text(label.c_str());
  ImGui::NextColumn();

  // Calculo manual del ancho disponible
  float availWidth = ImGui::GetContentRegionAvail().x;
  float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

  // Dividimos el espacio entre 3 (X, Y, Z)
  float fullItemWidth = (availWidth - 2 * itemSpacing) / 3.0f;

  // Definimos el tamaño del botón y del slider
  float buttonWidth = ImGui::GetFrameHeight(); // Botón cuadrado
  float dragWidth = fullItemWidth - buttonWidth - itemSpacing;
  if (dragWidth < 1.0f) dragWidth = 1.0f;

  // --- EJE X (Rojo) ---
  ImGui::PushID("X");
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
  if (ImGui::Button("X", ImVec2(buttonWidth, buttonWidth))) values[0] = resetValue;
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::SetNextItemWidth(dragWidth);
  ImGui::DragFloat("##v", &values[0], 0.1f, 0.0f, 0.0f, "%.3f");
  ImGui::PopID();

  ImGui::SameLine();

  // --- EJE Y (Verde) ---
  ImGui::PushID("Y");
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
  if (ImGui::Button("Y", ImVec2(buttonWidth, buttonWidth))) values[1] = resetValue;
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::SetNextItemWidth(dragWidth);
  ImGui::DragFloat("##v", &values[1], 0.1f, 0.0f, 0.0f, "%.3f");
  ImGui::PopID();

  ImGui::SameLine();

  // --- EJE Z (Azul) ---
  ImGui::PushID("Z");
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
  if (ImGui::Button("Z", ImVec2(buttonWidth, buttonWidth))) values[2] = resetValue;
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::SetNextItemWidth(dragWidth);
  ImGui::DragFloat("##v", &values[2], 0.1f, 0.0f, 0.0f, "%.3f");
  ImGui::PopID();

  ImGui::Columns(1);
  ImGui::PopID();
}

void BaseApp::render() {
  float ClearColor[4] = { 0.11f, 0.12f, 0.17f, 1.0f };
  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

  m_viewport.render(m_deviceContext);
  m_depthStencilView.render(m_deviceContext);
  m_shaderProgram.render(m_deviceContext);

  m_cbNeverChanges.render(m_deviceContext, 0, 1);
  m_cbChangeOnResize.render(m_deviceContext, 1, 1);

  for (auto& actor : m_actors) {
    actor->render(m_deviceContext);
  }

  // --- RENDER IMGUI ---
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

  ImGui::Begin("Inspector - MinerEngine");

  ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "TRANSFORMACION");
  ImGui::Separator();

  if (ImGui::CollapsingHeader("Propiedades del Actor", ImGuiTreeNodeFlags_DefaultOpen)) {
    for (size_t i = 0; i < m_actors.size(); ++i) {
      ImGui::PushID((int)i);
      std::string name = m_actors[i]->getName();

      if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
        auto transform = m_actors[i]->getComponent<Transform>();
        if (transform) {
          ImGui::Spacing();

          EU::Vector3 pos = transform->getPosition();
          EU::Vector3 rot = transform->getRotation();
          EU::Vector3 scale = transform->getScale();

          float p[3] = { pos.x, pos.y, pos.z };
          float r[3] = { rot.x, rot.y, rot.z };
          float s[3] = { scale.x, scale.y, scale.z };

          // Dibujar controles con colores
          DrawVec3Control("Posicion", p, 0.0f);
          DrawVec3Control("Rotacion", r, 0.0f);
          DrawVec3Control("Escala", s, 1.0f);

          ImGui::Spacing();

          transform->setPosition(EU::Vector3(p[0], p[1], p[2]));
          transform->setRotation(EU::Vector3(r[0], r[1], r[2]));
          transform->setScale(EU::Vector3(s[0], s[1], s[2]));
        }
        ImGui::TreePop();
      }
      ImGui::PopID();
    }
  }
  ImGui::End();

  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  m_swapChain.present();
}

void BaseApp::destroy() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

  m_cbNeverChanges.destroy();
  m_cbChangeOnResize.destroy();
  m_shaderProgram.destroy();
  m_depthStencil.destroy();
  m_depthStencilView.destroy();
  m_renderTargetView.destroy();
  m_swapChain.destroy();
  m_backBuffer.destroy();
  m_deviceContext.destroy();
  m_device.destroy();
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    return true;

  switch (message) {
  case WM_DESTROY: PostQuitMessage(0); return 0;
  default: return DefWindowProc(hWnd, message, wParam, lParam);
  }
}