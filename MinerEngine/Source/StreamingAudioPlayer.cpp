#include "StreamingAudioPlayer.h"
#include "Prerequisites.h"

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfmediaengine.h>
#include <mferror.h>
#include <wrl/client.h>
#include <Shlwapi.h>

#include <algorithm>
#include <atomic>
#include <new>
#include <string>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using Microsoft::WRL::ComPtr;

//============================================================
// CALLBACK DE MEDIA FOUNDATION
//============================================================

class MediaEngineNotify final :
	public IMFMediaEngineNotify
{
public:

	MediaEngineNotify()
		: m_referenceCount(1)
	{
	}

	STDMETHODIMP QueryInterface(
		REFIID interfaceId,
		void** object) override
	{
		if (object == nullptr)
		{
			return E_POINTER;
		}

		*object = nullptr;

		if (interfaceId == IID_IUnknown ||
			interfaceId == __uuidof(IMFMediaEngineNotify))
		{
			*object =
				static_cast<IMFMediaEngineNotify*>(this);

			AddRef();

			return S_OK;
		}

		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef() override
	{
		return ++m_referenceCount;
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		const ULONG referenceCount =
			--m_referenceCount;

		if (referenceCount == 0)
		{
			delete this;
		}

		return referenceCount;
	}

	STDMETHODIMP EventNotify(
		DWORD eventId,
		DWORD_PTR parameter1,
		DWORD parameter2) override
	{
		(void)parameter1;
		(void)parameter2;

		m_lastEvent.store(eventId);

		return S_OK;
	}

	DWORD getLastEvent() const
	{
		return m_lastEvent.load();
	}

private:

	std::atomic<ULONG> m_referenceCount;
	std::atomic<DWORD> m_lastEvent{ 0 };
};

//============================================================
// IMPLEMENTACION PRIVADA
//============================================================

struct StreamingAudioPlayer::Impl
{
	ComPtr<IMFMediaEngineClassFactory> factory;
	ComPtr<IMFMediaEngine> mediaEngine;
	ComPtr<IMFAttributes> attributes;
	ComPtr<MediaEngineNotify> notify;

	bool mediaFoundationStarted = false;
	bool comInitializedHere = false;
	bool ready = false;
};

//============================================================
// CONSTRUCTOR / DESTRUCTOR
//============================================================

StreamingAudioPlayer::StreamingAudioPlayer()
	: m_impl(std::make_unique<Impl>())
{
}

StreamingAudioPlayer::~StreamingAudioPlayer()
{
	shutdown();
}

//============================================================
// INICIALIZACION
//============================================================

bool
StreamingAudioPlayer::init()
{
	if (!m_impl)
	{
		m_impl =
			std::make_unique<Impl>();
	}

	if (m_impl->ready)
	{
		return true;
	}

	HRESULT result =
		CoInitializeEx(
			nullptr,
			COINIT_MULTITHREADED
		);

	if (result == S_OK ||
		result == S_FALSE)
	{
		m_impl->comInitializedHere = true;
	}
	else if (result != RPC_E_CHANGED_MODE)
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudo inicializar COM"
		);

		return false;
	}

	result =
		MFStartup(
			MF_VERSION,
			MFSTARTUP_FULL
		);

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudo inicializar Media Foundation"
		);

		if (m_impl->comInitializedHere)
		{
			CoUninitialize();
			m_impl->comInitializedHere = false;
		}

		return false;
	}

	m_impl->mediaFoundationStarted = true;

	result =
		MFCreateAttributes(
			m_impl->attributes.GetAddressOf(),
			1
		);

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudieron crear los atributos"
		);

		shutdown();
		return false;
	}

	m_impl->notify.Attach(
		new (std::nothrow) MediaEngineNotify()
	);

	if (!m_impl->notify)
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudo crear el callback"
		);

		shutdown();
		return false;
	}

	result =
		m_impl->attributes->SetUnknown(
			MF_MEDIA_ENGINE_CALLBACK,
			m_impl->notify.Get()
		);

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudo registrar el callback"
		);

		shutdown();
		return false;
	}

	result =
		CoCreateInstance(
			CLSID_MFMediaEngineClassFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(
				m_impl->factory.GetAddressOf()
			)
		);

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudo crear MediaEngineClassFactory"
		);

		shutdown();
		return false;
	}

	result =
		m_impl->factory->CreateInstance(
			MF_MEDIA_ENGINE_AUDIOONLY,
			m_impl->attributes.Get(),
			m_impl->mediaEngine.GetAddressOf()
		);

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"init",
			"No se pudo crear IMFMediaEngine"
		);

		shutdown();
		return false;
	}

	m_impl->ready = true;

	MESSAGE(
		"StreamingAudioPlayer",
		"init",
		"Reproductor de streaming inicializado"
	);

	return true;
}

