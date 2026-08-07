#pragma once

#include "Prerequisites.h"

#include <memory>
#include <vector>

class Actor;

/**
 * @class AudioSystem
 * @brief Administra AudioEngine y las fuentes de audio activas.
 */
class AudioSystem
{
public:

	AudioSystem();
	~AudioSystem();

	AudioSystem(const AudioSystem&) = delete;
	AudioSystem& operator=(const AudioSystem&) = delete;

	bool init();

	void update();

	void playOnStart(
		const std::vector<EU::TSharedPointer<Actor>>& actors
	);

	void pauseAll();

	void resumeAll();

	void stopAll();

	void shutdown();

	bool isReady() const;

private:

	struct Impl;
	std::unique_ptr<Impl> m_impl;
};