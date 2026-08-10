#include "ECS/ParticleEmitterComponent.h"
#include "GUI/GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS/Actor.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "ECS/MeshRendererComponent.h"
#include "ECS/LightComponent.h"
#include "ECS/RigidbodyComponent.h"
#include "ECS/BoxColliderComponent.h"
#include "ECS/RotateBehaviorComponent.h"
#include "ECS/AudioSourceComponent.h"
#include "Rendering/DeferredRenderer.h"

// Déjalo después de los headers principales del motor.

#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <commdlg.h>
#include <fstream>

#pragma comment(lib, "Comdlg32.lib")


//============================================================
// LISTAR ARCHIVOS DE AUDIO
//============================================================

static std::vector<std::string>
listAudioFiles(const std::string& directory)
{
	std::vector<std::string> files;

	const std::string pattern =
		directory + "\\*";

	WIN32_FIND_DATAA findData{};

	HANDLE findHandle =
		FindFirstFileA(
			pattern.c_str(),
			&findData
		);

	if (findHandle == INVALID_HANDLE_VALUE)
	{
		return files;
	}

	do
	{
		if (findData.dwFileAttributes &
			FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}

		std::string fileName =
			findData.cFileName;

		std::string lowerName =
			fileName;

		std::transform(
			lowerName.begin(),
			lowerName.end(),
			lowerName.begin(),
			[](unsigned char character)
			{
				return static_cast<char>(
					std::tolower(character)
					);
			}
		);

		const bool isWav =
			lowerName.size() >= 4 &&
			lowerName.substr(
				lowerName.size() - 4
			) == ".wav";

		const bool isMp3 =
			lowerName.size() >= 4 &&
			lowerName.substr(
				lowerName.size() - 4
			) == ".mp3";

		if (isWav || isMp3)
		{
			files.push_back(
				fileName
			);
		}
	} while (FindNextFileA(
		findHandle,
		&findData
	));

	FindClose(findHandle);

	std::sort(
		files.begin(),
		files.end()
	);

	return files;
}


//============================================================
// VALIDAR ARCHIVO WAV
//============================================================

static bool
isValidWaveFile(const std::string& filePath)
{
	std::ifstream file(
		filePath,
		std::ios::binary
	);

	if (!file.is_open())
	{
		ERROR(
			"GUI",
			"isValidWaveFile",
			"No se pudo abrir el archivo seleccionado"
		);

		return false;
	}

	char header[12] = {};

	file.read(
		header,
		sizeof(header)
	);

	if (file.gcount() != sizeof(header))
	{
		ERROR(
			"GUI",
			"isValidWaveFile",
			"El archivo es demasiado pequeno o esta danado"
		);

		return false;
	}

	const bool hasRiffHeader =
		header[0] == 'R' &&
		header[1] == 'I' &&
		header[2] == 'F' &&
		header[3] == 'F';

	const bool hasWaveHeader =
		header[8] == 'W' &&
		header[9] == 'A' &&
		header[10] == 'V' &&
		header[11] == 'E';

	if (!hasRiffHeader || !hasWaveHeader)
	{
		ERROR(
			"GUI",
			"isValidWaveFile",
			"El archivo seleccionado no es un WAV valido"
		);

		return false;
	}

	return true;
}

//============================================================
// IMPORTAR ARCHIVO DE AUDIO
//============================================================

static bool
importAudioFile(std::string& outRelativePath)
{
	char selectedFile[MAX_PATH] = {};

	OPENFILENAMEA dialog{};

	dialog.lStructSize =
		sizeof(OPENFILENAMEA);

	dialog.hwndOwner =
		nullptr;

	dialog.lpstrFile =
		selectedFile;

	dialog.nMaxFile =
		MAX_PATH;

	dialog.lpstrFilter =
		"Audio Files (*.wav;*.mp3)\0*.wav;*.mp3\0"
		"Wave Audio (*.wav)\0*.wav\0"
		"MP3 Audio (*.mp3)\0*.mp3\0";

	dialog.nFilterIndex = 1;

	dialog.Flags =
		OFN_FILEMUSTEXIST |
		OFN_PATHMUSTEXIST |
		OFN_NOCHANGEDIR;

	dialog.lpstrTitle =
		"Import Audio File";

	if (!GetOpenFileNameA(&dialog))
	{
		// El usuario canceló la selección.
		return false;
	}

	const std::string sourcePath =
		selectedFile;

	// Confirmar que el contenido sea realmente WAV.
	std::string lowerSourcePath =
		sourcePath;

	std::transform(
		lowerSourcePath.begin(),
		lowerSourcePath.end(),
		lowerSourcePath.begin(),
		[](unsigned char character)
		{
			return static_cast<char>(
				std::tolower(character)
				);
		}
	);

	const bool isWav =
		lowerSourcePath.size() >= 4 &&
		lowerSourcePath.substr(
			lowerSourcePath.size() - 4
		) == ".wav";

	const bool isMp3 =
		lowerSourcePath.size() >= 4 &&
		lowerSourcePath.substr(
			lowerSourcePath.size() - 4
		) == ".mp3";

	if (!isWav && !isMp3)
	{
		ERROR(
			"GUI",
			"importAudioFile",
			"Solo se permiten archivos WAV o MP3"
		);

		return false;
	}

	// La validación RIFF solo corresponde a archivos WAV.
	if (isWav &&
		!isValidWaveFile(sourcePath))
	{
		ERROR(
			"GUI",
			"importAudioFile",
			"Importacion cancelada: WAV invalido"
		);

		return false;
	}

	const std::size_t separatorPosition =
		sourcePath.find_last_of("\\/");

	const std::string fileName =
		separatorPosition == std::string::npos
		? sourcePath
		: sourcePath.substr(
			separatorPosition + 1
		);

	if (fileName.empty())
	{
		ERROR(
			"GUI",
			"importAudioFile",
			"El archivo seleccionado no tiene nombre"
		);

		return false;
	}

	const std::string audioDirectory =
		"Assets\\Audio";

	// Crear Assets/Audio si todavía no existe.
	if (!CreateDirectoryA(
		audioDirectory.c_str(),
		nullptr))
	{
		const DWORD directoryError =
			GetLastError();

		if (directoryError !=
			ERROR_ALREADY_EXISTS)
		{
			ERROR(
				"GUI",
				"importAudioFile",
				"No se pudo crear Assets/Audio"
			);

			return false;
		}
	}

	const std::string destinationPath =
		audioDirectory +
		"\\" +
		fileName;

	// FALSE permite reemplazar un archivo
	// que tenga el mismo nombre.
	if (!CopyFileA(
		sourcePath.c_str(),
		destinationPath.c_str(),
		FALSE))
	{
		ERROR(
			"GUI",
			"importAudioFile",
			"No se pudo copiar el archivo"
		);

		return false;
	}

	outRelativePath =
		"Assets/Audio/" +
		fileName;

	MESSAGE(
		"GUI",
		"importAudioFile",
		"Audio importado correctamente"
	);

	return true;
}

//============================================================
// LISTAR TEXTURAS DE PARTÍCULAS
//============================================================

static std::vector<std::string>
listParticleTextureFiles(
	const std::string& directory)
{
	std::vector<std::string> files;

	const std::string pattern =
		directory + "\\*";

	WIN32_FIND_DATAA findData{};

	HANDLE findHandle =
		FindFirstFileA(
			pattern.c_str(),
			&findData
		);

	if (findHandle ==
		INVALID_HANDLE_VALUE)
	{
		return files;
	}

	do
	{
		if (findData.dwFileAttributes &
			FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}

		std::string fileName =
			findData.cFileName;

		std::string lowerName =
			fileName;

		std::transform(
			lowerName.begin(),
			lowerName.end(),
			lowerName.begin(),
			[](unsigned char character)
			{
				return static_cast<char>(
					std::tolower(character)
					);
			}
		);

		const bool isPng =
			lowerName.size() >= 4 &&
			lowerName.substr(
				lowerName.size() - 4
			) == ".png";

		if (isPng)
		{
			files.push_back(
				fileName
			);
		}
	} while (FindNextFileA(
		findHandle,
		&findData
	));

	FindClose(
		findHandle
	);

	std::sort(
		files.begin(),
		files.end()
	);

	return files;
}

//============================================================
// IMPORTAR TEXTURA DE PARTÍCULAS
//============================================================

static bool
importParticleTexture(
	std::string& outRelativePath)
{
	char selectedFile[MAX_PATH] = {};

	OPENFILENAMEA dialog{};

	dialog.lStructSize =
		sizeof(OPENFILENAMEA);

	dialog.hwndOwner =
		nullptr;

	dialog.lpstrFile =
		selectedFile;

	dialog.nMaxFile =
		MAX_PATH;

	dialog.lpstrFilter =
		"PNG Images (*.png)\0*.png\0"
		"All Files (*.*)\0*.*\0";

	dialog.nFilterIndex = 1;

	dialog.Flags =
		OFN_PATHMUSTEXIST |
		OFN_FILEMUSTEXIST |
		OFN_NOCHANGEDIR;

	dialog.lpstrTitle =
		"Import Particle Texture";

	if (!GetOpenFileNameA(
		&dialog
	))
	{
		// El usuario canceló el explorador.
		return false;
	}

	const std::string sourcePath =
		selectedFile;

	std::string lowerSourcePath =
		sourcePath;

	std::transform(
		lowerSourcePath.begin(),
		lowerSourcePath.end(),
		lowerSourcePath.begin(),
		[](unsigned char character)
		{
			return static_cast<char>(
				std::tolower(character)
				);
		}
	);

	const bool isPng =
		lowerSourcePath.size() >= 4 &&
		lowerSourcePath.substr(
			lowerSourcePath.size() - 4
		) == ".png";

	if (!isPng)
	{
		ERROR(
			"GUI",
			"importParticleTexture",
			"Solo se permiten archivos PNG"
		);

		return false;
	}

	const std::size_t separatorPosition =
		sourcePath.find_last_of(
			"\\/"
		);

	const std::string fileName =
		separatorPosition ==
		std::string::npos
		? sourcePath
		: sourcePath.substr(
			separatorPosition + 1
		);

	if (fileName.empty())
	{
		ERROR(
			"GUI",
			"importParticleTexture",
			"El archivo seleccionado no tiene nombre"
		);

		return false;
	}

	const std::string particleDirectory =
		"Assets\\Textures\\Particles";

	// Crear la carpeta si todavía no existe.
	if (!CreateDirectoryA(
		particleDirectory.c_str(),
		nullptr
	))
	{
		const DWORD directoryError =
			GetLastError();

		if (directoryError !=
			ERROR_ALREADY_EXISTS)
		{
			ERROR(
				"GUI",
				"importParticleTexture",
				"No se pudo crear la carpeta de particulas"
			);

			return false;
		}
	}

	const std::string destinationPath =
		particleDirectory +
		"\\" +
		fileName;

	// FALSE permite reemplazar un PNG con el mismo nombre.
	if (!CopyFileA(
		sourcePath.c_str(),
		destinationPath.c_str(),
		FALSE
	))
	{
		ERROR(
			"GUI",
			"importParticleTexture",
			"No se pudo copiar la textura PNG"
		);

		return false;
	}

	outRelativePath =
		"Assets/Textures/Particles/" +
		fileName;

	MESSAGE(
		"GUI",
		"importParticleTexture",
		"Textura de particulas importada correctamente"
	);

	return true;
}

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

