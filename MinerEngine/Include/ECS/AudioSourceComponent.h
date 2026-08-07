#pragma once

#include "Prerequisites.h"
#include "ECS/Component.h"

#include <cstring>

class DeviceContext;

/**
 * @class AudioSourceComponent
 * @brief Configuracion de una fuente de audio asociada a un actor.
 *
 * Contiene las propiedades configurables del sonido.
 * AudioSystem se encarga de cargarlo y reproducirlo.
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
		// Evitar advertencia por parametro no utilizado.
		(void)deltaTime;
	}

	void render(DeviceContext& deviceContext) override
	{
		// Evitar advertencia por parametro no utilizado.
		(void)deviceContext;
	}

	void destroy() override
	{
	}

public:

	// Ruta relativa al directorio bin del motor.
	char filePath[260] = {};

	// Volumen entre 0 y 1.
	float volume = 1.0f;

	// Repetir continuamente.
	bool loop = false;

	// Reproducir cuando comienza el modo Play.
	bool playOnStart = true;

	// Activar audio posicional 3D.
	// Su funcionamiento se implementara posteriormente.
	bool spatial3D = false;

	// Solicitud enviada desde el Inspector para escuchar el sonido.
	// No se guarda como propiedad permanente del componente.
	bool previewRequested = false;

	// Solicitud enviada desde el Inspector para detener el Preview.
	// No se guarda como propiedad permanente del componente.
	bool stopPreviewRequested = false;
};