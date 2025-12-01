#include "BaseApp.h"
#include "ResourceManager.h"
#include <algorithm> // Para min/max
#include <vector>
#include <string>

// --- IMGUI INCLUDES ---
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

// Handler externo
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---------------------------------------------------------
// VARIABLES GLOBALES Y SISTEMAS
// ---------------------------------------------------------

// --- ESTADO DE LA SELECCIÓN ---
static int g_SelectedActorIndex = -1;
static bool g_IsDragging = false;

// --- FISICA DE ROTACIÓN (SUAVIZADO) ---
static EU::Vector3 g_TargetRotation = EU::Vector3(0, 0, 0); // Hacia donde queremos ir
static float g_RotationSensitivity = 0.5f;                  // Velocidad del mouse

// --- SISTEMA DE NOTIFICACIONES (TOASTS) ---
struct Notification {
  std::string message;
  float timer;
  ImVec4 color;
};
static std::vector<Notification> g_Notifications;

// Función para lanzar un mensaje flotante
void AddNotification(const std::string& msg, ImVec4 color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f)) {
  g_Notifications.push_back({ msg, 3.0f, color }); // El mensaje vive 3 segundos
}

// ---------------------------------------------------------
// ESTILOS Y HELPERS DE UI
// ---------------------------------------------------------

// --- ESTILO "CYBER GLASS" (FUTURISTA) ---
void SetupStyle() {
  ImGuiStyle& style = ImGui::GetStyle();

  // Redondeo extremo para look moderno
  style.WindowRounding = 12.0f;
  style.ChildRounding = 10.0f;
  style.FrameRounding = 12.0f;
  style.GrabRounding = 12.0f;
  style.PopupRounding = 10.0f;
  style.ScrollbarRounding = 12.0f;

  style.WindowBorderSize = 0.0f; // Sin bordes duros
  style.FramePadding = ImVec2(10, 8);
  style.ItemSpacing = ImVec2(8, 10);

  // Paleta de Colores
  ImVec4* colors = style.Colors;

  // Fondo semitransparente (Glassmorphism)
  colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.85f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);

  // Textos
  colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

  // Acentos (Violeta Neón)
  ImVec4 accentColor = ImVec4(0.60f, 0.10f, 0.90f, 1.00f);
  ImVec4 accentHover = ImVec4(0.70f, 0.20f, 1.00f, 1.00f);

  colors[ImGuiCol_Header] = ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.7f);
  colors[ImGuiCol_HeaderHovered] = accentHover;
  colors[ImGuiCol_HeaderActive] = accentColor;

  colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f); // Botones sutiles
  colors[ImGuiCol_ButtonHovered] = accentHover;
  colors[ImGuiCol_ButtonActive] = accentColor;

  colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
}

// --- CONTROLES DE VECTOR (ESTILO UNITY - FIX MANUAL) ---
void DrawVec3Control(const std::string& label, float* values, float resetValue = 0.0f, float columnWidth = 100.0f) {
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text(label.c_str());
  ImGui::NextColumn();

  // --- CÁLCULO MANUAL DEL ANCHO ---
  float availWidth = ImGui::GetContentRegionAvail().x;
  float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
  // Dividimos el espacio en 3
  float fullItemWidth = (availWidth - (itemSpacing * 2)) / 3.0f;
  float buttonSize = ImGui::GetFrameHeight();
  float dragWidth = fullItemWidth - buttonSize;
  if (dragWidth < 1.0f) dragWidth = 1.0f;

  // X (Rojo)
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
  if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) values[0] = resetValue;
  ImGui::PopStyleColor(3);

  ImGui::SameLine(0, 0);
  ImGui::SetNextItemWidth(dragWidth);
  ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");

  ImGui::SameLine();

  // Y (Verde)
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
  if (ImGui::Button("Y", ImVec2(buttonSize, buttonSize))) values[1] = resetValue;
  ImGui::PopStyleColor(3);

  ImGui::SameLine(0, 0);
  ImGui::SetNextItemWidth(dragWidth);
  ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");

  ImGui::SameLine();

  // Z (Azul)
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
  if (ImGui::Button("Z", ImVec2(buttonSize, buttonSize))) values[2] = resetValue;
  ImGui::PopStyleColor(3);

  ImGui::SameLine(0, 0);
  ImGui::SetNextItemWidth(dragWidth);
  ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");

  ImGui::Columns(1);
  ImGui::PopID();
}