static const ImVec4 kAccent = ImVec4(0.55f, 0.35f, 0.90f, 1.0f);
static const ImVec4 kAccentHi = ImVec4(0.70f, 0.50f, 1.00f, 1.0f);

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

	appleLiquidStyle(1.0f, kAccent);

	ImGui_ImplWin32_Init(window.m_hWnd);
	ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

	toolTipData();
	selectedActorIndex = 0;
}

void GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	style.WindowRounding = 8.0f;  style.ChildRounding = 6.0f;  style.FrameRounding = 5.0f;
	style.PopupRounding = 6.0f;   style.TabRounding = 6.0f;    style.GrabRounding = 5.0f;
	style.ScrollbarRounding = 12.0f; style.WindowBorderSize = 1.0f; style.FrameBorderSize = 0.0f;
	style.WindowPadding = ImVec2(12.0f, 12.0f); style.FramePadding = ImVec2(10.0f, 6.0f);
	style.ItemSpacing = ImVec2(10.0f, 8.0f);    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
	style.IndentSpacing = 18.0f; style.ScrollbarSize = 13.0f; style.GrabMinSize = 10.0f;
	style.WindowTitleAlign = ImVec2(0.02f, 0.5f); style.WindowMenuButtonPosition = ImGuiDir_None;

	const ImVec4 bg0 = ImVec4(0.090f, 0.075f, 0.130f, opacity);
	const ImVec4 bg1 = ImVec4(0.140f, 0.120f, 0.195f, opacity);
	const ImVec4 bg2 = ImVec4(0.200f, 0.165f, 0.290f, opacity);
	const ImVec4 bg3 = ImVec4(0.270f, 0.220f, 0.380f, opacity);
	const ImVec4 txt = ImVec4(0.92f, 0.90f, 0.97f, 1.0f);
	const ImVec4 txtD = ImVec4(0.56f, 0.52f, 0.64f, 1.0f);

	colors[ImGuiCol_Text] = txt;                 colors[ImGuiCol_TextDisabled] = txtD;
	colors[ImGuiCol_WindowBg] = bg0;             colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0.12f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.09f, 0.16f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.34f, 0.27f, 0.48f, 0.50f);
	colors[ImGuiCol_FrameBg] = bg1;              colors[ImGuiCol_FrameBgHovered] = bg2;  colors[ImGuiCol_FrameBgActive] = bg3;
	colors[ImGuiCol_TitleBg] = ImVec4(0.075f, 0.062f, 0.110f, 1.0f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.11f, 0.21f, 1.0f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.085f, 0.070f, 0.120f, 1.0f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0.22f);
	colors[ImGuiCol_ScrollbarGrab] = bg2; colors[ImGuiCol_ScrollbarGrabHovered] = bg3; colors[ImGuiCol_ScrollbarGrabActive] = accent;
	colors[ImGuiCol_CheckMark] = kAccentHi;
	colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.34f, 0.82f, 1.0f); colors[ImGuiCol_SliderGrabActive] = kAccentHi;
	colors[ImGuiCol_Button] = bg1; colors[ImGuiCol_ButtonHovered] = bg2; colors[ImGuiCol_ButtonActive] = accent;
	colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.28f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.48f);
	colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.68f);
	colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.24f, 0.42f, 0.55f);
	colors[ImGuiCol_SeparatorHovered] = accent; colors[ImGuiCol_SeparatorActive] = kAccentHi;
	colors[ImGuiCol_ResizeGrip] = ImVec4(accent.x, accent.y, accent.z, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent.x, accent.y, accent.z, 0.55f);
	colors[ImGuiCol_ResizeGripActive] = kAccentHi;
	colors[ImGuiCol_Tab] = bg1; colors[ImGuiCol_TabHovered] = ImVec4(accent.x, accent.y, accent.z, 0.65f);
	colors[ImGuiCol_TabActive] = ImVec4(0.32f, 0.24f, 0.46f, 1.0f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.085f, 0.15f, 1.0f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.13f, 0.23f, 1.0f);
	colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.55f);
	colors[ImGuiCol_DockingEmptyBg] = bg0;
	colors[ImGuiCol_PlotLines] = kAccentHi; colors[ImGuiCol_PlotHistogram] = accent;
	colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	colors[ImGuiCol_NavHighlight] = kAccentHi;

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		colors[ImGuiCol_WindowBg].w = 1.0f;
	}
}

void GUI::update(Viewport& viewport, Window& window) {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
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

void GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 2.0f, 0.0f });
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.70f, 0.22f, 0.24f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.82f, 0.30f, 0.32f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.60f, 0.16f, 0.18f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
	ImGui::PopFont(); ImGui::PopStyleColor(3); ImGui::SameLine();
	ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth(); ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.28f, 0.58f, 0.30f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.36f, 0.68f, 0.38f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.20f, 0.50f, 0.22f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
	ImGui::PopFont(); ImGui::PopStyleColor(3); ImGui::SameLine();
	ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth(); ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.20f, 0.42f, 0.80f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.28f, 0.52f, 0.90f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.14f, 0.34f, 0.70f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
	ImGui::PopFont(); ImGui::PopStyleColor(3); ImGui::SameLine();
	ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);
	ImGui::PopID();
}

void GUI::toolTipData() {}
void GUI::ToolBar() {}

