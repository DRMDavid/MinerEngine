#include "GUI/GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS/Actor.h"
#include "EngineUtilities/Utilities/Camera.h"
#include <string>
#include <vector>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

void GUI::awake() {}

void GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Estilo Morado "Deep Nebula"
	appleLiquidStyle(0.95f, ImVec4(0.55f, 0.35f, 0.90f, 1.0f));

	ImGui_ImplWin32_Init(window.m_hWnd);
	ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

	toolTipData();
	selectedActorIndex = 0;
}

void GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	style.WindowRounding = 8.0f;
	style.ChildRounding = 6.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 6.0f;
	style.TabRounding = 6.0f;
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 12.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	const ImVec4 bgDeep = ImVec4(0.10f, 0.08f, 0.14f, opacity);
	const ImVec4 bgMid = ImVec4(0.16f, 0.14f, 0.22f, opacity);
	const ImVec4 purpleLight = ImVec4(0.24f, 0.18f, 0.35f, opacity);
	const ImVec4 purpleElect = ImVec4(0.55f, 0.35f, 0.90f, 1.00f);
	const ImVec4 textMain = ImVec4(0.90f, 0.88f, 0.95f, 1.00f);

	colors[ImGuiCol_Text] = textMain;
	colors[ImGuiCol_WindowBg] = bgDeep;
	colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0.1f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.10f, 0.16f, 0.95f);
	colors[ImGuiCol_Border] = ImVec4(0.30f, 0.25f, 0.40f, 0.50f);
	colors[ImGuiCol_FrameBg] = bgMid;
	colors[ImGuiCol_FrameBgHovered] = purpleLight;
	colors[ImGuiCol_FrameBgActive] = purpleElect;
	colors[ImGuiCol_TitleBg] = bgDeep;
	colors[ImGuiCol_TitleBgActive] = bgMid;
	colors[ImGuiCol_MenuBarBg] = bgDeep;
	colors[ImGuiCol_Button] = bgMid;
	colors[ImGuiCol_ButtonHovered] = purpleLight;
	colors[ImGuiCol_ButtonActive] = purpleElect;
	colors[ImGuiCol_Header] = ImVec4(purpleElect.x, purpleElect.y, purpleElect.z, 0.30f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(purpleElect.x, purpleElect.y, purpleElect.z, 0.50f);
	colors[ImGuiCol_HeaderActive] = purpleElect;
	colors[ImGuiCol_Tab] = bgMid;
	colors[ImGuiCol_TabHovered] = purpleElect;
	colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.25f, 0.50f, 1.0f);
	colors[ImGuiCol_CheckMark] = purpleElect;
	colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.30f, 0.80f, 1.0f);
	colors[ImGuiCol_SliderGrabActive] = purpleElect;
	colors[ImGuiCol_DockingPreview] = ImVec4(purpleElect.x, purpleElect.y, purpleElect.z, 0.60f);
}

void GUI::update(Viewport& viewport, Window& window) {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetOrthographic(false);

	drawStudioTopRibbon();
	drawEditorDockspace();
	closeApp();
	drawGizmoToolbar();
}

void GUI::render() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void GUI::destroy() {
	if (ImGui::GetCurrentContext() == nullptr) return;
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// =================================================================================
// 1. MEJORA: CONTROLES XYZ ESTILIZADOS (ROJO, VERDE, AZUL) ESTILO UNITY/UNREAL
// =================================================================================
void GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0]; // Asume que la fuente 0 es la principal

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 2.0f, 0.0f }); // Espaciado más ajustado

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// BOTÓN X (Rojo)
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.7f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.3f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.6f, 0.1f, 0.1f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	// BOTÓN Y (Verde)
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.3f, 0.6f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.4f, 0.7f, 0.4f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.5f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	// BOTÓN Z (Azul)
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.4f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.5f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.3f, 0.7f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);
	ImGui::PopID();
}

void GUI::toolTipData() {}

void GUI::ToolBar() {}

