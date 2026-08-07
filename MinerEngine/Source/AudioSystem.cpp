#include <Audio.h>

#include "AudioSystem.h"
#include "ECS/Actor.h"
#include "ECS/AudioSourceComponent.h"

#include <exception>
#include <string>
#include <utility>
#include <vector>

struct AudioSystem::Impl
{
	struct ActiveSound
	{
		// Debe declararse antes que instance.
		std::unique_ptr<DirectX::SoundEffect> sound;
		std::unique_ptr<DirectX::SoundEffectInstance> instance;

		std::string filePath;
	};

	std::unique_ptr<DirectX::AudioEngine> engine;
	std::vector<ActiveSound> activeSounds;

	bool criticalErrorReported = false;
};

AudioSystem::AudioSystem()
	: m_impl(std::make_unique<Impl>())
{
}

AudioSystem::~AudioSystem()
{
	shutdown();
}

bool
AudioSystem::init()
{
	if (!m_impl)
	{
		m_impl =
			std::make_unique<Impl>();
	}

	if (m_impl->engine)
	{
		return true;
	}

	DirectX::AUDIO_ENGINE_FLAGS flags =
		DirectX::AudioEngine_Default;

#ifdef _DEBUG
	flags |= DirectX::AudioEngine_Debug;
#endif

	try
	{
		m_impl->engine =
			std::make_unique<DirectX::AudioEngine>(
				flags
			);

		if (m_impl->engine->IsAudioDevicePresent())
		{
			MESSAGE(
				"AudioSystem",
				"init",
				"AudioEngine inicializado correctamente"
			);

			return true;
		}

		ERROR(
			"AudioSystem",
			"init",
			"No se detecto un dispositivo de audio"
		);
	}
	catch (const std::exception& exception)
	{
		ERROR(
			"AudioSystem",
			"init",
			exception.what()
		);

		m_impl->engine.reset();
	}

	return false;
}

void
AudioSystem::update()
{
	if (!m_impl || !m_impl->engine)
	{
		return;
	}

	if (!m_impl->engine->Update() &&
		m_impl->engine->IsCriticalError())
	{
		if (!m_impl->criticalErrorReported)
		{
			ERROR(
				"AudioSystem",
				"update",
				"El dispositivo de audio encontro un error critico"
			);

			m_impl->criticalErrorReported = true;
		}

		return;
	}

	m_impl->criticalErrorReported = false;
}

void
AudioSystem::playOnStart(
	const std::vector<EU::TSharedPointer<Actor>>& actors)
{
	if (!m_impl || !m_impl->engine)
	{
		ERROR(
			"AudioSystem",
			"playOnStart",
			"AudioEngine no esta disponible"
		);

		return;
	}

	// Evitar instancias duplicadas al entrar nuevamente en Play.
	stopAll();

	for (const auto& actor : actors)
	{
		if (actor.isNull())
		{
			continue;
		}

		auto audioSource =
			actor->getComponent<AudioSourceComponent>();

		if (!audioSource ||
			!audioSource->isEnabled() ||
			!audioSource->playOnStart)
		{
			continue;
		}

		if (audioSource->filePath[0] == '\0')
		{
			ERROR(
				"AudioSystem",
				"playOnStart",
				"Audio Source no tiene archivo"
			);

			continue;
		}

		if (audioSource->spatial3D)
		{
			MESSAGE(
				"AudioSystem",
				"playOnStart",
				"Spatial 3D se implementara en el siguiente paso"
			);
		}

		try
		{
			std::string path =
				audioSource->filePath;

			std::wstring widePath(
				path.begin(),
				path.end()
			);

			Impl::ActiveSound activeSound;

			activeSound.filePath = path;

			activeSound.sound =
				std::make_unique<DirectX::SoundEffect>(
					m_impl->engine.get(),
					widePath.c_str()
				);

			activeSound.instance =
				activeSound.sound->CreateInstance();

			activeSound.instance->SetVolume(
				audioSource->volume
			);

			activeSound.instance->Play(
				audioSource->loop
			);

			m_impl->activeSounds.push_back(
				std::move(activeSound)
			);

			MESSAGE(
				"AudioSystem",
				"playOnStart",
				"Audio Source reproducido"
			);
		}
		catch (const std::exception& exception)
		{
			ERROR(
				"AudioSystem",
				"playOnStart",
				exception.what()
			);
		}
	}
}

void
AudioSystem::pauseAll()
{
	if (!m_impl)
	{
		return;
	}

	for (auto& activeSound : m_impl->activeSounds)
	{
		if (activeSound.instance)
		{
			activeSound.instance->Pause();
		}
	}

	MESSAGE(
		"AudioSystem",
		"pauseAll",
		"Todos los sonidos fueron pausados"
	);
}

void
AudioSystem::resumeAll()
{
	if (!m_impl)
	{
		return;
	}

	for (auto& activeSound : m_impl->activeSounds)
	{
		if (activeSound.instance)
		{
			activeSound.instance->Resume();
		}
	}

	MESSAGE(
		"AudioSystem",
		"resumeAll",
		"Todos los sonidos fueron reanudados"
	);
}

void
AudioSystem::stopAll()
{
	if (!m_impl)
	{
		return;
	}

	for (auto& activeSound : m_impl->activeSounds)
	{
		if (activeSound.instance)
		{
			// true fuerza la detención inmediata,
			// incluso cuando el sonido está en loop.
			activeSound.instance->Stop(true);
		}
	}

	// Destruir primero las instancias y después sus SoundEffect.
	m_impl->activeSounds.clear();

	MESSAGE(
		"AudioSystem",
		"stopAll",
		"Todos los sonidos fueron detenidos"
	);
}

void
AudioSystem::shutdown()
{
	if (!m_impl)
	{
		return;
	}

	stopAll();

	// AudioEngine debe destruirse después de las instancias.
	m_impl->engine.reset();
}

bool
AudioSystem::isReady() const
{
	return
		m_impl &&
		m_impl->engine &&
		m_impl->engine->IsAudioDevicePresent();
}