void GUI::closeApp() {
	if (show_exit_popup) { ImGui::OpenPopup("Exit?"); show_exit_popup = false; }
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Estas a punto de salir de MinerEngine.");
		ImGui::Text("Estas seguro?");
		ImGui::Spacing(); ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120, 0))) { exit(0); ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus(); ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor)
{
	ImGui::Begin("Inspector");

	if (!actor)
	{
		ImGui::TextDisabled("No actor selected");
		ImGui::End();
		return;
	}

	//============================================================
	// HEADER
	//============================================================

	ImGui::PushStyleVar(
		ImGuiStyleVar_FramePadding,
		ImVec2(8.0f, 6.0f)
	);

	ImGui::PushStyleVar(
		ImGuiStyleVar_ItemSpacing,
		ImVec2(8.0f, 8.0f)
	);

	ImGui::Text("Actor");
	ImGui::Separator();

	char objectName[128] = {};

	strcpy_s(
		objectName,
		sizeof(objectName),
		actor->getName().c_str()
	);

	if (ImGui::InputText(
		"Name",
		objectName,
		IM_ARRAYSIZE(objectName)))
	{
		actor->setName(objectName);
	}

	ImGui::Spacing();

	//============================================================
	// TRANSFORM
	//============================================================

	if (ImGui::CollapsingHeader(
		"Transform",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		inspectorContainer(actor);
	}

	//============================================================
	// MESH RENDERER
	//============================================================

	if (ImGui::CollapsingHeader(
		"Mesh Renderer",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		auto meshRenderer =
			actor->getComponent<MeshRendererComponent>();

		if (meshRenderer)
		{
			bool visible =
				meshRenderer->isVisible();

			if (ImGui::Checkbox(
				"Visible",
				&visible))
			{
				meshRenderer->setVisible(visible);
			}
		}
		else
		{
			ImGui::TextDisabled(
				"No MeshRendererComponent found."
			);
		}

		bool castShadow =
			actor->canCastShadow();

		if (ImGui::Checkbox(
			"Cast Shadows",
			&castShadow))
		{
			actor->setCastShadow(castShadow);
		}
	}

	//============================================================
	// MATERIAL
	//============================================================

	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::TextDisabled("Material Editor");
		ImGui::Separator();

		ImGui::Text("Albedo");
		ImGui::Text("Normal");
		ImGui::Text("Metallic");
		ImGui::Text("Roughness");
	}

	//============================================================
	// LIGHT
	//============================================================

	auto lightComponent =
		actor->getComponent<LightComponent>();

	if (lightComponent)
	{
		if (ImGui::CollapsingHeader(
			"Light Settings",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& lightData =
				lightComponent->getLightData();

			ImGui::TextDisabled("Light Properties");

			ImGui::ColorEdit3(
				"Color",
				&lightData.color.x
			);

			ImGui::DragFloat(
				"Intensity",
				&lightData.intensity,
				0.1f,
				0.0f,
				100.0f,
				"%.2f"
			);

			if (lightData.type == LightType::Point ||
				lightData.type == LightType::Spot)
			{
				ImGui::DragFloat(
					"Range",
					&lightData.range,
					0.5f,
					0.1f,
					500.0f,
					"%.1f"
				);
			}

			if (lightData.type == LightType::Spot)
			{
				ImGui::DragFloat(
					"Spot Angle",
					&lightData.spotAngle,
					1.0f,
					1.0f,
					90.0f,
					"%.1f"
				);
			}

			ImGui::Spacing();

			bool castShadows =
				actor->canCastShadow();

			if (ImGui::Checkbox(
				"Light Cast Shadows",
				&castShadows))
			{
				actor->setCastShadow(castShadows);
			}
		}
	}

	//============================================================
	// OBTENER COMPONENTES
	//============================================================

	auto rigidbody =actor->getComponent<RigidbodyComponent>();

	auto boxCollider =actor->getComponent<BoxColliderComponent>();

	auto rotateBehavior =actor->getComponent<RotateBehaviorComponent>();

	auto audioSource =actor->getComponent<AudioSourceComponent>();

	auto particleEmitter=actor->getComponent<ParticleEmitterComponent>();

	//============================================================
	// PHYSICS
	//============================================================

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Physics");

	//============================================================
	// RIGIDBODY COMPONENT
	//============================================================

	if (rigidbody)
	{
		bool rigidbodyEnabled =
			rigidbody->isEnabled();

		if (ImGui::Checkbox(
			"Rigidbody",
			&rigidbodyEnabled))
		{
			rigidbody->setEnabled(
				rigidbodyEnabled
			);

			if (rigidbodyEnabled)
			{
				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Rigidbody activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Rigidbody desactivado"
				);
			}
		}

		if (rigidbody->isEnabled())
		{
			ImGui::Indent();

			ImGui::DragFloat(
				"Mass",
				&rigidbody->mass,
				0.1f,
				0.01f,
				1000.0f,
				"%.2f"
			);

			if (ImGui::Checkbox(
				"Use Gravity",
				&rigidbody->useGravity))
			{
				if (rigidbody->useGravity)
				{
					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Gravedad activada"
					);
				}
				else
				{
					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Gravedad desactivada"
					);
				}
			}

			if (ImGui::Checkbox(
				"Is Kinematic",
				&rigidbody->isKinematic))
			{
				if (rigidbody->isKinematic)
				{
					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Rigidbody configurado como Kinematic"
					);
				}
				else
				{
					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Rigidbody configurado como Dynamic"
					);
				}
			}

			ImGui::DragFloat3(
				"Velocity",
				&rigidbody->velocity.x,
				0.1f
			);

			ImGui::Text(
				"Grounded: %s",
				rigidbody->isGrounded
				? "Yes"
				: "No"
			);

			ImGui::Unindent();
		}
	}
	else
	{
		if (ImGui::Button(
			"Add Rigidbody",
			ImVec2(-1.0f, 0.0f)))
		{
			auto newRigidbody =
				EU::MakeShared<RigidbodyComponent>();

			newRigidbody->mass = 1.0f;
			newRigidbody->useGravity = true;
			newRigidbody->isKinematic = false;
			newRigidbody->isGrounded = false;

			newRigidbody->velocity =
				EU::Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			actor->addComponent(newRigidbody);

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Rigidbody agregado al actor"
			);
		}
	}

	//============================================================
	// BOX COLLIDER COMPONENT
	//============================================================

	ImGui::Spacing();

	if (boxCollider)
	{
		bool colliderEnabled =
			boxCollider->isEnabled();

		if (ImGui::Checkbox(
			"Box Collider",
			&colliderEnabled))
		{
			boxCollider->setEnabled(
				colliderEnabled
			);

			if (colliderEnabled)
			{
				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Box Collider activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Box Collider desactivado"
				);
			}
		}

		if (boxCollider->isEnabled())
		{
			ImGui::Indent();

			ImGui::DragFloat3(
				"Center",
				&boxCollider->center.x,
				0.1f
			);

			ImGui::DragFloat3(
				"Size",
				&boxCollider->size.x,
				0.1f,
				0.01f,
				1000.0f
			);

			if (ImGui::Checkbox(
				"Is Trigger",
				&boxCollider->isTrigger))
			{
				if (boxCollider->isTrigger)
				{
					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Box Collider configurado como Trigger"
					);
				}
				else
				{
					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Box Collider configurado como Solido"
					);
				}
			}

			ImGui::Unindent();
		}
	}
	else
	{
		if (ImGui::Button(
			"Add Box Collider",
			ImVec2(-1.0f, 0.0f)))
		{
			auto newCollider =
				EU::MakeShared<BoxColliderComponent>();

			newCollider->center =
				EU::Vector3(
					0.0f,
					0.0f,
					0.0f
				);

			newCollider->size =
				EU::Vector3(
					1.0f,
					1.0f,
					1.0f
				);

			newCollider->isTrigger = false;

			actor->addComponent(newCollider);

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Box Collider agregado al actor"
			);
		}
	}

	//============================================================
	// ROTATE BEHAVIOR COMPONENT
	//============================================================

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (rotateBehavior)
	{
		bool behaviorEnabled =
			rotateBehavior->isEnabled();

		if (ImGui::Checkbox(
			"Rotate Behavior",
			&behaviorEnabled))
		{
			rotateBehavior->setEnabled(
				behaviorEnabled
			);

			if (behaviorEnabled)
			{
				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Rotate Behavior activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Rotate Behavior desactivado"
				);
			}
		}

		if (rotateBehavior->isEnabled())
		{
			ImGui::Indent();

			ImGui::DragFloat3(
				"Rotation Axis",
				&rotateBehavior->axis.x,
				0.05f,
				-1.0f,
				1.0f
			);

			ImGui::DragFloat(
				"Rotation Speed",
				&rotateBehavior->speed,
				1.0f,
				0.0f,
				1000.0f,
				"%.2f"
			);

			bool reverseDirection =
				rotateBehavior->direction < 0.0f;

			if (ImGui::Checkbox(
				"Reverse Direction",
				&reverseDirection))
			{
				if (reverseDirection)
				{
					rotateBehavior->direction =
						-1.0f;

					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Direccion invertida"
					);
				}
				else
				{
					rotateBehavior->direction =
						1.0f;

					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Direccion normal"
					);
				}
			}

			ImGui::Unindent();
		}
	}
	else
	{
		if (ImGui::Button(
			"Add Rotate Behavior",
			ImVec2(-1.0f, 0.0f)))
		{
			auto newBehavior =
				EU::MakeShared<RotateBehaviorComponent>();

			newBehavior->axis =
				EU::Vector3(
					0.0f,
					1.0f,
					0.0f
				);

			newBehavior->speed = 45.0f;
			newBehavior->direction = 1.0f;

			actor->addComponent(newBehavior);

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Rotate Behavior agregado al actor"
			);
		}
	}

	//============================================================
    // AUDIO SOURCE COMPONENT
    //============================================================

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      
      ImGui::Text("Audio");
      
      if (audioSource)
      {
      	  bool audioEnabled =
		  audioSource->isEnabled();

	  if (ImGui::Checkbox(
		  "Audio Source",
		  &audioEnabled))
	     {
		  audioSource->setEnabled(
			audioEnabled
		);

		if (audioEnabled)
		{
			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Audio Source activado"
			);
		}
		else
		{
			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Audio Source desactivado"
			);
		}
	}

	if (audioEnabled)
	{
		ImGui::Indent();
		static std::vector<std::string> audioFiles =
			listAudioFiles("Assets/Audio");

		//====================================================
		// IMPORTAR AUDIO DESDE LA COMPUTADORA
		//====================================================

		if (ImGui::Button(
			"Import Audio",
			ImVec2(-1.0f, 0.0f)))
		{
			std::string importedAudioPath;

			if (importAudioFile(
				importedAudioPath))
			{
				// Detener el Preview anterior.
				audioSource->previewRequested = false;
				audioSource->stopPreviewRequested = true;

				// Seleccionar automáticamente el WAV importado.
				strcpy_s(
					audioSource->filePath,
					sizeof(audioSource->filePath),
					importedAudioPath.c_str()
				);

				// Actualizar la lista de archivos.
				audioFiles =
					listAudioFiles(
						"Assets/Audio"
					);

				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Archivo importado y seleccionado"
				);
			}
		}

		//====================================================
		// ACTUALIZAR ARCHIVOS DE AUDIO
		//====================================================

		if (ImGui::Button(
			"Refresh Audio Files",
			ImVec2(-1.0f, 0.0f)))
		{
			audioFiles =
				listAudioFiles(
					"Assets/Audio"
				);

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Lista de audio actualizada"
			);
		}

		//====================================================
		// SELECTOR DE ARCHIVO
		//====================================================

		const char* currentAudio =
			audioSource->filePath[0] != '\0'
			? audioSource->filePath
			: "Select WAV";

		if (ImGui::BeginCombo(
			"Audio File",
			currentAudio))
		{
			for (const std::string& fileName :
				audioFiles)
			{
				const std::string fullPath =
					"Assets/Audio/" +
					fileName;

				const bool selected =
					fullPath ==
					audioSource->filePath;

				if (ImGui::Selectable(
					fileName.c_str(),
					selected))
				{
					// Detener el Preview anterior
					// al cambiar de archivo.
					audioSource->previewRequested = false;
					audioSource->stopPreviewRequested = true;

					strcpy_s(
						audioSource->filePath,
						sizeof(audioSource->filePath),
						fullPath.c_str()
					);

					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Archivo de audio seleccionado"
					);
				}

				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		if (audioFiles.empty())
		{
			ImGui::TextDisabled(
				"No WAV files found in Assets/Audio"
			);
		}

		//====================================================
		// PREVIEW DEL SONIDO PRINCIPAL
		//====================================================

		if (ImGui::Button(
			"Preview",
			ImVec2(120.0f, 0.0f)))
		{
			if (audioSource->filePath[0] != '\0')
			{
				audioSource->previewRequested = true;
				audioSource->stopPreviewRequested = false;

				MESSAGE(
					"GUI",
					"inspectorGeneral",
					"Preview solicitado"
				);
			}
			else
			{
				ERROR(
					"GUI",
					"inspectorGeneral",
					"Selecciona un archivo de audio"
				);
			}
		}

		ImGui::SameLine();

		if (ImGui::Button(
			"Stop Preview",
			ImVec2(120.0f, 0.0f)))
		{
			audioSource->previewRequested = false;
			audioSource->stopPreviewRequested = true;

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Detener Preview solicitado"
			);
		}

		ImGui::Spacing();

		//====================================================
		// PROPIEDADES DEL SONIDO PRINCIPAL
		//====================================================

		ImGui::SliderFloat(
			"Volume",
			&audioSource->volume,
			0.0f,
			1.0f,
			"%.2f"
		);

		ImGui::Checkbox(
			"Loop",
			&audioSource->loop
		);

		ImGui::Checkbox(
			"Play On Start",
			&audioSource->playOnStart
		);

		ImGui::Checkbox(
			"Spatial 3D",
			&audioSource->spatial3D
		);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//====================================================
		// SONIDOS ADICIONALES
		//====================================================

		ImGui::Text("Additional Sounds");

		if (ImGui::Button(
			"Add Sound",
			ImVec2(-1.0f, 0.0f)))
		{
			audioSource->sounds.emplace_back();

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Nuevo sonido agregado"
			);
		}

		int soundToRemove = -1;

		for (int soundIndex = 0;
			soundIndex <
			static_cast<int>(audioSource->sounds.size());
			++soundIndex)
		{
			AudioClipData& sound =
				audioSource->sounds[soundIndex];

			ImGui::PushID(soundIndex);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			const std::string soundTitle =
				"Sound " +
				std::to_string(soundIndex + 2);

			ImGui::Text(
				"%s",
				soundTitle.c_str()
			);

			const char* currentAdditionalAudio =
				sound.filePath[0] != '\0'
				? sound.filePath
				: "Select WAV";

			if (ImGui::BeginCombo(
				"Audio File",
				currentAdditionalAudio))
			{
				for (const std::string& fileName : audioFiles)
				{
					const std::string fullPath =
						"Assets/Audio/" + fileName;

					const bool selected =
						fullPath == sound.filePath;

					if (ImGui::Selectable(
						fileName.c_str(),
						selected))
					{
						sound.previewRequested = false;
						sound.stopPreviewRequested = true;

						strcpy_s(
							sound.filePath,
							sizeof(sound.filePath),
							fullPath.c_str()
						);

						MESSAGE(
							"GUI",
							"inspectorGeneral",
							"Archivo adicional seleccionado"
						);
					}

					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			//================================================
			// PREVIEW DEL SONIDO ADICIONAL
			//================================================

			if (ImGui::Button(
				"Preview",
				ImVec2(120.0f, 0.0f)))
			{
				if (sound.filePath[0] != '\0')
				{
					sound.previewRequested = true;
					sound.stopPreviewRequested = false;

					MESSAGE(
						"GUI",
						"inspectorGeneral",
						"Preview adicional solicitado"
					);
				}
				else
				{
					ERROR(
						"GUI",
						"inspectorGeneral",
						"Selecciona un archivo de audio"
					);
				}
			}

			ImGui::SameLine();

			if (ImGui::Button(
				"Stop Preview",
				ImVec2(120.0f, 0.0f)))
			{
				sound.previewRequested = false;
				sound.stopPreviewRequested = true;
			}

			ImGui::SliderFloat(
				"Volume",
				&sound.volume,
				0.0f,
				1.0f,
				"%.2f"
			);

			ImGui::Checkbox(
				"Loop",
				&sound.loop
			);

			ImGui::Checkbox(
				"Play On Start",
				&sound.playOnStart
			);

			ImGui::Checkbox(
				"Spatial 3D",
				&sound.spatial3D
			);

			if (ImGui::Button(
				"Remove Sound",
				ImVec2(-1.0f, 0.0f)))
			{
				soundToRemove = soundIndex;
			}

			ImGui::PopID();
		}

		// Eliminar después del for para no invalidar el vector.
		if (soundToRemove >= 0)
		{
			// Stop Preview es global, por eso usamos la solicitud
			// del componente principal antes de borrar el elemento.
			audioSource->previewRequested = false;
			audioSource->stopPreviewRequested = true;

			audioSource->sounds.erase(
				audioSource->sounds.begin() +
				soundToRemove
			);

			MESSAGE(
				"GUI",
				"inspectorGeneral",
				"Sonido eliminado"
			);
		}

		ImGui::Unindent();
	}
	  }
	  else
	  {
		  if (ImGui::Button(
			  "Add Audio Source",
			  ImVec2(-1.0f, 0.0f)))
		  {
			  auto newAudioSource =
				  EU::MakeShared<AudioSourceComponent>();

			  actor->addComponent(
				  newAudioSource
			  );

			  MESSAGE(
				  "GUI",
				  "inspectorGeneral",
				  "Audio Source agregado al actor"
			  );
		  }
	  }


	  //============================================================
	  // PARTICLE EMITTER COMPONENT
	  //============================================================

	  ImGui::Spacing();
	  ImGui::Separator();
	  ImGui::Spacing();

	  ImGui::Text("Particles");

	  if (particleEmitter)
	  {
		  bool particleEnabled =
			  particleEmitter->isEnabled();

		  if (ImGui::Checkbox(
			  "Particle Emitter",
			  &particleEnabled))
		  {
			  particleEmitter->setEnabled(
				  particleEnabled
			  );
		  }

		  if (particleEnabled)
		  {
			  ImGui::Indent();

			  //====================================================
			  // CONFIGURACION GENERAL
			  //====================================================

			  ImGui::Checkbox(
				  "Play On Start##Particles",
				  &particleEmitter->playOnStart
			  );

			  ImGui::Checkbox(
				  "Looping##Particles",
				  &particleEmitter->looping
			  );

			  if (!particleEmitter->looping)
			  {
				  ImGui::SliderFloat(
					  "Duration##Particles",
					  &particleEmitter->duration,
					  0.10f,
					  60.0f,
					  "%.2f"
				  );
			  }

			  ImGui::Spacing();
			  ImGui::Separator();
			  ImGui::Spacing();

			  ImGui::Text("Particle Layers");

			  //====================================================
			  // AGREGAR CAPA
			  //====================================================

			  if (ImGui::Button(
				  "+ Add Particle Layer",
				  ImVec2(-1.0f, 0.0f)))
			  {
				  ParticleLayer newLayer;

				  const std::size_t newIndex =
					  particleEmitter->layers.size();

				  sprintf_s(
					  newLayer.name,
					  sizeof(newLayer.name),
					  "Particle Layer %zu",
					  newIndex
				  );

				  particleEmitter->layers.push_back(
					  newLayer
				  );

				  MESSAGE(
					  "GUI",
					  "inspectorGeneral",
					  "Nueva capa de particulas agregada"
				  );
			  }

			  ImGui::Spacing();

			  int layerToRemove = -1;

			  // Lista compartida por todas las capas.
			  static std::vector<std::string>
				  particleTextureFiles =
				  listParticleTextureFiles(
					  "Assets/Textures/Particles"
				  );

			  //====================================================
			  // DIBUJAR CAPAS
			  //====================================================

			  for (std::size_t layerIndex = 0;
				  layerIndex <
				  particleEmitter->layers.size();
				  ++layerIndex)
			  {
				  ParticleLayer& layer =
					  particleEmitter
					  ->layers[layerIndex];

				  ImGui::PushID(
					  static_cast<int>(
						  layerIndex
						  )
				  );

				  const bool layerOpen =
					  ImGui::CollapsingHeader(
						  layer.name,
						  ImGuiTreeNodeFlags_DefaultOpen
					  );

				  if (layerOpen)
				  {
					  ImGui::Indent();

					  ImGui::Checkbox(
						  "Enabled",
						  &layer.enabled
					  );

					  ImGui::InputText(
						  "Layer Name",
						  layer.name,
						  sizeof(layer.name)
					  );

					  //================================================
					  // TEXTURA
					  //================================================

					  ImGui::Spacing();
					  ImGui::TextDisabled("Texture");

					  if (ImGui::Button(
						  "Import PNG",
						  ImVec2(130.0f, 0.0f)))
					  {
						  std::string importedTexturePath;

						  if (importParticleTexture(
							  importedTexturePath
						  ))
						  {
							  strcpy_s(
								  layer.texturePath,
								  sizeof(layer.texturePath),
								  importedTexturePath.c_str()
							  );

							  particleTextureFiles =
								  listParticleTextureFiles(
									  "Assets/Textures/Particles"
								  );

							  MESSAGE(
								  "GUI",
								  "inspectorGeneral",
								  "PNG importado y seleccionado"
							  );
						  }
					  }

					  ImGui::SameLine();

					  if (ImGui::Button(
						  "Refresh PNG",
						  ImVec2(130.0f, 0.0f)))
					  {
						  particleTextureFiles =
							  listParticleTextureFiles(
								  "Assets/Textures/Particles"
							  );

						  MESSAGE(
							  "GUI",
							  "inspectorGeneral",
							  "Lista de PNG actualizada"
						  );
					  }

					  const char* currentParticleTexture =
						  layer.texturePath[0] != '\0'
						  ? layer.texturePath
						  : "Select Particle PNG";

					  if (ImGui::BeginCombo(
						  "Texture",
						  currentParticleTexture))
					  {
						  const bool noTextureSelected =
							  layer.texturePath[0] == '\0';

						  if (ImGui::Selectable(
							  "None - Generated Circle",
							  noTextureSelected))
						  {
							  layer.texturePath[0] =
								  '\0';
						  }

						  if (noTextureSelected)
						  {
							  ImGui::SetItemDefaultFocus();
						  }

						  for (const std::string& fileName :
							  particleTextureFiles)
						  {
							  const std::string fullPath =
								  "Assets/Textures/Particles/" +
								  fileName;

							  const bool selected =
								  fullPath ==
								  layer.texturePath;

							  if (ImGui::Selectable(
								  fileName.c_str(),
								  selected))
							  {
								  strcpy_s(
									  layer.texturePath,
									  sizeof(layer.texturePath),
									  fullPath.c_str()
								  );

								  MESSAGE(
									  "GUI",
									  "inspectorGeneral",
									  "Textura de particulas seleccionada"
								  );
							  }

							  if (selected)
							  {
								  ImGui::SetItemDefaultFocus();
							  }
						  }

						  ImGui::EndCombo();
					  }

					  if (particleTextureFiles.empty())
					  {
						  ImGui::TextDisabled(
							  "No PNG files found"
						  );
					  }
					  else if (layer.texturePath[0] == '\0')
					  {
						  ImGui::TextDisabled(
							  "Using generated circle"
						  );
					  }

					  //================================================
					  // EMISION
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();
					  ImGui::TextDisabled("Emission");

					  ImGui::SliderFloat(
						  "Emission Rate",
						  &layer.emissionRate,
						  0.0f,
						  500.0f,
						  "%.1f"
					  );

					  int maxParticles =
						  static_cast<int>(
							  layer.maxParticles
							  );

					  if (ImGui::SliderInt(
						  "Max Particles",
						  &maxParticles,
						  1,
						  10000))
					  {
						  layer.maxParticles =
							  static_cast<unsigned int>(
								  maxParticles
								  );
					  }

					  ImGui::SliderFloat(
						  "Lifetime",
						  &layer.particleLifetime,
						  0.10f,
						  30.0f,
						  "%.2f"
					  );

					  ImGui::Checkbox(
						  "Randomize Lifetime",
						  &layer.randomizeLifetime
					  );

					  if (layer.randomizeLifetime)
					  {
						  ImGui::SliderFloat(
							  "Lifetime Variation",
							  &layer.lifetimeVariation,
							  0.0f,
							  0.95f,
							  "%.2f"
						  );
					  }

					  //================================================
					  // FORMA DE EMISION
					  //================================================

					  int emissionShape =
						  static_cast<int>(
							  layer.emissionShape
							  );

					  const char* emissionShapes[] =
					  {
						  "Box",
						  "Sphere",
						  "Cone"
					  };

					  if (ImGui::Combo(
						  "Emission Shape",
						  &emissionShape,
						  emissionShapes,
						  3))
					  {
						  layer.emissionShape =
							  static_cast<
							  ParticleEmissionShape
							  >(
								  emissionShape
								  );
					  }

					  if (layer.emissionShape ==
						  ParticleEmissionShape::Box)
					  {
						  ImGui::DragFloat3(
							  "Box Size",
							  &layer.emitterSize.x,
							  0.01f,
							  0.0f,
							  20.0f,
							  "%.2f"
						  );

						  ImGui::TextDisabled(
							  "Particles spawn inside the box"
						  );
					  }
					  else if (layer.emissionShape ==
						  ParticleEmissionShape::Sphere)
					  {
						  ImGui::SliderFloat(
							  "Sphere Radius",
							  &layer.sphereRadius,
							  0.0f,
							  20.0f,
							  "%.2f"
						  );

						  ImGui::TextDisabled(
							  "Particles move out from the sphere"
						  );
					  }
					  else if (layer.emissionShape ==
						  ParticleEmissionShape::Cone)
					  {
						  ImGui::SliderFloat(
							  "Cone Angle",
							  &layer.coneAngleDegrees,
							  0.0f,
							  89.0f,
							  "%.1f degrees"
						  );

						  ImGui::SliderFloat(
							  "Cone Base Radius",
							  &layer.coneBaseRadius,
							  0.0f,
							  20.0f,
							  "%.2f"
						  );

						  ImGui::TextDisabled(
							  "Cone points toward local +Y"
						  );
					  }

					  //================================================
					  // MOVIMIENTO
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();
					  ImGui::TextDisabled("Movement");

					  ImGui::SliderFloat(
						  "Start Speed",
						  &layer.startSpeed,
						  0.0f,
						  30.0f,
						  "%.2f"
					  );

					  ImGui::SliderFloat(
						  "Speed Variation",
						  &layer.speedVariation,
						  0.0f,
						  30.0f,
						  "%.2f"
					  );

					  ImGui::SliderFloat(
						  "Gravity",
						  &layer.gravityMultiplier,
						  -20.0f,
						  20.0f,
						  "%.2f"
					  );

					  //================================================
					  // TAMAÑO
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();
					  ImGui::TextDisabled("Size");

					  ImGui::SliderFloat(
						  "Start Size",
						  &layer.startSize,
						  0.01f,
						  10.0f,
						  "%.2f"
					  );

					  ImGui::SliderFloat(
						  "End Size",
						  &layer.endSize,
						  0.0f,
						  10.0f,
						  "%.2f"
					  );

					  ImGui::SliderFloat(
						  "Size Variation",
						  &layer.sizeVariation,
						  0.0f,
						  10.0f,
						  "%.2f"
					  );

					  //================================================
					  // ROTACION
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();
					  ImGui::TextDisabled("Rotation");

					  ImGui::DragFloatRange2(
						  "Start Rotation",
						  &layer.minimumStartRotation,
						  &layer.maximumStartRotation,
						  1.0f,
						  -360.0f,
						  360.0f,
						  "Min: %.1f",
						  "Max: %.1f"
					  );

					  ImGui::DragFloatRange2(
						  "Angular Velocity",
						  &layer.minimumAngularVelocity,
						  &layer.maximumAngularVelocity,
						  1.0f,
						  -720.0f,
						  720.0f,
						  "Min: %.1f",
						  "Max: %.1f"
					  );

					  ImGui::TextDisabled(
						  "Rotation values use degrees"
					  );

					  //================================================
					  // COLOR
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();
					  ImGui::TextDisabled("Color");

					  ImGui::ColorEdit4(
						  "Start Color",
						  &layer.startColor.x
					  );

					  ImGui::ColorEdit4(
						  "End Color",
						  &layer.endColor.x
					  );

					  //================================================
					  // RENDER
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();
					  ImGui::TextDisabled("Rendering");

					  int blendMode =
						  static_cast<int>(
							  layer.blendMode
							  );

					  const char* blendModes[] =
					  {
						  "Alpha",
						  "Additive"
					  };

					  if (ImGui::Combo(
						  "Blend Mode",
						  &blendMode,
						  blendModes,
						  2))
					  {
						  layer.blendMode =
							  static_cast<
							  ParticleBlendMode
							  >(
								  blendMode
								  );
					  }

					  ImGui::Checkbox(
						  "Depth Sorting",
						  &layer.depthSorting
					  );

					  if (layer.blendMode ==
						  ParticleBlendMode::Alpha)
					  {
						  if (layer.depthSorting)
						  {
							  ImGui::TextDisabled(
								  "Alpha particles sorted back to front"
							  );
						  }
						  else
						  {
							  ImGui::TextDisabled(
								  "Warning: Alpha may render incorrectly"
							  );
						  }
					  }
					  else
					  {
						  ImGui::TextDisabled(
							  "Sorting is normally unnecessary for Additive"
						  );
					  }

					  //================================================
					  // ELIMINAR CAPA
					  //================================================

					  ImGui::Spacing();
					  ImGui::Separator();

					  if (particleEmitter->layers.size() > 1)
					  {
						  if (ImGui::Button(
							  "Remove Layer",
							  ImVec2(-1.0f, 0.0f)))
						  {
							  layerToRemove =
								  static_cast<int>(
									  layerIndex
									  );
						  }
					  }
					  else
					  {
						  ImGui::TextDisabled(
							  "An emitter needs at least one layer"
						  );
					  }

					  ImGui::Unindent();
				  }

				  ImGui::PopID();
				  ImGui::Spacing();
			  }

			  //====================================================
			  // ELIMINAR CAPA SELECCIONADA
			  //====================================================

			  if (layerToRemove >= 0 &&
				  particleEmitter->layers.size() > 1)
			  {
				  particleEmitter->layers.erase(
					  particleEmitter->layers.begin() +
					  layerToRemove
				  );

				  MESSAGE(
					  "GUI",
					  "inspectorGeneral",
					  "Capa de particulas eliminada"
				  );
			  }

			  //====================================================
			  // COMPATIBILIDAD LEGACY
			  //====================================================

			  if (!particleEmitter->layers.empty())
			  {
				  const ParticleLayer& firstLayer =
					  particleEmitter->layers[0];

				  particleEmitter->emissionRate =
					  firstLayer.emissionRate;

				  particleEmitter->maxParticles =
					  firstLayer.maxParticles;

				  particleEmitter->particleLifetime =
					  firstLayer.particleLifetime;

				  particleEmitter->startSpeed =
					  firstLayer.startSpeed;

				  particleEmitter->startSize =
					  firstLayer.startSize;

				  particleEmitter->endSize =
					  firstLayer.endSize;

				  particleEmitter->gravityMultiplier =
					  firstLayer.gravityMultiplier;

				  particleEmitter->startColor =
					  firstLayer.startColor;

				  particleEmitter->endColor =
					  firstLayer.endColor;

				  particleEmitter->emitterSize =
					  firstLayer.emitterSize;
			  }

			  ImGui::Unindent();
		  }
	  }
	  else
	  {
		  if (ImGui::Button(
			  "Add Particle Emitter",
			  ImVec2(-1.0f, 0.0f)))
		  {
			  auto newParticleEmitter =
				  EU::MakeShared<
				  ParticleEmitterComponent
				  >();

			  actor->addComponent(
				  newParticleEmitter
			  );

			  MESSAGE(
				  "GUI",
				  "inspectorGeneral",
				  "Particle Emitter agregado al actor"
			  );
		  }
	  }