void GUI::closeApp() {
	if (show_exit_popup) {
		ImGui::OpenPopup("Exit?");
		show_exit_popup = false;
	}
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Estas a punto de salir de MinerEngine.\n¿Estas seguro?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			exit(0);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

// =================================================================================
// 3. MEJORA: ESTRUCTURA DEL INSPECTOR Y PROPIEDADES DE RENDER/SOMBRAS
// =================================================================================
void GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
	ImGui::Begin("Inspector");

	if (!actor) {
		ImGui::TextDisabled("No actor selected");
		ImGui::End();
		return;
	}

	// Sección de Cabecera (Nombre y Tags)
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.10f, 0.16f, 0.5f));
	ImGui::BeginChild("HeaderRegion", ImVec2(0, 95), true);

	bool isStatic = false;
	ImGui::Checkbox("##Static", &isStatic);
	ImGui::SameLine();

	char objectName[128];
	strcpy_s(objectName, sizeof(objectName), actor->getName().c_str());
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
	if (ImGui::InputText("##ObjectName", objectName, IM_ARRAYSIZE(objectName))) {
		actor->setName(std::string(objectName));
	}

	ImGui::SameLine();
	ImGui::Button("Icon", ImVec2(30, 0));

	ImGui::Spacing();

	const char* tags[] = { "Untagged", "Player", "Enemy", "Environment" };
	static int currentTag = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
	ImGui::Combo("Tag", &currentTag, tags, IM_ARRAYSIZE(tags));
	ImGui::SameLine();

	const char* layers[] = { "Default", "TransparentFX", "Ignore Raycast", "Water", "UI" };
	static int currentLayer = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::Combo("Layer", &currentLayer, layers, IM_ARRAYSIZE(layers));

	ImGui::EndChild();
	ImGui::PopStyleColor();

	ImGui::Spacing();

	// Componente Transform
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		inspectorContainer(actor);
	}

	ImGui::Spacing();

	// Componente de Rendering (Sombras)
	if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent(10.0f);
		bool castShadow = actor->canCastShadow();
		if (ImGui::Checkbox("Cast Shadows", &castShadow)) {
			actor->setCastShadow(castShadow);
		}
		ImGui::TextDisabled("Configuraciones de luz y material...");
		ImGui::Unindent(10.0f);
	}

	ImGui::End();
}

void GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	if (!actor) return;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

	// Actualizamos los valores de la posición usando las funciones nativas de tu motor
	vec3Control("Position", const_cast<float*>(transform->getPosition().data()), 0.0f, 75.0f);
	vec3Control("Rotation", const_cast<float*>(transform->getRotation().data()), 0.0f, 75.0f);
	vec3Control("Scale", const_cast<float*>(transform->getScale().data()), 1.0f, 75.0f);
}

