#include <Audio.h>

#include "AudioSystem.h"
#include "ECS/Actor.h"
#include "ECS/AudioSourceComponent.h"

#include <exception>
#include <string>
#include <utility>
#include <vector>

//============================================================
// IMPLEMENTACION PRIVADA
//============================================================

struct AudioSystem::Impl
{
	struct ActiveSound
	{
		// SoundEffect se declara antes que instance.
		// De esta forma instance se destruye primero.
		std::unique_ptr<DirectX::SoundEffect> sound;
		std::unique_ptr<DirectX::SoundEffectInstance> instance;

		std::string filePath;
	};

	std::unique_ptr<DirectX::AudioEngine> engine;

	std::vector<ActiveSound> activeSounds;

	// Sonido utilizado por el Preview del Inspector.
	std::unique_ptr<DirectX::SoundEffect> previewSound;
	std::unique_ptr<DirectX::SoundEffectInstance> previewInstance;

	bool criticalErrorReported = false;
};

//============================================================
// CONSTRUCTOR
//============================================================

AudioSystem::AudioSystem()
	: m_impl(std::make_unique<Impl>())
{
}

//============================================================
// DESTRUCTOR
//============================================================

AudioSystem::~AudioSystem()
{
	shutdown();
}

//============================================================
// INICIALIZACION
//============================================================

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

//============================================================
// ACTUALIZAR AUDIO ENGINE
//============================================================