//============================================================
// FINALIZAR INSPECTOR
//============================================================

ImGui::PopStyleVar(2);

ImGui::End();
}
void GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	if (!actor) return;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;
	vec3Control("Position", const_cast<float*>(transform->getPosition().data()), 0.0f, 75.0f);
	vec3Control("Rotation", const_cast<float*>(transform->getRotation().data()), 0.0f, 75.0f);
	vec3Control("Scale", const_cast<float*>(transform->getScale().data()), 1.0f, 75.0f);
}

void GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));

	if (ImGui::Button("Show All")) {
		for (const auto& a : actors) {
			if (a.isNull()) continue;
			auto mr = a->getComponent<MeshRendererComponent>();
			if (mr) mr->setVisible(true);
		}
	}
	ImGui::SameLine();
	static ImGuiTextFilter filter;
	filter.Draw("Search", ImGui::GetContentRegionAvail().x - 8.0f);
	ImGui::Separator();

	for (int i = 0; i < (int)actors.size(); ++i) {
		const auto& actor = actors[i];
		if (!actor) continue;
		std::string actorName = actor->getName();
		if (!filter.PassFilter(actorName.c_str())) continue;

		ImGui::PushID(i);

		auto mr = actor->getComponent<MeshRendererComponent>();
		if (mr) {
			bool vis = mr->isVisible();
			if (ImGui::Checkbox("##vis", &vis)) mr->setVisible(vis);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(vis ? "Visible (click para ocultar)" : "Oculto (click para mostrar)");
		}
		else {
			ImGui::Dummy(ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
		}
		ImGui::SameLine();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (selectedActorIndex == i) flags |= ImGuiTreeNodeFlags_Selected;

		bool hidden = (mr && !mr->isVisible());
		if (hidden) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.48f, 0.56f, 1.0f));
		std::string label;

		if (actorName.find("Light") != std::string::npos)
		{
			label = "[L] " + actorName;
		}
		else if (actorName.find("Camera") != std::string::npos)
		{
			label = "[C] " + actorName;
		}
		else
		{
			label = "[M] " + actorName;
		}

		bool nodeOpen = ImGui::TreeNodeEx(
			"##node",
			flags,
			"%s",
			label.c_str());
		if (hidden) ImGui::PopStyleColor();

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) selectedActorIndex = i;

		// ===== PUNTO (b): menu de clic derecho =====
		if (ImGui::BeginPopupContextItem()) {
			selectedActorIndex = i;
			ImGui::TextDisabled("Actor Options");
			ImGui::Separator();

			if (ImGui::MenuItem("Duplicate", "Ctrl+D")) m_duplicateRequested = true;
			if (ImGui::MenuItem("Copy", "Ctrl+C")) m_copyRequested = true;
			if (ImGui::MenuItem("Paste", "Ctrl+V")) m_pasteRequested = true;

			ImGui::Separator();
			if (ImGui::MenuItem("Isolate (solo)")) {
				for (const auto& a2 : actors) {
					if (a2.isNull()) continue;
					auto mr2 = a2->getComponent<MeshRendererComponent>();
					if (mr2) mr2->setVisible(a2.get() == actor.get());
				}
			}
			if (ImGui::MenuItem("Show All")) {
				for (const auto& a2 : actors) {
					if (a2.isNull()) continue;
					auto mr2 = a2->getComponent<MeshRendererComponent>();
					if (mr2) mr2->setVisible(true);
				}
			}
			if (mr) {
				bool vis = mr->isVisible();
				if (ImGui::MenuItem(vis ? "Hide" : "Show")) mr->setVisible(!vis);
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Directional Light"))
					m_createDirectionalLightRequested = true;

				if (ImGui::Button("+ Add Point Light"))
				{
					// Usamos la misma bandera que usa el MenuItem
					m_createPointLightRequested = true;
				}

				if (ImGui::Button("+ Add Spot Light"))
				{
					// Usamos la misma bandera que usa el MenuItem
					m_createSpotLightRequested = true;
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			if (ImGui::MenuItem("Delete", "Del")) m_deleteRequested = true;
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
		// ===========================================

		if (nodeOpen) {
			auto transform = actor->getComponent<Transform>();
			if (transform)
				ImGui::TextDisabled("   Pos: %.1f, %.1f, %.1f",
					transform->getPosition().x, transform->getPosition().y, transform->getPosition().z);
			ImGui::TreePop();
		}



		ImGui::PopID();



	}
	ImGui::PopStyleVar(2);
	ImGui::End();
}

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor) {
	// =========================================================
	// NUEVO: Atajos de teclado para cambiar herramienta (1, 2, 3)
	// =========================================================
	ImGuiIO& io = ImGui::GetIO();

	// Solo cambiamos de herramienta si el usuario NO está escribiendo 
	// en una caja de texto (como al cambiarle el nombre al Actor)
	if (!io.WantTextInput) {
		if (GetAsyncKeyState('1') & 0x8000) mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (GetAsyncKeyState('2') & 0x8000) mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (GetAsyncKeyState('3') & 0x8000) mCurrentGizmoOperation = ImGuizmo::SCALE;
	}
	// =========================================================

	if (!actor) return;
	static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

	float rectX = m_viewportPos.x, rectY = m_viewportPos.y;
	float rectW = m_viewportSize.x, rectH = m_viewportSize.y;
	if (rectW < 64.0f || rectH < 64.0f) { m_isUsingGizmo = false; return; }

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

	float snapValue = m_snapScale;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE)         snapValue = m_snapRotate;
	else if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = m_snapTranslate;
	float snap[3] = { snapValue, snapValue, snapValue };
	bool useSnap = m_snapEnabled || ImGui::GetIO().KeyCtrl;
	bool canManipulate = m_viewportHovered || m_viewportActive || m_isUsingGizmo;

	if (canManipulate)
		ImGuizmo::Manipulate(vArr, pArr, mCurrentGizmoOperation, mCurrentGizmoMode, mArr, nullptr, useSnap ? snap : nullptr);

	m_isUsingGizmo = ImGuizmo::IsUsing();
	if (m_isUsingGizmo) {
		float newPos[3], newRot[3], newSca[3];
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);
		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
		transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));
	}
}

