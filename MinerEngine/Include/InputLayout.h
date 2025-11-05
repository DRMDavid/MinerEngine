/**
 * @file InputLayout.h
 * @brief Define la clase InputLayout, que gestiona el layout de los vértices para el pipeline de gráficos.
 *
 * El InputLayout describe cómo se estructuran los datos de los vértices (posición, color,
 * coordenadas de textura, etc.) y cómo deben ser interpretados por el Vertex Shader.
 */
#pragma once

#include "Prerequisites.h"

 // Declaraciones adelantadas (Forward Declarations) para evitar inclusiones innecesarias
class Device;
class DeviceContext;

/**
 * @class InputLayout
 * @brief Encapsula la gestión de un objeto ID3D11InputLayout de DirectX 11.
 *
 * Esta clase es responsable de la creación, vinculación y liberación del layout
 * de entrada de los vértices que define el formato de los datos del búfer de vértices.
 */
class
	InputLayout {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	InputLayout() = default;

	/**
	 * @brief Destructor por defecto.
	 *
	 * Asegura que se llama a destroy() para liberar el recurso de DirectX si es necesario.
	 */
	~InputLayout() = default;

	/**
	 * @brief Inicializa el objeto InputLayout creando el recurso de DirectX.
	 *
	 * @param device Referencia al objeto Device de DirectX utilizado para crear el layout.
	 * @param Layout Vector de estructuras que describen los elementos del layout (e.g., Position, Color).
	 * @param VertexShaderData Puntero al búfer de datos (blob) del Vertex Shader compilado,
	 *                         necesario para la creación del layout.
	 * @return HRESULT El código de resultado de la operación (S_OK si es exitosa).
	 */
	HRESULT
		init(Device& device,
			           std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
			           ID3DBlob* VertexShaderData);

	/**
	 * @brief Lógica de actualización del layout (generalmente vacía para un layout estático).
	 *
	 * Este método podría usarse para manejar cambios dinámicos si el layout de entrada cambiara.
	 */
	void
		update();

	/**
	 * @brief Vincula el InputLayout al pipeline de entrada del renderizado.
	 *
	 * Establece el layout de entrada actual en el DeviceContext antes de dibujar.
	 * @param deviceContext Referencia al contexto del dispositivo para establecer el recurso.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Limpia y libera el recurso ID3D11InputLayout de DirectX.
	 */
	void
		destroy();

public:
	/// @brief Puntero al objeto nativo ID3D11InputLayout de DirectX.
	ID3D11InputLayout* m_inputLayout = nullptr;
};