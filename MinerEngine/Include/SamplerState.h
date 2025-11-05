#pragma once

#include "Prerequisites.h"

// Declaraciones adelantadas (Forward Declarations)
class Device;
class DeviceContext;

/**
 * @class SamplerState
 * @brief Clase que gestiona el estado del muestreador de texturas (Sampler State).
 *
 * Responsable de la creación, vinculación y liberación del objeto ID3D11SamplerState.
 */
class
	SamplerState {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	SamplerState() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~SamplerState() = default;

	/**
	 * @brief Inicializa el objeto SamplerState creando el recurso ID3D11SamplerState en el dispositivo.
	 *
	 * Configura las propiedades de muestreo (e.g., filtrado, modo de borde/direccionamiento de UVs).
	 * @param device Referencia al objeto Device de DirectX para la creación del estado.
	 * @return HRESULT El código de resultado de la operación (S_OK si es exitosa).
	 */
	HRESULT
		init(Device& device);

	/**
	 * @brief Lógica de actualización (generalmente vacía para un estado estático).
	 *
	 * Se podría usar si las propiedades de muestreo cambiaran dinámicamente.
	 */
	void
		update();

	/**
	 * @brief Vincula el SamplerState a los recursos de shader (Generalmente Pixel Shader o Compute Shader).
	 *
	 * @param deviceContext Referencia al contexto del dispositivo para establecer el recurso.
	 * @param StartSlot Índice del slot inicial en el array de muestreadores donde se vinculará este muestreador.
	 * @param NumSamplers Número de muestreadores a vincular (normalmente 1).
	 */
	void
		render(DeviceContext& deviceContext,
			unsigned int StartSlot,
			unsigned int NumSamplers);

	/**
	 * @brief Limpia y libera el recurso ID3D11SamplerState de DirectX.
	 */
	void
		destroy();

public:
	/// @brief Puntero al objeto nativo ID3D11SamplerState de DirectX.
	ID3D11SamplerState* m_sampler = nullptr;
};