// ---------------------------------------------------------
// MATEMÁTICAS (RAYCASTING)
// ---------------------------------------------------------
bool RaySphereIntersect(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR sphereCenter, float sphereRadius, float& outDist) {
  XMVECTOR L = XMVectorSubtract(sphereCenter, rayOrigin);
  XMVECTOR tcaVec = XMVector3Dot(L, rayDir);
  float tca = XMVectorGetX(tcaVec);
  if (tca < 0) return false;
  XMVECTOR d2Vec = XMVector3Dot(L, L) - (tcaVec * tcaVec);
  float d2 = XMVectorGetX(d2Vec);
  float radius2 = sphereRadius * sphereRadius;
  if (d2 > radius2) return false;
  float thc = sqrt(radius2 - d2);
  outDist = tca - thc;
  return true;
}

// ---------------------------------------------------------
// CICLO DE VIDA DE LA APP
// ---------------------------------------------------------

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

  // Inicialización DirectX Standard
  hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window); if (FAILED(hr)) return hr;
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM); if (FAILED(hr)) return hr;
  hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0); if (FAILED(hr)) return hr;
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT); if (FAILED(hr)) return hr;
  hr = m_viewport.init(m_window); if (FAILED(hr)) return hr;

  // --- CARGA DE ACTOR DE EJEMPLO ---
  m_Printstream = EU::MakeShared<Actor>(m_device);
  if (!m_Printstream.isNull()) {
    m_model = new Model3D("Assets/Desert.fbx", ModelType::FBX);
    m_Printstream->setMesh(m_device, m_model->GetMeshes());

    hr = m_PrintstreamAlbedo.init(m_device, "Assets/texture_16px 197", ExtensionType::PNG);
    std::vector<Texture> textures; textures.push_back(m_PrintstreamAlbedo);
    m_Printstream->setTextures(textures);

    m_Printstream->setName("Desert Printstream");
    m_actors.push_back(m_Printstream);

    // VALORES ORIGINALES (Se usaran tambien en el reset)
    auto transform = m_Printstream->getComponent<Transform>();
    if (transform) {
      transform->setPosition(EU::Vector3(-3.200f, -4.000f, 5.500f));
      transform->setRotation(EU::Vector3(-0.040f, -4.660f, 0.000f));
      transform->setScale(EU::Vector3(1.0f, 1.0f, 1.0f));
    }
  }

  // Shaders y Buffers
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

  // --- INICIALIZAR IMGUI CON NUESTRO ESTILO ---
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  SetupStyle(); // <--- Aplicar Estilo Neon Glass

  ImGui_ImplWin32_Init(m_window.m_hWnd);
  ImGui_ImplDX11_Init(m_device.m_device, m_deviceContext.m_deviceContext);

  AddNotification("Bienvenido a MinerEngine", ImVec4(0.6f, 0.1f, 0.9f, 1.0f));

  return S_OK;
}