void GUI::drawGizmoToolbar()
{
	// Toolbar deshabilitada temporalmente.
	// Los atajos de teclado seguirán funcionando
	// porque se manejan en otro lugar del editor.
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
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.085f, 0.070f, 0.120f, 1.0f));
	if (ImGui::Begin("##StudioMenuBar", nullptr, menuFlags)) {

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::MenuItem("New Scene"); ImGui::MenuItem("Open Scene..."); ImGui::MenuItem("Save");
				ImGui::Separator();
				if (ImGui::MenuItem("Exit MinerEngine")) show_exit_popup = true;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo", "Ctrl+Z")) m_undoRequested = true;
				if (ImGui::MenuItem("Redo", "Ctrl+Y")) m_redoRequested = true;
				ImGui::Separator();
				if (ImGui::MenuItem("Copy", "Ctrl+C")) m_copyRequested = true;
				if (ImGui::MenuItem("Paste", "Ctrl+V")) m_pasteRequested = true;
				if (ImGui::MenuItem("Duplicate", "Ctrl+D")) m_duplicateRequested = true;
				if (ImGui::MenuItem("Delete", "Del")) m_deleteRequested = true;
				ImGui::Separator();
				if (ImGui::MenuItem("Save Prefab")) m_savePrefabRequested = true;
				if (ImGui::MenuItem("Load Prefab")) m_loadPrefabRequested = true;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Directional Light"))
					m_createDirectionalLightRequested = true;

				if (ImGui::MenuItem("Point Light"))
					m_createPointLightRequested = true;

				if (ImGui::MenuItem("Spot Light"))
					m_createSpotLightRequested = true;

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();

		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::SetNextWindowPos(
		ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight),
		ImGuiCond_Always
	);

	ImGui::SetNextWindowSize(
		ImVec2(viewport->Size.x, ribbonHeight),
		ImGuiCond_Always
	);

	ImGuiWindowFlags ribbonFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoScrollbar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(
		ImGuiStyleVar_WindowPadding,
		ImVec2(10.0f, 10.0f)
	);

	ImGui::PushStyleVar(
		ImGuiStyleVar_ItemSpacing,
		ImVec2(8.0f, 6.0f)
	);

	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.125f, 0.105f, 0.180f, 1.0f)
	);

	if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags))
	{
		if (ImGui::Button(
			"Play",
			ImVec2(80.0f, 32.0f)))
		{
			m_playRequested = true;
			m_pauseRequested = false;
			m_stopRequested = false;
		}

		ImGui::SameLine();

		if (ImGui::Button(
			"Pause",
			ImVec2(80.0f, 32.0f)))
		{
			m_playRequested = false;
			m_pauseRequested = true;
			m_stopRequested = false;
		}

		ImGui::SameLine();

		if (ImGui::Button(
			"Stop",
			ImVec2(80.0f, 32.0f)))
		{
			m_playRequested = false;
			m_pauseRequested = false;
			m_stopRequested = true;
		}	
	}
	ImGui::End();

	ImGui::PopStyleColor();
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
		m_viewportPos = panelMin; m_viewportSize = panelSize;
		if (viewportSRV) ImGui::Image((ImTextureID)viewportSRV, panelSize);
		else {
			ImVec2 panelMax(panelMin.x + panelSize.x, panelMin.y + panelSize.y);
			m_viewportDrawList->AddRectFilled(panelMin, panelMax, IM_COL32(24, 20, 34, 255));
			m_viewportDrawList->AddText(ImVec2(panelMin.x + 12.0f, panelMin.y + 12.0f), IM_COL32(220, 215, 240, 255), "Viewport no renderizado");
		}
		m_viewportHovered = ImGui::IsItemHovered();
		m_viewportActive = ImGui::IsItemActive();
		m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void GUI::drawViewportGrid(Camera& cam) {
	if (!m_showGrid) return;
	if (m_viewportSize.x < 16.0f || m_viewportSize.y < 16.0f) return;
	float view[16], proj[16], identity[16];
	ToFloatArray(cam.getView(), view);
	ToFloatArray(cam.getProj(), proj);
	ToFloatArray(XMMatrixIdentity(), identity);
	if (m_viewportDrawList) ImGuizmo::SetDrawlist(m_viewportDrawList);
	ImGuizmo::SetRect(m_viewportPos.x, m_viewportPos.y, m_viewportSize.x, m_viewportSize.y);
	ImGuizmo::DrawGrid(view, proj, identity, m_gridSize);
}