// =================================================================================
// 2. MEJORA: MENÚ CONTEXTUAL (CLICK DERECHO) EN EL OUTLINER
// =================================================================================
void GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	static ImGuiTextFilter filter;
	filter.Draw("Search...", ImGui::GetContentRegionAvail().x);

	ImGui::Separator();

	for (int i = 0; i < (int)actors.size(); ++i) {
		const auto& actor = actors[i];
		if (!actor) continue;

		std::string actorName = actor->getName();
		if (!filter.PassFilter(actorName.c_str())) continue;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (selectedActorIndex == i) flags |= ImGuiTreeNodeFlags_Selected;

		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			selectedActorIndex = i;
		}

		// EL MENÚ CONTEXTUAL MÁGICO
		if (ImGui::BeginPopupContextItem()) {
			selectedActorIndex = i; // Seleccionarlo también al dar click derecho
			ImGui::TextDisabled("Actor Options");
			ImGui::Separator();
			if (ImGui::MenuItem("Rename...")) { /* Lógica futura */ }
			if (ImGui::MenuItem("Duplicate", "Ctrl+D")) { /* Lógica futura */ }
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); // Texto rojo
			if (ImGui::MenuItem("Delete", "Del")) { /* Lógica futura */ }
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		if (nodeOpen) {
			auto transform = actor->getComponent<Transform>();
			if (transform) {
				ImGui::TextDisabled(" Pos: %.1f, %.1f, %.1f",
					transform->getPosition().x,
					transform->getPosition().y,
					transform->getPosition().z);
			}
			ImGui::TreePop();
		}
	}

	// Espacio vacío para deseleccionar o click derecho global
	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
		if (ImGui::MenuItem("Create Empty Actor")) { /* Lógica futura */ }
		if (ImGui::MenuItem("Create 3D Object")) { /* Lógica futura */ }
		ImGui::EndPopup();
	}

	ImGui::End();
}

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor) {
	if (!actor) return;
	static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

	float rectX = m_viewportPos.x;
	float rectY = m_viewportPos.y;
	float rectW = m_viewportSize.x;
	float rectH = m_viewportSize.y;

	if (rectW < 64.0f || rectH < 64.0f) {
		m_isUsingGizmo = false;
		return;
	}

	float* pos = const_cast<float*>(transform->getPosition().data());
	float* rot = const_cast<float*>(transform->getRotation().data());
	float* sca = const_cast<float*>(transform->getScale().data());

	float mArr[16];
	ImGuizmo::RecomposeMatrixFromComponents(pos, rot, sca, mArr);

	float vArr[16], pArr[16];
	ToFloatArray(cam.getView(), vArr);
	ToFloatArray(cam.getProj(), pArr);

	ImGuizmo::SetOrthographic(false);

	if (m_viewportDrawList) ImGuizmo::SetDrawlist(m_viewportDrawList);
	else ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	ImGuizmo::SetID(0);
	ImGuizmo::SetGizmoSizeClipSpace(0.15f);
	ImGuizmo::AllowAxisFlip(false);
	ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

	float snapValue = 25.0f;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE)    snapValue = 5.0f;
	if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

	float snap[3] = { snapValue, snapValue, snapValue };
	bool useSnap = ImGui::GetIO().KeyCtrl;
	bool canManipulate = m_viewportHovered || m_viewportActive || m_isUsingGizmo;

	if (canManipulate) {
		ImGuizmo::Manipulate(vArr, pArr, mCurrentGizmoOperation, mCurrentGizmoMode, mArr, nullptr, useSnap ? snap : nullptr);
	}

	m_isUsingGizmo = ImGuizmo::IsUsing();

	if (m_isUsingGizmo) {
		float newPos[3], newRot[3], newSca[3];
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);
		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
		transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));
	}
}