void BaseApp::update(float deltaTime) {
  // Actualizar Constantes
  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
  m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

  // -----------------------------------------------------------
  // INPUT Y LÓGICA DE SELECCIÓN
  // -----------------------------------------------------------
  ImGuiIO& io = ImGui::GetIO();

  // Solo procesamos input 3D si el mouse no está sobre una ventana de ImGui
  if (!io.WantCaptureMouse) {

    // --- SELECCIÓN (CLICK IZQUIERDO) ---
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      POINT mousePos; GetCursorPos(&mousePos); ScreenToClient(m_window.m_hWnd, &mousePos);
      float mouseX = (float)mousePos.x;
      float mouseY = (float)mousePos.y;

      XMVECTOR mouseNear = XMVectorSet(mouseX, mouseY, 0.0f, 0.0f);
      XMVECTOR mouseFar = XMVectorSet(mouseX, mouseY, 1.0f, 0.0f);
      XMVECTOR rayOrigin = XMVector3Unproject(mouseNear, 0, 0, m_window.m_width, m_window.m_height, 0.0f, 1.0f, m_Projection, m_View, XMMatrixIdentity());
      XMVECTOR rayEnd = XMVector3Unproject(mouseFar, 0, 0, m_window.m_width, m_window.m_height, 0.0f, 1.0f, m_Projection, m_View, XMMatrixIdentity());
      XMVECTOR rayDir = XMVector3Normalize(rayEnd - rayOrigin);

      int hitIndex = -1;
      float closestDist = FLT_MAX;

      for (size_t i = 0; i < m_actors.size(); ++i) {
        auto transform = m_actors[i]->getComponent<Transform>();
        if (transform) {
          EU::Vector3 pos = transform->getPosition();
          XMVECTOR center = XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
          float avgScale = (transform->getScale().x + transform->getScale().y + transform->getScale().z) / 3.0f;
          float radius = 15.0f * avgScale;

          float dist = 0.0f;
          if (RaySphereIntersect(rayOrigin, rayDir, center, radius, dist)) {
            if (dist < closestDist) {
              closestDist = dist;
              hitIndex = (int)i;
            }
          }
        }
      }

      g_SelectedActorIndex = hitIndex;

      // Al seleccionar, actualizamos el Target para evitar saltos bruscos
      if (hitIndex != -1) {
        g_IsDragging = true;
        auto t = m_actors[hitIndex]->getComponent<Transform>();
        if (t) g_TargetRotation = t->getRotation();
      }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      g_IsDragging = false;
    }

    // --- ARRASTRE Y ROTACIÓN ---
    if (g_IsDragging && g_SelectedActorIndex != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      // Factor base reducido, multiplicado por la sensibilidad global
      float sensitivity = 0.01f * g_RotationSensitivity;

      // [CORRECCIÓN] Usamos += para que la rotación siga al mouse naturalmente
      g_TargetRotation.y += io.MouseDelta.x * sensitivity;
      g_TargetRotation.x += io.MouseDelta.y * sensitivity;
    }
  }

  // --- APLICAR FISICA DE ROTACIÓN (LERP) ---
  // Esto hace que el objeto "vuele" suavemente hacia la rotación objetivo
  if (g_SelectedActorIndex != -1 && g_SelectedActorIndex < m_actors.size()) {
    auto transform = m_actors[g_SelectedActorIndex]->getComponent<Transform>();
    if (transform) {
      EU::Vector3 currentRot = transform->getRotation();

      float smoothSpeed = 10.0f * deltaTime; // Ajustar suavidad aquí

      currentRot.x += (g_TargetRotation.x - currentRot.x) * smoothSpeed;
      currentRot.y += (g_TargetRotation.y - currentRot.y) * smoothSpeed;
      currentRot.z += (g_TargetRotation.z - currentRot.z) * smoothSpeed;

      transform->setRotation(currentRot);
    }
  }

  // Actualizar lógica interna de los actores
  for (auto& actor : m_actors) {
    actor->update(deltaTime, m_deviceContext);
  }
}