void GUI::drawEditorDockspace() {
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	const float topOffset = 96.0f;
	ImVec2 dockPos = ImVec2(mainViewport->Pos.x, mainViewport->Pos.y + topOffset);
	ImVec2 dockSize = ImVec2(mainViewport->Size.x, mainViewport->Size.y - topOffset);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;
	ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(mainViewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("##MainEditorDockspace", nullptr, window_flags);
	ImGuiID dockspace_id = ImGui::GetID("##EditorDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	if (!m_dockLayoutInitialized) {
		m_dockLayoutInitialized = true;
		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, dockSize);
		ImGuiID dockMain = dockspace_id;
		ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.19f, nullptr, &dockMain);
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.26f, nullptr, &dockMain);
		ImGuiID dockBottom = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.28f, nullptr, &dockMain);
		ImGuiID dockLeftBottom = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.45f, nullptr, &dockLeft);
		ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
		ImGui::DockBuilderDockWindow("Lighting", dockLeftBottom);
		ImGui::DockBuilderDockWindow("Inspector", dockRight);
		ImGui::DockBuilderDockWindow("G-Buffer", dockRight);
		ImGui::DockBuilderDockWindow("Console", dockBottom);
		ImGui::DockBuilderDockWindow("Render", dockBottom);
		ImGui::DockBuilderDockWindow("Performance", dockBottom);
		ImGui::DockBuilderDockWindow("Viewport", dockMain);
		ImGui::DockBuilderDockWindow("Content", dockBottom);
        ImGui::DockBuilderFinish(dockspace_id);
	}
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void GUI::drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
	ID3D11ShaderResourceView* normalRoughnessSRV,
	ID3D11ShaderResourceView* worldAoSRV,
	ID3D11ShaderResourceView* emissiveAlphaSRV) {
	ImGui::Begin("G-Buffer");
	const char* modes[] = { "Final Lit", "Shadow Factor", "Albedo", "World Normal", "World Position", "Metal / Rough / AO", "Emissive" };
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.55f, 1.0f, 1.0f));
	ImGui::TextUnformatted("Modo de visualizacion");
	ImGui::PopStyleColor();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::Combo("##DeferredDebugMode", &m_deferredDebugViewMode, modes, IM_ARRAYSIZE(modes));
	ImGui::Checkbox("Visualizar factor de sombra", &m_visualizeDeferredShadowFactor);
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

	auto placeholder = [&](ImVec2 size, const char* msg) {
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(25, 22, 36, 255), 4.0f);
		dl->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(140, 90, 230, 120), 4.0f);
		dl->AddText(ImVec2(p.x + 8.0f, p.y + 8.0f), IM_COL32(205, 200, 225, 255), msg);
		ImGui::Dummy(size);
		};
	float fullW = ImGui::GetContentRegionAvail().x;
	float cellW = (fullW - 8.0f) * 0.5f;
	ImVec2 cell(cellW, cellW * 0.5625f);
	auto target = [&](const char* label, ID3D11ShaderResourceView* srv) {
		ImGui::BeginGroup();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.72f, 0.98f, 1.0f));
		ImGui::TextUnformatted(label);
		ImGui::PopStyleColor();
		if (srv) {
			ImGui::Image((ImTextureID)srv, cell);
			if (ImGui::IsItemClicked()) { m_previewSRV = srv; m_previewLabel = label; m_showPreview = true; }
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Click para ampliar: %s", label);
		}
		else placeholder(cell, "N/A");
		ImGui::EndGroup();
		};
	target("Albedo + Metallic", albedoMetallicSRV);   ImGui::SameLine();
	target("Normal + Roughness", normalRoughnessSRV);
	ImGui::Spacing();
	target("World Pos + AO", worldAoSRV);             ImGui::SameLine();
	target("Emissive + Alpha", emissiveAlphaSRV);
	ImGui::End();
}

void GUI::drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
	ID3D11ShaderResourceView* viewportSRV,
	ID3D11ShaderResourceView* shadowMapSRV) {
	ImGui::Begin("Render");
	auto placeholder = [&](ImVec2 size, const char* msg) {
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(25, 22, 36, 255), 4.0f);
		dl->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(140, 90, 230, 120), 4.0f);
		dl->AddText(ImVec2(p.x + 10.0f, p.y + 10.0f), IM_COL32(205, 200, 225, 255), msg);
		ImGui::Dummy(size);
		};
	auto section = [&](const char* label, ID3D11ShaderResourceView* srv, ImVec2 size) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.55f, 1.0f, 1.0f));
		ImGui::TextUnformatted(label);
		ImGui::PopStyleColor();
		if (srv) {
			ImGui::Image((ImTextureID)srv, size);
			if (ImGui::IsItemClicked()) { m_previewSRV = srv; m_previewLabel = label; m_showPreview = true; }
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Click para ampliar: %s", label);
		}
		else placeholder(size, "Sin datos");
		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
		};
	float w = ImGui::GetContentRegionAvail().x;
	ImVec2 wide(w, w * 0.5625f);
	ImVec2 square(w, w);
	section("Shadow Map (profundidad de la luz)", shadowMapSRV, square);
	section("Pre-Shadow Pass (sin sombras)", preShadowSRV, wide);
	section("Resultado final (viewport)", viewportSRV, wide);
	ImGui::End();
}

