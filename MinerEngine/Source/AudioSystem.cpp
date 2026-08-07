#include <Audio.h>

#include "AudioSystem.h"
#include "StreamingAudioPlayer.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h"
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
		std::unique_ptr<DirectX::SoundEffect> sound;
		std::unique_ptr<DirectX::SoundEffectInstance> instance;

		std::string filePath;

		// Actor que contiene la fuente de audio.
		EU::TSharedPointer<Actor> actor;

		// Indica si esta instancia utiliza audio 3D.
		bool spatial3D = false;

		// -1 significa sonido principal.
        // 0 o mayor representa sounds[index].
		int soundIndex = -1;

		// Posición y propiedades espaciales del emisor.
		DirectX::AudioEmitter emitter;
	};

	std::unique_ptr<DirectX::AudioEngine> engine;

	// Sonidos WAV cortos cargados completamente en memoria.
	std::vector<ActiveSound> activeSounds;

	// Listener 3D asociado a la cámara.
	DirectX::AudioListener listener;

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
// ACTUALIZAR AUDIO 3D
//============================================================

void
AudioSystem::update3D(
	const EU::Vector3& listenerPosition,
	const EU::Vector3& listenerForward,
	const EU::Vector3& listenerUp)
{
	if (!m_impl || !m_impl->engine)
	{
		return;
	}

	//========================================================
	// ACTUALIZAR LISTENER
	//========================================================

	m_impl->listener.SetPosition(
		DirectX::XMFLOAT3(
			listenerPosition.x,
			listenerPosition.y,
			listenerPosition.z
		)
	);

	m_impl->listener.SetOrientation(
		DirectX::XMFLOAT3(
			listenerForward.x,
			listenerForward.y,
			listenerForward.z
		),
		DirectX::XMFLOAT3(
			listenerUp.x,
			listenerUp.y,
			listenerUp.z
		)
	);

	//========================================================
	// ACTUALIZAR SONIDOS ACTIVOS
	//========================================================

	for (auto& activeSound :
		m_impl->activeSounds)
	{
		if (!activeSound.instance ||
			activeSound.actor.isNull())
		{
			continue;
		}

		auto audioSource =
			activeSound.actor
			->getComponent<AudioSourceComponent>();

		if (!audioSource)
		{
			continue;
		}

		//====================================================
		// VOLUMEN EN TIEMPO REAL
		//====================================================

		float currentVolume = 1.0f;

		if (activeSound.soundIndex < 0)
		{
			// Sonido principal.
			currentVolume =
				audioSource->volume;
		}
		else if (
			activeSound.soundIndex <
			static_cast<int>(
				audioSource->sounds.size()
				))
		{
			// Sonido adicional.
			currentVolume =
				audioSource
				->sounds[activeSound.soundIndex]
				.volume;
		}

		activeSound.instance->SetVolume(
			currentVolume
		);

		// Los sonidos 2D solamente necesitan actualizar volumen.
		if (!activeSound.spatial3D)
		{
			continue;
		}

		//====================================================
		// POSICION DEL EMISOR 3D
		//====================================================

		auto transform =
			activeSound.actor
			->getComponent<Transform>();

		if (!transform)
		{
			continue;
		}

		const EU::Vector3& sourcePosition =
			transform->getPosition();

		activeSound.emitter.SetPosition(
			DirectX::XMFLOAT3(
				sourcePosition.x,
				sourcePosition.y,
				sourcePosition.z
			)
		);

		activeSound.emitter.CurveDistanceScaler =
			20.0f;

		activeSound.emitter.DopplerScaler =
			1.0f;

		activeSound.instance->Apply3D(
			m_impl->listener,
			activeSound.emitter,
			false
		);
	}
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
		 [this, &actor](
				const char* filePath,
				float volume,
				bool loop,
			    bool playOnStart,
			    bool spatial3D,
			    int soundIndex)
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

					Impl::ActiveSound activeSound;

					// Guardar la ruta del archivo.
					activeSound.filePath =
						path;

					// Guardar el actor propietario.
					activeSound.actor =
						actor;

					// Guardar si utiliza audio 3D.
					activeSound.spatial3D =
						spatial3D;

					// -1 para el sonido principal.
					// 0 o mayor para sonidos adicionales.
					activeSound.soundIndex =
						soundIndex;

					// Cargar el WAV.
					activeSound.sound =
						std::make_unique<
						DirectX::SoundEffect
						>(
							m_impl->engine.get(),
							widePath.c_str()
						);

					// Configurar la instancia como 2D o 3D.
					DirectX::
						SOUND_EFFECT_INSTANCE_FLAGS
						instanceFlags =
						DirectX::
						SoundEffectInstance_Default;

					if (spatial3D)
					{
						instanceFlags =
							DirectX::
							SoundEffectInstance_Use3D;
					}

					// Crear una sola instancia.
					activeSound.instance =
						activeSound.sound
						->CreateInstance(
							instanceFlags
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

					// Aplicar volumen inicial.
					activeSound.instance->SetVolume(
						volume
					);

					// Aplicar posición inicial si es 3D.
					if (spatial3D)
					{
						auto transform =
							actor->getComponent<Transform>();

						if (transform)
						{
							const EU::Vector3&
								sourcePosition =
								transform->getPosition();

							activeSound.emitter.SetPosition(
								DirectX::XMFLOAT3(
									sourcePosition.x,
									sourcePosition.y,
									sourcePosition.z
								)
							);

							// Alcance aproximado de 20
							// unidades del mundo.
							activeSound.emitter
								.CurveDistanceScaler =
								20.0f;

							activeSound.emitter
								.DopplerScaler =
								1.0f;

							// false porque el motor utiliza
							// coordenadas Left-Handed.
							activeSound.instance->Apply3D(
								m_impl->listener,
								activeSound.emitter,
								false
							);
						}
					}

					// Reproducir la instancia.
					activeSound.instance->Play(
						loop
					);

					// Guardar exactamente la misma instancia
					// para actualizar volumen y posición.
					m_impl->activeSounds.push_back(
						std::move(activeSound)
					);

					if (spatial3D)
					{
						MESSAGE(
							"AudioSystem",
							"playOnStart",
							"Audio WAV 3D reproducido"
						);
					}
					else
					{
						MESSAGE(
							"AudioSystem",
							"playOnStart",
							"Audio WAV 2D reproducido"
						);
					}

				
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
				audioSource->spatial3D,
				-1
			);

		// Sonidos adicionales.
			for (int soundIndex = 0;
				soundIndex <
				static_cast<int>(audioSource->sounds.size());
				++soundIndex)
			{
				const AudioClipData& sound =
					audioSource->sounds[soundIndex];

				playSound(
					sound.filePath,
					sound.volume,
					sound.loop,
					sound.playOnStart,
					sound.spatial3D,
					soundIndex
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