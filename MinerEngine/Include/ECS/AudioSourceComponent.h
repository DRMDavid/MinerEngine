#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

#include <cstring>
#include <vector>

class DeviceContext;

//============================================================
// DATOS DE UN SONIDO ADICIONAL
//============================================================

struct AudioClipData
{
	AudioClipData()
	{
		filePath[0] = '\0';
	}

	// Ruta relativa al directorio bin.
	char filePath[260] = {};

	// Volumen entre 0 y 1.
	float volume = 1.0f;

	// Repetir continuamente.
	bool loop = false;

	// Reproducir al entrar en Play.
	bool playOnStart = false;

	// Audio posicional.
	bool spatial3D = false;

	// Solicitudes temporales enviadas desde el Inspector.
	bool previewRequested = false;
	bool stopPreviewRequested = false;
};

/**
 * @class AudioSourceComponent
 * @brief Componente que almacena uno o varios sonidos.
 */
class AudioSourceComponent : public Component
{
public:

	AudioSourceComponent()
		: Component(ComponentType::AUDIO_SOURCE)
	{
		strcpy_s(
			filePath,
			sizeof(filePath),
			"Assets/Audio/Test.wav"
		);
	}

	void init() override
	{
	}

	void update(float deltaTime) override
	{
		(void)deltaTime;
	}

	void render(DeviceContext& deviceContext) override
	{
		(void)deviceContext;
	}

	void destroy() override
	{
		sounds.clear();
	}

public:

	//========================================================
	// SONIDO PRINCIPAL
	// Se conserva para no romper el sistema actual.
	//========================================================

	char filePath[260] = {};

	float volume = 1.0f;

	bool loop = false;

	bool playOnStart = true;

	bool spatial3D = false;

	bool previewRequested = false;

	bool stopPreviewRequested = false;

	//========================================================
	// SONIDOS ADICIONALES
	//========================================================

	std::vector<AudioClipData> sounds;
};