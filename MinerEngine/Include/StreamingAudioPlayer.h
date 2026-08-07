#pragma once

#include <memory>
#include <string>

/**
 * @class StreamingAudioPlayer
 * @brief Reproduce MP3 y archivos largos usando Media Foundation.
 */
class StreamingAudioPlayer
{
public:

	StreamingAudioPlayer();
	~StreamingAudioPlayer();

	StreamingAudioPlayer(
		const StreamingAudioPlayer&) = delete;

	StreamingAudioPlayer& operator=(
		const StreamingAudioPlayer&) = delete;

	/**
	 * @brief Inicializa COM y Media Foundation.
	 */
	bool init();

	/**
	 * @brief Carga un archivo MP3, WAV u otro formato compatible.
	 */
	bool load(
		const std::wstring& filePath
	);

	/**
	 * @brief Inicia o reanuda la reproducción.
	 */
	void play();

	/**
	 * @brief Pausa la reproducción.
	 */
	void pause();

	/**
	 * @brief Reanuda un sonido pausado.
	 */
	void resume();

	/**
	 * @brief Detiene el sonido y vuelve al inicio.
	 */
	void stop();

	/**
	 * @brief Establece el volumen entre 0 y 1.
	 */
	void setVolume(float volume);

	/**
	 * @brief Activa o desactiva la reproducción en loop.
	 */
	void setLoop(bool loop);

	/**
	 * @brief Libera Media Foundation y sus recursos.
	 */
	void shutdown();

	bool isReady() const;
	bool isPlaying() const;

private:

	struct Impl;
	std::unique_ptr<Impl> m_impl;
};