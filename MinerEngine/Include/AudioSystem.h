#pragma once

#include "Prerequisites.h"

#include <memory>
#include <vector>

class Actor;

/**
 * @class AudioSystem
 * @brief Administra AudioEngine, sonidos activos y Preview.
 */
class AudioSystem
{
public:

	AudioSystem();
	~AudioSystem();

	// AudioSystem no puede copiarse.
	AudioSystem(const AudioSystem&) = delete;
	AudioSystem& operator=(const AudioSystem&) = delete;

	/**
	 * @brief Inicializa DirectXTK AudioEngine.
	 * @return true cuando existe un dispositivo de audio.
	 */
	bool init();

	/**
	 * @brief Actualiza el motor de audio.
	 */
	void update();

	void update3D(
		const EU::Vector3& listenerPosition,
		const EU::Vector3& listenerForward,
		const EU::Vector3& listenerUp
	);

	/**
	 * @brief Procesa las solicitudes de Preview del Inspector.
	 */
	void processPreviewRequests(
		const std::vector<EU::TSharedPointer<Actor>>& actors
	);

	/**
	 * @brief Detiene el sonido reproducido como Preview.
	 */
	void stopPreview();

	/**
	 * @brief Reproduce los componentes que tienen Play On Start.
	 */
	void playOnStart(
		const std::vector<EU::TSharedPointer<Actor>>& actors
	);

	/**
	 * @brief Pausa todos los sonidos del modo Play.
	 */
	void pauseAll();

	/**
	 * @brief Reanuda todos los sonidos pausados.
	 */
	void resumeAll();

	/**
	 * @brief Detiene todos los sonidos y también el Preview.
	 */
	void stopAll();

	/**
	 * @brief Destruye todos los recursos de audio.
	 */
	void shutdown();

	/**
	 * @brief Indica si AudioEngine está disponible.
	 */
	bool isReady() const;

private:

	// Oculta las clases de DirectXTK para evitar conflictos
	// entre DirectXMath y XNAMath en los demás archivos.
	struct Impl;

	std::unique_ptr<Impl> m_impl;
};