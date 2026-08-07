#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

#include <cstring>

class DeviceContext;

/**
 * @class AudioSourceComponent
 * @brief Configuración de una fuente de audio asociada a un actor.
 *
 * En esta primera etapa contiene únicamente los datos configurables.
 * AudioSystem se encargará posteriormente de cargar y reproducir el sonido.
 */
class AudioSourceComponent : public Component
{
public:

	AudioSourceComponent()
		: Component(ComponentType::AUDIO_SOURCE)
	{
		strcpy_s(
			filePath,
			"Assets/Audio/Test.wav"
		);
	}

	void init() override {}

	void update(float deltaTime) override {}

	void render(DeviceContext& deviceContext) override {}

	void destroy() override {}

public:

	// Ruta relativa al directorio bin del motor.
	char filePath[260] = {};

	// Volumen entre 0 y 1.
	float volume = 1.0f;

	// Repetir continuamente.
	bool loop = false;

	// Reproducir cuando comienza Play.
	bool playOnStart = true;

	// Activar audio posicional 3D.
	bool spatial3D = false;
};