#include <Audio.h>

#include "AudioSystem.h"
#include "StreamingAudioPlayer.h"
#include "ECS/Actor.h"
#include "ECS/AudioSourceComponent.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <string>
#include <utility>
#include <vector>



//============================================================
// DETERMINAR SI EL ARCHIVO NECESITA STREAMING
//============================================================

static bool
shouldStreamAudio(const std::string& filePath)
{
	std::string lowerPath =
		filePath;

	std::transform(
		lowerPath.begin(),
		lowerPath.end(),
		lowerPath.begin(),
		[](unsigned char character)
		{
			return static_cast<char>(
				std::tolower(character)
				);
		}
	);

	// Los MP3 siempre utilizan Media Foundation.
	const bool isMp3 =
		lowerPath.size() >= 4 &&
		lowerPath.substr(
			lowerPath.size() - 4
		) == ".mp3";

	if (isMp3)
	{
		return true;
	}

	// Consultar el tamaño de los WAV.
	WIN32_FILE_ATTRIBUTE_DATA fileData{};

	if (!GetFileAttributesExA(
		filePath.c_str(),
		GetFileExInfoStandard,
		&fileData))
	{
		return false;
	}

	ULARGE_INTEGER fileSize{};

	fileSize.HighPart =
		fileData.nFileSizeHigh;

	fileSize.LowPart =
		fileData.nFileSizeLow;

	// A partir de 8 MB se reproduce por streaming.
	const ULONGLONG streamingThreshold =
		8ULL * 1024ULL * 1024ULL;

	return
		fileSize.QuadPart >=
		streamingThreshold;
}
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

	// Sonidos WAV cortos cargados completamente en memoria.
	std::vector<ActiveSound> activeSounds;

	// Reproductores MP3 o archivos largos por streaming.
	std::vector<std::unique_ptr<StreamingAudioPlayer>>
		activeStreams;

	// Preview WAV corto.
	std::unique_ptr<DirectX::SoundEffect> previewSound;
	std::unique_ptr<DirectX::SoundEffectInstance> previewInstance;

	// Preview MP3 o archivo largo.
	std::unique_ptr<StreamingAudioPlayer> previewStream;

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
		m_impl->previewInstance != nullptr ||
		m_impl->previewStream != nullptr;

	// Detener Preview WAV.
	if (m_impl->previewInstance)
	{
		m_impl->previewInstance->Stop(true);
		m_impl->previewInstance.reset();
	}

	m_impl->previewSound.reset();

	// Detener Preview MP3 o streaming.
	if (m_impl->previewStream)
	{
		m_impl->previewStream->stop();
		m_impl->previewStream->shutdown();
		m_impl->previewStream.reset();
	}

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

		auto processSoundPreview =
			[this](
				char* filePath,
				float volume,
				bool loop,
				bool& previewRequested,
				bool& stopPreviewRequested)
			{
				//============================================
				// DETENER PREVIEW
				//============================================

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

				// Consumir la solicitud.
				previewRequested = false;

				if (filePath == nullptr ||
					filePath[0] == '\0')
				{
					ERROR(
						"AudioSystem",
						"processPreviewRequests",
						"No se selecciono un archivo"
					);

					return;
				}

				try
				{
					stopPreview();

					const std::string path =
						filePath;

					const std::wstring widePath(
						path.begin(),
						path.end()
					);

					//========================================
					// MP3 O ARCHIVO GRANDE
					//========================================

					if (shouldStreamAudio(path))
					{
						auto stream =
							std::make_unique<
							StreamingAudioPlayer
							>();

						if (!stream->init())
						{
							ERROR(
								"AudioSystem",
								"processPreviewRequests",
								"No se pudo iniciar streaming"
							);

							return;
						}

						if (!stream->load(widePath))
						{
							stream->shutdown();

							ERROR(
								"AudioSystem",
								"processPreviewRequests",
								"No se pudo cargar el archivo"
							);

							return;
						}

						stream->setVolume(
							volume
						);

						stream->setLoop(
							loop
						);

						stream->play();

						m_impl->previewStream =
							std::move(stream);

						MESSAGE(
							"AudioSystem",
							"processPreviewRequests",
							"Reproduciendo Preview por streaming"
						);

						return;
					}

					//========================================
					// WAV CORTO
					//========================================

					m_impl->previewSound =
						std::make_unique<
						DirectX::SoundEffect
						>(
							m_impl->engine.get(),
							widePath.c_str()
						);

					m_impl->previewInstance =
						m_impl->previewSound
						->CreateInstance(
							DirectX::
							SoundEffectInstance_Default
						);

					if (!m_impl->previewInstance)
					{
						m_impl->previewSound.reset();

						ERROR(
							"AudioSystem",
							"processPreviewRequests",
							"No se pudo crear el Preview"
						);

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
						"Reproduciendo Preview WAV"
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

		//============================================
		// SONIDO PRINCIPAL
		//============================================

		processSoundPreview(
			audioSource->filePath,
			audioSource->volume,
			audioSource->loop,
			audioSource->previewRequested,
			audioSource->stopPreviewRequested
		);

		//============================================
		// SONIDOS ADICIONALES
		//============================================

		for (AudioClipData& sound :
			audioSource->sounds)
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

	// Detener Preview y sonidos anteriores.
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

				try
				{
					const std::string path =
						filePath;

					const std::wstring widePath(
						path.begin(),
						path.end()
					);

					//========================================
					// MP3 O ARCHIVO GRANDE
					//========================================

					if (shouldStreamAudio(path))
					{
						auto stream =
							std::make_unique<
							StreamingAudioPlayer
							>();

						if (!stream->init())
						{
							ERROR(
								"AudioSystem",
								"playOnStart",
								"No se pudo iniciar streaming"
							);

							return;
						}

						if (!stream->load(widePath))
						{
							stream->shutdown();

							ERROR(
								"AudioSystem",
								"playOnStart",
								"No se pudo cargar el streaming"
							);

							return;
						}

						stream->setVolume(
							volume
						);

						stream->setLoop(
							loop
						);

						stream->play();

						m_impl->activeStreams.push_back(
							std::move(stream)
						);

						MESSAGE(
							"AudioSystem",
							"playOnStart",
							"Audio reproducido por streaming"
						);

						return;
					}

					//========================================
					// WAV CORTO
					//========================================

					if (spatial3D)
					{
						MESSAGE(
							"AudioSystem",
							"playOnStart",
							"Spatial 3D se implementara posteriormente"
						);
					}

					Impl::ActiveSound activeSound;

					activeSound.filePath =
						path;

					activeSound.sound =
						std::make_unique<
						DirectX::SoundEffect
						>(
							m_impl->engine.get(),
							widePath.c_str()
						);

					activeSound.instance =
						activeSound.sound
						->CreateInstance(
							DirectX::
							SoundEffectInstance_Default
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
						"Audio WAV reproducido"
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

		// Sonido principal.
		playSound(
			audioSource->filePath,
			audioSource->volume,
			audioSource->loop,
			audioSource->playOnStart,
			audioSource->spatial3D
		);

		// Sonidos adicionales.
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

	for (auto& stream : m_impl->activeStreams)
	{
		if (stream)
		{
			stream->pause();
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

	for (auto& stream : m_impl->activeStreams)
	{
		if (stream)
		{
			stream->resume();
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

	// Detener cualquier Preview.
	stopPreview();

	// Detener WAV cortos.
	for (auto& activeSound : m_impl->activeSounds)
	{
		if (activeSound.instance)
		{
			activeSound.instance->Stop(true);
		}
	}

	m_impl->activeSounds.clear();

	// Detener MP3 y archivos largos.
	for (auto& stream : m_impl->activeStreams)
	{
		if (stream)
		{
			stream->stop();
			stream->shutdown();
		}
	}

	m_impl->activeStreams.clear();

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