void
AudioSystem::update()
{
	if (!m_impl || !m_impl->engine)
	{
		return;
	}

	const bool updateResult =
		m_impl->engine->Update();

	if (!updateResult &&
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

//============================================================
// DETENER PREVIEW
//============================================================

void
AudioSystem::stopPreview()
{
	if (!m_impl)
	{
		return;
	}

	const bool hadPreview =
		m_impl->previewInstance != nullptr;

	if (m_impl->previewInstance)
	{
		// true fuerza una detencion inmediata,
		// incluso cuando el sonido esta en loop.
		m_impl->previewInstance->Stop(true);
		m_impl->previewInstance.reset();
	}

	// SoundEffect debe destruirse despues de su instancia.
	m_impl->previewSound.reset();

	if (hadPreview)
	{
		MESSAGE(
			"AudioSystem",
			"stopPreview",
			"Preview detenido"
		);
	}
}

//============================================================
// PROCESAR SOLICITUDES DE PREVIEW
//============================================================

void
AudioSystem::processPreviewRequests(
	const std::vector<EU::TSharedPointer<Actor>>& actors)
{
	if (!m_impl || !m_impl->engine)
	{
		return;
	}

	for (const auto& actor : actors)
	{
		if (actor.isNull())
		{
			continue;
		}

		auto audioSource =
			actor->getComponent<AudioSourceComponent>();

		if (!audioSource)
		{
			continue;
		}

		// Función local para procesar cualquier sonido.
		auto processSoundPreview =
			[this](
				char* filePath,
				float volume,
				bool loop,
				bool& previewRequested,
				bool& stopPreviewRequested)
			{
				if (stopPreviewRequested)
				{
					stopPreviewRequested = false;
					previewRequested = false;

					stopPreview();
				}

				if (!previewRequested)
				{
					return;
				}

				// Consumir la solicitud para no repetirla
				// automáticamente cada frame.
				previewRequested = false;

				if (filePath == nullptr ||
					filePath[0] == '\0')
				{
					ERROR(
						"AudioSystem",
						"processPreviewRequests",
						"No se selecciono un archivo de audio"
					);

					return;
				}

				try
				{
					// Solo puede existir un Preview activo.
					stopPreview();

					const std::string path =
						filePath;

					const std::wstring widePath(
						path.begin(),
						path.end()
					);

					m_impl->previewSound =
						std::make_unique<DirectX::SoundEffect>(
							m_impl->engine.get(),
							widePath.c_str()
						);

					m_impl->previewInstance =
						m_impl->previewSound->CreateInstance(
							DirectX::SoundEffectInstance_Default
						);

					if (!m_impl->previewInstance)
					{
						ERROR(
							"AudioSystem",
							"processPreviewRequests",
							"No se pudo crear el Preview"
						);

						m_impl->previewSound.reset();
						return;
					}

					m_impl->previewInstance->SetVolume(
						volume
					);

					m_impl->previewInstance->Play(
						loop
					);

					MESSAGE(
						"AudioSystem",
						"processPreviewRequests",
						"Reproduciendo Preview"
					);
				}
				catch (const std::exception& exception)
				{
					stopPreview();

					ERROR(
						"AudioSystem",
						"processPreviewRequests",
						exception.what()
					);
				}
			};

		// Procesar el sonido principal.
		processSoundPreview(
			audioSource->filePath,
			audioSource->volume,
			audioSource->loop,
			audioSource->previewRequested,
			audioSource->stopPreviewRequested
		);

		// Procesar todos los sonidos adicionales.
		for (AudioClipData& sound : audioSource->sounds)
		{
			processSoundPreview(
				sound.filePath,
				sound.volume,
				sound.loop,
				sound.previewRequested,
				sound.stopPreviewRequested
			);
		}
	}
}
//============================================================
// REPRODUCIR AL ENTRAR EN PLAY
//============================================================

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

	// Detener Preview y sonidos de una ejecución anterior.
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
			!audioSource->isEnabled())
		{
			continue;
		}

		// Función local para reproducir cualquier sonido.
		auto playSound =
			[this](
				const char* filePath,
				float volume,
				bool loop,
				bool playOnStart,
				bool spatial3D)
			{
				if (!playOnStart)
				{
					return;
				}

				if (filePath == nullptr ||
					filePath[0] == '\0')
				{
					ERROR(
						"AudioSystem",
						"playOnStart",
						"Audio Source no tiene archivo"
					);

					return;
				}

				if (spatial3D)
				{
					MESSAGE(
						"AudioSystem",
						"playOnStart",
						"Spatial 3D se implementara posteriormente"
					);
				}

				try
				{
					const std::string path =
						filePath;

					const std::wstring widePath(
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
						activeSound.sound->CreateInstance(
							DirectX::SoundEffectInstance_Default
						);

					if (!activeSound.instance)
					{
						ERROR(
							"AudioSystem",
							"playOnStart",
							"No se pudo crear la instancia"
						);

						return;
					}

					activeSound.instance->SetVolume(
						volume
					);

					activeSound.instance->Play(
						loop
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
			};

		// Reproducir el sonido principal.
		playSound(
			audioSource->filePath,
			audioSource->volume,
			audioSource->loop,
			audioSource->playOnStart,
			audioSource->spatial3D
		);

		// Reproducir los sonidos adicionales.
		for (const AudioClipData& sound :
			audioSource->sounds)
		{
			playSound(
				sound.filePath,
				sound.volume,
				sound.loop,
				sound.playOnStart,
				sound.spatial3D
			);
		}
	}
}

//============================================================
// PAUSAR SONIDOS
//============================================================

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

//============================================================
// REANUDAR SONIDOS
//============================================================

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

//============================================================
// DETENER TODOS LOS SONIDOS
//============================================================

void
AudioSystem::stopAll()
{
	if (!m_impl)
	{
		return;
	}

	// Detener también el Preview del Inspector.
	stopPreview();

	for (auto& activeSound : m_impl->activeSounds)
	{
		if (activeSound.instance)
		{
			// true fuerza una detencion inmediata,
			// incluso cuando el sonido esta en loop.
			activeSound.instance->Stop(true);
		}
	}

	// Destruye primero SoundEffectInstance y después SoundEffect.
	m_impl->activeSounds.clear();

	MESSAGE(
		"AudioSystem",
		"stopAll",
		"Todos los sonidos fueron detenidos"
	);
}

//============================================================
// DESTRUIR SISTEMA
//============================================================

void
AudioSystem::shutdown()
{
	if (!m_impl)
	{
		return;
	}

	stopAll();

	// AudioEngine se destruye después de todos los sonidos.
	m_impl->engine.reset();

	m_impl->criticalErrorReported = false;
}

//============================================================
// COMPROBAR ESTADO
//============================================================

bool
AudioSystem::isReady() const
{
	return
		m_impl &&
		m_impl->engine &&
		m_impl->engine->IsAudioDevicePresent();
}	