void GUI::drawLightingPanel(float* lightDir,float* lightColor,DeferredRenderer& deferredRenderer)
{
	ImGui::Begin("Scene Settings");

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.72f, 0.28f, 0.40f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.36f, 0.48f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.20f, 0.32f, 1.0f));

		if (ImGui::Button("Reset Scene"))
		{
			m_resetRequested = true;
			m_deferredDebugViewMode = 0;
			m_visualizeDeferredShadowFactor = false;
		}

		ImGui::PopStyleColor(3);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Restablece la escena a su estado inicial.");
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Environment", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (lightDir)
			vec3Control("Sun Direction", lightDir, 0.0f, 100.0f);

		if (lightColor)
			vec3Control("Sun Color", lightColor, 1.0f, 100.0f);
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Lighting"))
	{
		ImGui::TextDisabled("Point Lights: -");
		ImGui::TextDisabled("Spot Lights : -");
		ImGui::TextDisabled("Area Lights : 0");

		ImGui::Separator();

		// ¡AQUÍ ESTÁ LA MAGIA DE LOS BOTONES!
		if (ImGui::Button("+ Add Point Light"))
		{
			m_createPointLightRequested = true;
		}

		if (ImGui::Button("+ Add Spot Light"))
		{
			m_createSpotLightRequested = true;
		}
	}

	ImGui::Spacing();

	//============================================================
	// POST PROCESSING
	//============================================================

	if (ImGui::CollapsingHeader(
		"Post Processing",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool tonemappingEnabled =
			deferredRenderer.isTonemappingEnabled();

		if (ImGui::Checkbox(
			"Tonemapping",
			&tonemappingEnabled))
		{
			deferredRenderer.setTonemappingEnabled(
				tonemappingEnabled
			);

			if (tonemappingEnabled)
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"Tonemapping activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"Tonemapping desactivado"
				);
			}
		}

		float exposure =
			deferredRenderer.getTonemappingExposure();

		if (ImGui::SliderFloat(
			"Exposure",
			&exposure,
			0.10f,
			3.00f,
			"%.2f"))
		{
			deferredRenderer.setTonemappingExposure(
				exposure
			);
		}

		float gamma =
			deferredRenderer.getTonemappingGamma();

		if (ImGui::SliderFloat(
			"Gamma",
			&gamma,
			0.50f,
			2.50f,
			"%.2f"))
		{
			deferredRenderer.setTonemappingGamma(
				gamma
			);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//====================================================
		// BLOOM
		//====================================================

		bool bloomEnabled =
			deferredRenderer.isBloomEnabled();

		if (ImGui::Checkbox(
			"Bloom",
			&bloomEnabled))
		{
			deferredRenderer.setBloomEnabled(
				bloomEnabled
			);

			if (bloomEnabled)
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"Bloom activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"Bloom desactivado"
				);
			}
		}

		float bloomThreshold =
			deferredRenderer.getBloomThreshold();

		if (ImGui::SliderFloat(
			"Bloom Threshold",
			&bloomThreshold,
			0.0f,
			3.0f,
			"%.2f"))
		{
			deferredRenderer.setBloomThreshold(
				bloomThreshold
			);
		}

		float bloomIntensity =
			deferredRenderer.getBloomIntensity();

		if (ImGui::SliderFloat(
			"Bloom Intensity",
			&bloomIntensity,
			0.0f,
			3.0f,
			"%.2f"))
		{
			deferredRenderer.setBloomIntensity(
				bloomIntensity
			);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//====================================================
		// FXAA
		//====================================================

		bool fxaaEnabled =
			deferredRenderer.isFxaaEnabled();

		if (ImGui::Checkbox(
			"FXAA",
			&fxaaEnabled))
		{
			deferredRenderer.setFxaaEnabled(
				fxaaEnabled
			);

			if (fxaaEnabled)
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"FXAA activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"FXAA desactivado"
				);
			}
		}

		ImGui::TextDisabled(
			"Suaviza bordes dentados en la imagen final."
		);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//====================================================
		// SSAO
		//====================================================

		bool ssaoEnabled =
			deferredRenderer.isSsaoEnabled();

		if (ImGui::Checkbox(
			"SSAO",
			&ssaoEnabled))
		{
			deferredRenderer.setSsaoEnabled(
				ssaoEnabled
			);

			if (ssaoEnabled)
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"SSAO activado"
				);
			}
			else
			{
				MESSAGE(
					"GUI",
					"drawLightingPanel",
					"SSAO desactivado"
				);
			}
		}

		float ssaoRadius =
			deferredRenderer.getSsaoRadius();

		if (ImGui::SliderFloat(
			"SSAO Radius",
			&ssaoRadius,
			0.05f,
			5.0f,
			"%.2f"))
		{
			deferredRenderer.setSsaoRadius(
				ssaoRadius
			);
		}

		float ssaoBias =
			deferredRenderer.getSsaoBias();

		if (ImGui::SliderFloat(
			"SSAO Bias",
			&ssaoBias,
			0.0f,
			0.20f,
			"%.3f"))
		{
			deferredRenderer.setSsaoBias(
				ssaoBias
			);
		}

		float ssaoIntensity =
			deferredRenderer.getSsaoIntensity();

		if (ImGui::SliderFloat(
			"SSAO Intensity",
			&ssaoIntensity,
			0.0f,
			5.0f,
			"%.2f"))
		{
			deferredRenderer.setSsaoIntensity(
				ssaoIntensity
			);
		}

		float ssaoPower =
			deferredRenderer.getSsaoPower();

		if (ImGui::SliderFloat(
			"SSAO Power",
			&ssaoPower,
			0.10f,
			5.0f,
			"%.2f"))
		{
			deferredRenderer.setSsaoPower(
				ssaoPower
			);
		}

		ImGui::TextDisabled(
			"Agrega sombras de contacto usando el G-Buffer."
		);

		ImGui::Spacing();

		if (ImGui::Button(
			"Reset Post Processing"))
		{
			deferredRenderer.setSsaoEnabled(
				false
			);

			deferredRenderer.setSsaoRadius(
				1.50f
			);

			deferredRenderer.setSsaoBias(
				0.02f
			);

			deferredRenderer.setSsaoIntensity(
				1.20f
			);

			deferredRenderer.setSsaoPower(
				1.50f
			);

			deferredRenderer.setFxaaEnabled(
				false
			);

			deferredRenderer.setTonemappingEnabled(
				false
			);

			deferredRenderer.setTonemappingExposure(
				1.0f
			);

			deferredRenderer.setTonemappingGamma(
				1.0f
			);

			deferredRenderer.setBloomEnabled(
				false
			);

			deferredRenderer.setBloomThreshold(
				0.80f
			);

			deferredRenderer.setBloomIntensity(
				0.80f
			);

			MESSAGE(
				"GUI",
				"drawLightingPanel",
				"Post Processing restablecido"
			);


		}
	}

	ImGui::End();
}


void GUI::drawStatsPanel(float deltaTime, unsigned int drawCalls) {
	ImGui::Begin("Performance");
	static float history[120] = {};
	static int idx = 0;
	static float accum = 0.0f; static int frames = 0;
	static float fps = 0.0f; static float ms = 0.0f;

	float dtMs = deltaTime * 1000.0f;
	history[idx] = dtMs; idx = (idx + 1) % IM_ARRAYSIZE(history);
	accum += deltaTime; frames++;
	if (accum >= 0.25f) { fps = frames / accum; ms = (accum / frames) * 1000.0f; accum = 0.0f; frames = 0; }

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.55f, 1.0f, 1.0f));
	ImGui::SetWindowFontScale(1.7f);
	ImGui::Text("%.0f FPS", fps);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::TextDisabled("  %.2f ms", ms);

	ImGui::Spacing();
	ImGui::PlotLines("##frametimes", history, IM_ARRAYSIZE(history), idx,
		"Frame time (ms)", 0.0f, 33.3f, ImVec2(ImGui::GetContentRegionAvail().x, 80.0f));

	ImGui::Spacing(); ImGui::Separator();

	// --- Metricas del frame ---
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.72f, 0.98f, 1.0f));
	ImGui::Text("Draw calls:");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::Text("%u", drawCalls);

	ImGui::TextDisabled("Viewport: %.0f x %.0f", m_viewportSize.x, m_viewportSize.y);
	ImGui::End();
}


void GUI::drawConsolePanel() {
	ImGui::Begin("Console");
	if (ImGui::Button("Clear")) Logger::get().clear();
	ImGui::SameLine();
	ImGui::Checkbox("Info", &m_logShowInfo); ImGui::SameLine();
	ImGui::Checkbox("Warning", &m_logShowWarning); ImGui::SameLine();
	ImGui::Checkbox("Error", &m_logShowError); ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &m_logAutoScroll); ImGui::SameLine();
	m_logFilter.Draw("Filter", 160.0f);
	ImGui::Separator();
	ImGui::BeginChild("ConsoleScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	std::vector<LogEntry> entries = Logger::get().snapshot();
	for (const LogEntry& e : entries) {
		if (e.level == LogLevel::Info && !m_logShowInfo) continue;
		if (e.level == LogLevel::Warning && !m_logShowWarning) continue;
		if (e.level == LogLevel::Error && !m_logShowError) continue;
		if (!m_logFilter.PassFilter(e.message.c_str())) continue;
		ImVec4 col; const char* tag;
		switch (e.level) {
		case LogLevel::Error:   col = ImVec4(0.95f, 0.40f, 0.40f, 1.0f); tag = "[ERROR] "; break;
		case LogLevel::Warning: col = ImVec4(0.95f, 0.78f, 0.30f, 1.0f); tag = "[WARN]  "; break;
		default:                col = ImVec4(0.80f, 0.78f, 0.90f, 1.0f); tag = "[INFO]  "; break;
		}
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(tag); ImGui::SameLine();
		ImGui::TextUnformatted(e.message.c_str());
		ImGui::PopStyleColor();
	}
	if (m_logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) ImGui::SetScrollHereY(1.0f);
	ImGui::EndChild();
	ImGui::End();
}