void BaseApp::render() {
  // 1. ESCENA 3D (Fondo Oscuro para resaltar el Neón)
  float ClearColor[4] = { 0.05f, 0.05f, 0.07f, 1.0f };
  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

  m_viewport.render(m_deviceContext);
  m_depthStencilView.render(m_deviceContext);
  m_shaderProgram.render(m_deviceContext);
  m_cbNeverChanges.render(m_deviceContext, 0, 1);
  m_cbChangeOnResize.render(m_deviceContext, 1, 1);

  for (auto& actor : m_actors) {
    actor->render(m_deviceContext);
  }

  // 2. INTERFAZ (HUD)
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGuiIO& io = ImGui::GetIO();
  float screenW = (float)m_window.m_width;
  float screenH = (float)m_window.m_height;

  // --- A. TOOLBAR FLOTANTE (ARRIBA CENTRO) ---
  ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, 20.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
  ImGui::SetNextWindowBgAlpha(0.6f);
  ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav;

  if (ImGui::Begin("Toolbar", nullptr, toolbarFlags)) {
    ImGui::TextColored(ImVec4(0.6f, 0.1f, 0.9f, 1.0f), "MINER ENGINE");
  }
  ImGui::End();

  // --- B. INSPECTOR FLOTANTE (DERECHA) ---
  ImGui::SetNextWindowPos(ImVec2(screenW - 20, 80), ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
  ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse)) {

    // Lista de Objetos
    ImGui::Text("JERARQUIA");
    ImGui::Separator();
    ImGui::BeginChild("List", ImVec2(0, 150), true);
    for (size_t i = 0; i < m_actors.size(); ++i) {
      bool isSelected = (g_SelectedActorIndex == (int)i);
      if (ImGui::Selectable(m_actors[i]->getName().c_str(), isSelected)) {
        g_SelectedActorIndex = (int)i;
        // Sincronizar rotación objetivo al seleccionar
        auto t = m_actors[i]->getComponent<Transform>();
        if (t) g_TargetRotation = t->getRotation();
      }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Text("PROPIEDADES");
    ImGui::Separator();

    if (g_SelectedActorIndex != -1) {
      auto actor = m_actors[g_SelectedActorIndex];
      auto transform = actor->getComponent<Transform>();

      if (transform) {
        EU::Vector3 pos = transform->getPosition();
        EU::Vector3 rot = transform->getRotation();
        EU::Vector3 scale = transform->getScale();

        float p[3] = { pos.x, pos.y, pos.z };
        float r[3] = { rot.x, rot.y, rot.z };
        float s[3] = { scale.x, scale.y, scale.z };

        DrawVec3Control("Pos", p);
        DrawVec3Control("Rot", r);
        // Si editamos rotación a mano, actualizamos el Target para no pelear
        if (r[0] != rot.x || r[1] != rot.y || r[2] != rot.z) g_TargetRotation = EU::Vector3(r[0], r[1], r[2]);
        DrawVec3Control("Scl", s, 1.0f);

        transform->setPosition(EU::Vector3(p[0], p[1], p[2]));
        transform->setRotation(EU::Vector3(r[0], r[1], r[2]));
        transform->setScale(EU::Vector3(s[0], s[1], s[2]));

        ImGui::Spacing();

        // --- BOTÓN RESET CORREGIDO ---
        if (ImGui::Button("Reset Original", ImVec2(-1, 30))) {
          // Valores tomados de init()
          EU::Vector3 originalPos(-3.200f, -4.000f, 5.500f);
          EU::Vector3 originalRot(-0.040f, -4.660f, 0.000f);
          EU::Vector3 originalScl(1.0f, 1.0f, 1.0f);

          transform->setPosition(originalPos);
          transform->setRotation(originalRot);
          transform->setScale(originalScl);

          // IMPORTANTE: Actualizar target para que no "regrese" por el lerp
          g_TargetRotation = originalRot;

          AddNotification("Valores Originales Restaurados", ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        }
      }
    }
    else {
      ImGui::TextDisabled("Selecciona un objeto para editar.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("Configuración Input");
    ImGui::SliderFloat("Sensibilidad", &g_RotationSensitivity, 0.1f, 5.0f);
  }
  ImGui::End();

  // --- C. RENDERIZADO DE NOTIFICACIONES (TOASTS) ---
  float toastY = screenH - 50.0f;
  for (int i = 0; i < g_Notifications.size(); ) {
    Notification& n = g_Notifications[i];

    ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, toastY), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowBgAlpha(n.timer / 3.0f * 0.9f); // Fade out alpha

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f); // Cápsula
    ImGui::Begin(std::string("Toast" + std::to_string(i)).c_str(), nullptr, toolbarFlags | ImGuiWindowFlags_NoInputs);
    ImGui::TextColored(n.color, "%s", n.message.c_str());
    ImGui::End();
    ImGui::PopStyleVar();

    n.timer -= ImGui::GetIO().DeltaTime;
    toastY -= 45.0f; // Stack hacia arriba

    if (n.timer <= 0.0f) {
      g_Notifications.erase(g_Notifications.begin() + i);
    }
    else {
      ++i;
    }
  }

  // --- D. ESTADÍSTICAS ---
  ImGui::SetNextWindowPos(ImVec2(10, screenH - 10), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
  ImGui::SetNextWindowBgAlpha(0.3f);
  if (ImGui::Begin("Stats", nullptr, toolbarFlags | ImGuiWindowFlags_NoInputs)) {
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Actores: %d", (int)m_actors.size());
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