void GUI::drawGizmoToolbar() {
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	if (ImGui::Begin("GizmoToolBar", nullptr, window_flags)) {
		auto buttonMode = [&](const char* label, ImGuizmo::OPERATION op, const char* shortcut) {
			bool isActive = (mCurrentGizmoOperation == op);
			if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.35f, 0.90f, 1.0f)); // Acento Morado
			if (ImGui::Button(label)) mCurrentGizmoOperation = op;
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s (%s)", label, shortcut);
			if (isActive) ImGui::PopStyleColor();
			ImGui::SameLine();
			};

		buttonMode("T", ImGuizmo::TRANSLATE, "W");
		buttonMode("R", ImGuizmo::ROTATE, "E");
		buttonMode("S", ImGuizmo::SCALE, "R");

		static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
		if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local")) {
			mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void GUI::drawStudioTopRibbon() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float menuBarHeight = 24.0f;
	const float ribbonHeight = 72.0f;

	ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, menuBarHeight), ImGuiCond_Always);

	ImGuiWindowFlags menuFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.08f, 0.14f, 1.0f));

	if (ImGui::Begin("##StudioMenuBar", nullptr, menuFlags)) {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::MenuItem("New Scene");
				ImGui::MenuItem("Open Scene...");
				ImGui::MenuItem("Save");
				ImGui::Separator();
				if (ImGui::MenuItem("Exit MinerEngine")) show_exit_popup = true;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				ImGui::MenuItem("Undo");
				ImGui::MenuItem("Redo");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ribbonHeight), ImGuiCond_Always);

	ImGuiWindowFlags ribbonFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.18f, 0.15f, 0.23f, 1.0f));

	if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags)) {
		auto ribbonButton = [&](const char* id, const char* topText, const char* bottomText, ImVec2 size, bool active = false) -> bool {
			if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.35f, 0.90f, 1.0f));
			bool pressed = ImGui::Button(id, size);
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 topSize = ImGui::CalcTextSize(topText);
			ImVec2 bottomSize = ImGui::CalcTextSize(bottomText);
			float centerX = (min.x + max.x) * 0.5f;
			drawList->AddText(ImVec2(centerX - topSize.x * 0.5f, min.y + 10.0f), ImGui::GetColorU32(ImGuiCol_Text), topText);
			drawList->AddText(ImVec2(centerX - bottomSize.x * 0.5f, min.y + 34.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), bottomText);
			if (active) ImGui::PopStyleColor();
			return pressed;
			};

		auto separatorGroup = [&]() {
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(6.0f, 1.0f));
			ImGui::SameLine();
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* draw = ImGui::GetWindowDrawList();
			draw->AddLine(ImVec2(p.x, p.y), ImVec2(p.x, p.y + 48.0f), IM_COL32(140, 90, 230, 100), 1.0f);
			ImGui::Dummy(ImVec2(8.0f, 48.0f));
			ImGui::SameLine();
			};

		const ImVec2 btnSize(72.0f, 52.0f);

		ribbonButton("##Select", "Select", "Cursor", btnSize, false); ImGui::SameLine();
		if (ribbonButton("##Move", "Move", "W", btnSize, mCurrentGizmoOperation == ImGuizmo::TRANSLATE)) mCurrentGizmoOperation = ImGuizmo::TRANSLATE; ImGui::SameLine();
		if (ribbonButton("##Rotate", "Rotate", "E", btnSize, mCurrentGizmoOperation == ImGuizmo::ROTATE)) mCurrentGizmoOperation = ImGuizmo::ROTATE; ImGui::SameLine();
		if (ribbonButton("##Scale", "Scale", "R", btnSize, mCurrentGizmoOperation == ImGuizmo::SCALE)) mCurrentGizmoOperation = ImGuizmo::SCALE;

		separatorGroup();

		ribbonButton("##Part", "3D Object", "Mesh", btnSize, false); ImGui::SameLine();
		ribbonButton("##Light", "Light", "Point", btnSize, false); ImGui::SameLine();
		ribbonButton("##Material", "Material", "Editor", btnSize, false);

		separatorGroup();
		ribbonButton("##Play", "Play", "Game", btnSize, false);
	}
	ImGui::End();

	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar(3);
}

void GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV) {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::Begin("Viewport", nullptr, flags)) {
		m_viewportDrawList = ImGui::GetWindowDrawList();
		ImVec2 panelMin = ImGui::GetCursorScreenPos();
		ImVec2 panelSize = ImGui::GetContentRegionAvail();

		if (panelSize.x < 1.0f) panelSize.x = 1.0f;
		if (panelSize.y < 1.0f) panelSize.y = 1.0f;

		m_viewportPos = panelMin;
		m_viewportSize = panelSize;

		if (viewportSRV) {
			ImGui::Image((ImTextureID)viewportSRV, panelSize);
		}
		else {
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 panelMax(panelMin.x + panelSize.x, panelMin.y + panelSize.y);
			drawList->AddRectFilled(panelMin, panelMax, IM_COL32(30, 26, 40, 255));
			drawList->AddText(ImVec2(panelMin.x + 12.0f, panelMin.y + 12.0f), IM_COL32(220, 220, 240, 255), "Viewport no renderizado");
		}

		m_viewportHovered = ImGui::IsItemHovered();
		m_viewportActive = ImGui::IsItemActive();
		m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void GUI::drawEditorDockspace() {
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	const float topOffset = 96.0f;
	ImVec2 dockPos = ImVec2(mainViewport->Pos.x, mainViewport->Pos.y + topOffset);
	ImVec2 dockSize = ImVec2(mainViewport->Size.x, mainViewport->Size.y - topOffset);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(mainViewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("##MainEditorDockspace", nullptr, window_flags);
	ImGuiID dockspace_id = ImGui::GetID("##EditorDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	ImGui::PopStyleVar(3);


}