//============================================================
// CARGAR ARCHIVO
//============================================================

bool
StreamingAudioPlayer::load(
	const std::wstring& filePath)
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		ERROR(
			"StreamingAudioPlayer",
			"load",
			"El reproductor no esta inicializado"
		);

		return false;
	}

	if (filePath.empty())
	{
		ERROR(
			"StreamingAudioPlayer",
			"load",
			"La ruta del archivo esta vacia"
		);

		return false;
	}

	wchar_t absolutePath[MAX_PATH] = {};

	const DWORD pathLength =
		GetFullPathNameW(
			filePath.c_str(),
			MAX_PATH,
			absolutePath,
			nullptr
		);

	if (pathLength == 0 ||
		pathLength >= MAX_PATH)
	{
		ERROR(
			"StreamingAudioPlayer",
			"load",
			"No se pudo obtener la ruta absoluta"
		);

		return false;
	}

	wchar_t mediaUrl[2048] = {};
	DWORD mediaUrlLength = 2048;

	const HRESULT urlResult =
		UrlCreateFromPathW(
			absolutePath,
			mediaUrl,
			&mediaUrlLength,
			0
		);

	if (FAILED(urlResult))
	{
		ERROR(
			"StreamingAudioPlayer",
			"load",
			"No se pudo convertir la ruta a URL"
		);

		return false;
	}

	BSTR source =
		SysAllocString(mediaUrl);

	if (source == nullptr)
	{
		ERROR(
			"StreamingAudioPlayer",
			"load",
			"No se pudo crear la ruta para Media Foundation"
		);

		return false;
	}

	const HRESULT result =
		m_impl->mediaEngine->SetSource(
			source
		);

	SysFreeString(source);

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"load",
			"Media Foundation no pudo cargar el archivo"
		);

		return false;
	}

	m_impl->mediaEngine->Load();

	MESSAGE(
		"StreamingAudioPlayer",
		"load",
		"Archivo de streaming cargado"
	);

	return true;
}

//============================================================
// PLAY
//============================================================

void
StreamingAudioPlayer::play()
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		return;
	}

	const HRESULT result =
		m_impl->mediaEngine->Play();

	if (FAILED(result))
	{
		ERROR(
			"StreamingAudioPlayer",
			"play",
			"No se pudo reproducir el archivo"
		);
	}
}

//============================================================
// PAUSE
//============================================================

void
StreamingAudioPlayer::pause()
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		return;
	}

	m_impl->mediaEngine->Pause();
}

//============================================================
// RESUME
//============================================================

void
StreamingAudioPlayer::resume()
{
	play();
}

//============================================================
// STOP
//============================================================

void
StreamingAudioPlayer::stop()
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		return;
	}

	m_impl->mediaEngine->Pause();
	m_impl->mediaEngine->SetCurrentTime(0.0);
}

//============================================================
// VOLUME
//============================================================

void
StreamingAudioPlayer::setVolume(float volume)
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		return;
	}

	const float clampedVolume =
		(std::max)(0.0f, (std::min)(volume, 1.0f));

	m_impl->mediaEngine->SetVolume(
		static_cast<double>(clampedVolume)
	);
}

//============================================================
// LOOP
//============================================================

void
StreamingAudioPlayer::setLoop(bool loop)
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		return;
	}

	m_impl->mediaEngine->SetLoop(
		loop ? TRUE : FALSE
	);
}

//============================================================
// SHUTDOWN
//============================================================

void
StreamingAudioPlayer::shutdown()
{
	if (!m_impl)
	{
		return;
	}

	if (m_impl->mediaEngine)
	{
		m_impl->mediaEngine->Pause();
		m_impl->mediaEngine->Shutdown();
	}

	m_impl->mediaEngine.Reset();
	m_impl->factory.Reset();
	m_impl->attributes.Reset();
	m_impl->notify.Reset();

	m_impl->ready = false;

	if (m_impl->mediaFoundationStarted)
	{
		MFShutdown();
		m_impl->mediaFoundationStarted = false;
	}

	if (m_impl->comInitializedHere)
	{
		CoUninitialize();
		m_impl->comInitializedHere = false;
	}
}

//============================================================
// ESTADO
//============================================================

bool
StreamingAudioPlayer::isReady() const
{
	return
		m_impl &&
		m_impl->ready &&
		m_impl->mediaEngine;
}

bool
StreamingAudioPlayer::isPlaying() const
{
	if (!m_impl ||
		!m_impl->ready ||
		!m_impl->mediaEngine)
	{
		return false;
	}

	return
		m_impl->mediaEngine->IsPaused() == FALSE &&
		m_impl->mediaEngine->IsEnded() == FALSE;
}