void GUI::drawTexturePreview() {
	if (!m_showPreview) return;
	ImGui::SetNextWindowSize(ImVec2(720.0f, 480.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Texture Preview", &m_showPreview)) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.55f, 1.0f, 1.0f));
		ImGui::TextUnformatted(m_previewLabel.c_str());
		ImGui::PopStyleColor();
		ImGui::Separator();
		if (m_previewSRV) {
			ImVec2 avail = ImGui::GetContentRegionAvail();
			if (avail.x < 16.0f) avail.x = 16.0f;
			if (avail.y < 16.0f) avail.y = 16.0f;
			ImGui::Image((ImTextureID)m_previewSRV, avail);
		}
	}
	ImGui::End();
}

void GUI::drawContentBrowser(const std::vector<AssetThumb>& textureThumbs) {
	ImGui::Begin("Content");

	if (ImGui::BeginTabBar("##ContentTabs")) {

		// ---- MODELS ----
		if (ImGui::BeginTabItem("Models")) {
			std::vector<std::string> models;
			WIN32_FIND_DATAA fd;
			HANDLE h = FindFirstFileA("Assets\\Models\\*", &fd);
			if (h != INVALID_HANDLE_VALUE) {
				do {
					if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
					std::string n = fd.cFileName;
					std::string lo = n;
					for (char& c : lo) if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
					if (lo.size() >= 4 && (lo.compare(lo.size() - 4, 4, ".fbx") == 0 ||
						lo.compare(lo.size() - 4, 4, ".obj") == 0))
						models.push_back(n);
				} while (FindNextFileA(h, &fd));
				FindClose(h);
			}

			if (models.empty()) ImGui::TextDisabled("No hay modelos en Assets/Models");

			const float cell = 90.0f;
			float availW = ImGui::GetContentRegionAvail().x;
			int perRow = (int)(availW / (cell + 10.0f)); if (perRow < 1) perRow = 1;
			int col = 0;
			for (const std::string& m : models) {
				ImGui::PushID(m.c_str());
				ImGui::BeginGroup();
				ImGui::Button("FBX/OBJ", ImVec2(cell, cell));
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s(doble click para instanciar)", m.c_str());
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						m_assetSpawnPath = "Assets/Models/" + m;
						m_assetSpawnRequested = true;
					}
				ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cell);
				ImGui::TextWrapped("%s", m.c_str());
				ImGui::PopTextWrapPos();
				if (ImGui::SmallButton("Spawn")) {
					m_assetSpawnPath = "Assets/Models/" + m;
					m_assetSpawnRequested = true;
				}
				ImGui::EndGroup();
				ImGui::PopID();
				if (++col < perRow) ImGui::SameLine(); else col = 0;
			}
			ImGui::EndTabItem();
		}

		// ---- TEXTURES ----
		if (ImGui::BeginTabItem("Textures")) {
			if (textureThumbs.empty()) ImGui::TextDisabled("No hay texturas cargadas");
			const float cell = 84.0f;
			float availW = ImGui::GetContentRegionAvail().x;
			int perRow = (int)(availW / (cell + 10.0f)); if (perRow < 1) perRow = 1;
			int col = 0;
			for (const AssetThumb& t : textureThumbs) {
				ImGui::PushID(t.name.c_str());
				ImGui::BeginGroup();
				if (t.srv) ImGui::Image((ImTextureID)t.srv, ImVec2(cell, cell));
				else       ImGui::Dummy(ImVec2(cell, cell));
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", t.name.c_str());
				ImGui::EndGroup();
				ImGui::PopID();
				if (++col < perRow) ImGui::SameLine(); else col = 0;
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::End();
}

void GUI::drawSelectionOutline(Camera& cam, const EU::Vector3& mn, const EU::Vector3& mx, const XMMATRIX& world) {
	if (!m_viewportDrawList) return;
	if (m_viewportSize.x < 16.0f || m_viewportSize.y < 16.0f) return;

	XMMATRIX vp = cam.getView() * cam.getProj();

	ImVec2 pts[8];
	bool valid[8];
	for (int c = 0; c < 8; ++c) {
		float cx = (c & 1) ? mx.x : mn.x;
		float cy = (c & 2) ? mx.y : mn.y;
		float cz = (c & 4) ? mx.z : mn.z;
		XMVECTOR worldC = XMVector3TransformCoord(XMVectorSet(cx, cy, cz, 1.0f), world);
		XMVECTOR clip = XMVector4Transform(
			XMVectorSet(XMVectorGetX(worldC), XMVectorGetY(worldC), XMVectorGetZ(worldC), 1.0f), vp);
		float w = XMVectorGetW(clip);
		if (w <= 0.0001f) { valid[c] = false; pts[c] = ImVec2(0, 0); continue; }
		float ndcx = XMVectorGetX(clip) / w;
		float ndcy = XMVectorGetY(clip) / w;
		float sx = m_viewportPos.x + (ndcx * 0.5f + 0.5f) * m_viewportSize.x;
		float sy = m_viewportPos.y + (1.0f - (ndcy * 0.5f + 0.5f)) * m_viewportSize.y;
		pts[c] = ImVec2(sx, sy);
		valid[c] = true;
	}

	static const int edges[12][2] = {
		{0,1},{2,3},{4,5},{6,7},   // aristas en X
		{0,2},{1,3},{4,6},{5,7},   // aristas en Y
		{0,4},{1,5},{2,6},{3,7}    // aristas en Z
	};

	// Halo (linea gruesa oscura) + linea de acento encima = se ve mas pro
	for (int e = 0; e < 12; ++e) {
		int a = edges[e][0], b = edges[e][1];
		if (valid[a] && valid[b]) {
			m_viewportDrawList->AddLine(pts[a], pts[b], IM_COL32(0, 0, 0, 160), 4.0f);
			m_viewportDrawList->AddLine(pts[a], pts[b], IM_COL32(190, 140, 255, 240), 2.0f);
		}
	}
}

void GUI::drawLightGizmo(Camera& cam, EU::TSharedPointer<Actor> actor) {
	if (!m_viewportDrawList || !actor) return;
	auto lightComp = actor->getComponent<LightComponent>();
	auto transform = actor->getComponent<Transform>();
	if (!lightComp || !transform) return;

	if (m_viewportSize.x < 16.0f || m_viewportSize.y < 16.0f) return;

	XMMATRIX vp = cam.getView() * cam.getProj();
	auto& lightData = lightComp->getLightData();
	ImU32 col = IM_COL32((int)(lightData.color.x * 255), (int)(lightData.color.y * 255), (int)(lightData.color.z * 255), 255);

	// Encendemos las tijeras para no rayar la UI
	ImVec2 clipMin = m_viewportPos;
	ImVec2 clipMax = ImVec2(m_viewportPos.x + m_viewportSize.x, m_viewportPos.y + m_viewportSize.y);
	m_viewportDrawList->PushClipRect(clipMin, clipMax, true);

	auto drawLine3D = [&](const EU::Vector3& p1, const EU::Vector3& p2) {
		XMVECTOR v1 = XMVectorSet(p1.x, p1.y, p1.z, 1.0f);
		XMVECTOR v2 = XMVectorSet(p2.x, p2.y, p2.z, 1.0f);
		XMVECTOR c1 = XMVector4Transform(v1, vp);
		XMVECTOR c2 = XMVector4Transform(v2, vp);
		float w1 = XMVectorGetW(c1);
		float w2 = XMVectorGetW(c2);

		if (w1 <= 0.1f || w2 <= 0.1f) return;

		float x1 = m_viewportPos.x + (XMVectorGetX(c1) / w1 * 0.5f + 0.5f) * m_viewportSize.x;
		float y1 = m_viewportPos.y + (1.0f - (XMVectorGetY(c1) / w1 * 0.5f + 0.5f)) * m_viewportSize.y;
		float x2 = m_viewportPos.x + (XMVectorGetX(c2) / w2 * 0.5f + 0.5f) * m_viewportSize.x;
		float y2 = m_viewportPos.y + (1.0f - (XMVectorGetY(c2) / w2 * 0.5f + 0.5f)) * m_viewportSize.y;

		m_viewportDrawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, 2.0f);
		};

	EU::Vector3 pos = transform->getPosition();
	EU::Vector3 rot = transform->getRotation();

	// --- SOLUCIÓN: Crear la Matriz Matemáticamente en Tiempo Real ---
	// Convertimos los grados (del Inspector) a radianes para DirectXMath
	float pitch = rot.x * (XM_PI / 180.0f);
	float yaw = rot.y * (XM_PI / 180.0f);
	float roll = rot.z * (XM_PI / 180.0f);

	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX worldMat = rotMat * transMat; // Matriz limpia y perfecta

	const int segments = 32;

	// 1. DIBUJAR POINT LIGHT (Esfera 3D de 3 aros)
	if (lightData.type == LightType::Point) {
		float r = lightData.range;
		for (int axis = 0; axis < 3; ++axis) {
			EU::Vector3 prevP;
			for (int i = 0; i <= segments; ++i) {
				float rad = (i / (float)segments) * XM_2PI;
				float s = sinf(rad) * r;
				float c = cosf(rad) * r;

				EU::Vector3 currentP = pos;
				if (axis == 0) { currentP.y += c; currentP.z += s; }
				else if (axis == 1) { currentP.x += c; currentP.z += s; }
				else if (axis == 2) { currentP.x += c; currentP.y += s; }

				if (i > 0) drawLine3D(prevP, currentP);
				prevP = currentP;
			}
		}
	}
	// 2. DIBUJAR SPOT LIGHT (Cono 3D con rotación)
	else if (lightData.type == LightType::Spot) {
		float r = lightData.range;
		float radius = r * tanf((lightData.spotAngle * 0.5f) * (XM_PI / 180.0f));

		EU::Vector3 prevP;
		for (int i = 0; i <= segments; ++i) {
			float rad = (i / (float)segments) * XM_2PI;
			float x = cosf(rad) * radius;
			float z = sinf(rad) * radius;

			// Cono apunta hacia abajo (-Y)
			XMVECTOR localP = XMVectorSet(x, -r, z, 1.0f);
			XMVECTOR worldP = XMVector4Transform(localP, worldMat);
			EU::Vector3 currentP(XMVectorGetX(worldP), XMVectorGetY(worldP), XMVectorGetZ(worldP));

			if (i > 0) drawLine3D(prevP, currentP);
			prevP = currentP;

			// Dibujar 4 líneas desde la punta hasta los bordes
			if (i % (segments / 4) == 0) {
				drawLine3D(pos, currentP);
			}
		}
	}
	// 3. DIBUJAR DIRECTIONAL LIGHT (Cilindro con flecha central)
	else if (lightData.type == LightType::Directional) {
		EU::Vector3 prevP;
		// Dibujar el aro superior
		for (int i = 0; i <= segments; ++i) {
			float rad = (i / (float)segments) * XM_2PI;
			XMVECTOR localP = XMVectorSet(cosf(rad) * 1.0f, 0, sinf(rad) * 1.0f, 1.0f);
			XMVECTOR worldP = XMVector4Transform(localP, worldMat);
			EU::Vector3 currentP(XMVectorGetX(worldP), XMVectorGetY(worldP), XMVectorGetZ(worldP));
			if (i > 0) drawLine3D(prevP, currentP);
			prevP = currentP;
		}

		// Flecha central sólida apuntando dirección (-Y)
		XMVECTOR localTip = XMVectorSet(0.0f, -3.0f, 0.0f, 1.0f);
		XMVECTOR worldTip = XMVector4Transform(localTip, worldMat);
		EU::Vector3 tipP(XMVectorGetX(worldTip), XMVectorGetY(worldTip), XMVectorGetZ(worldTip));
		drawLine3D(pos, tipP);

		// 4 líneas laterales para dar volumen de rayo de luz
		for (int i = 0; i < 4; ++i) {
			float rad = (i / 4.0f) * XM_2PI;
			XMVECTOR locBase = XMVectorSet(cosf(rad) * 1.0f, 0, sinf(rad) * 1.0f, 1.0f);
			XMVECTOR locTip2 = XMVectorSet(cosf(rad) * 1.0f, -3.0f, sinf(rad) * 1.0f, 1.0f);
			XMVECTOR wBase = XMVector4Transform(locBase, worldMat);
			XMVECTOR wTip = XMVector4Transform(locTip2, worldMat);
			EU::Vector3 pB(XMVectorGetX(wBase), XMVectorGetY(wBase), XMVectorGetZ(wBase));
			EU::Vector3 pT(XMVectorGetX(wTip), XMVectorGetY(wTip), XMVectorGetZ(wTip));
			drawLine3D(pB, pT);
		}
	}

	m_viewportDrawList->PopClipRect();

}
