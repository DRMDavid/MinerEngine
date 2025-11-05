#pragma once

#include "Prerequisites.h"
#include "InputLayout.h"

// Declaraciones adelantadas (Forward Declarations)
class Device;
class DeviceContext;

/**
 * @class ShaderProgram
 * @brief Clase que gestiona los Shaders de Vértice y Píxel y el Layout de Entrada de Vértices asociado.
 *
 * Contiene métodos para compilar shaders a partir de archivos, crear los recursos nativos
 * de DirectX (ID3D11VertexShader, ID3D11PixelShader) y vincularlos al contexto del dispositivo.
 */
class
	ShaderProgram {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	ShaderProgram() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~ShaderProgram() = default;

	/**
	 * @brief Inicializa el programa de shaders, compilando shaders y creando el InputLayout.
	 *
	 * Este método maneja la compilación inicial de los shaders desde un archivo y la creación
	 * del layout de entrada de vértices basado en la descripción proporcionada.
	 *
	 * @param device Referencia al objeto Device de DirectX.
	 * @param fileName Ruta del archivo que contiene el código fuente del shader.
	 * @param Layout Vector de estructuras que describen los elementos del layout de vértices.
	 * @return HRESULT El código de resultado de la operación (S_OK si es exitosa).
	 */
	HRESULT
		init(Device& device,
			const std::string& fileName,
			std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

	/**
	 * @brief Lógica de actualización (generalmente vacía para shaders estáticos).
	 */
	void
		update();

	/**
	 * @brief Vincula todos los shaders (VS y PS) y el InputLayout al pipeline.
	 *
	 * @param deviceContext Referencia al contexto del dispositivo para establecer los recursos.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Vincula un tipo específico de shader al pipeline de renderizado.
	 *
	 * @param deviceContext Referencia al contexto del dispositivo para establecer el recurso.
	 * @param type Especifica el tipo de shader a vincular (e.g., VertexShader, PixelShader).
	 */
	void
		render(DeviceContext& deviceContext, ShaderType type);

	/**
	 * @brief Limpia y libera todos los recursos de shaders y los búferes de datos compilados.
	 */
	void
		destroy();

	/**
	 * @brief Crea el InputLayout de vértices usando los datos del Vertex Shader compilado.
	 *
	 * @param device Referencia al objeto Device de DirectX.
	 * @param Layout Vector de estructuras que describen los elementos del layout de vértices.
	 * @return HRESULT El código de resultado de la creación del InputLayout.
	 */
	HRESULT
		CreateInputLayout(Device& device,
			std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

	/**
	 * @brief Crea un recurso de shader nativo de DirectX a partir de datos ya compilados.
	 *
	 * @param device Referencia al objeto Device de DirectX.
	 * @param type Especifica el tipo de shader a crear (VS o PS).
	 * @return HRESULT El código de resultado de la creación del shader.
	 */
	HRESULT
		CreateShader(Device& device, ShaderType type);

	/**
	 * @brief Compila un shader desde un archivo y luego crea el recurso de DirectX.
	 *
	 * @param device Referencia al objeto Device de DirectX.
	 * @param type Especifica el tipo de shader a compilar y crear.
	 * @param fileName Ruta del archivo que contiene el código fuente del shader.
	 * @return HRESULT El código de resultado de la operación.
	 */
	HRESULT
		CreateShader(Device& device, ShaderType type, const std::string& fileName);

	/**
	 * @brief Compila el código fuente del shader desde un archivo a un buffer de bytecodes.
	 *
	 * Esta es una función auxiliar para compilar archivos HLSL.
	 *
	 * @param szFileName Nombre del archivo del shader (HLSL).
	 * @param szEntryPoint Nombre de la función principal del shader (e.g., "VSMain", "PSMain").
	 * @param szShaderModel Modelo del shader a usar (e.g., "vs_4_0", "ps_4_0").
	 * @param ppBlobOut Puntero de salida para el búfer de datos compilado (ID3DBlob).
	 * @return HRESULT El código de resultado de la compilación.
	 */
	HRESULT
		CompileShaderFromFile(char* szFileName,
			LPCSTR szEntryPoint,
			LPCSTR szShaderModel,
			ID3DBlob** ppBlobOut);

public:
	/// @brief Puntero al objeto nativo ID3D11VertexShader de DirectX.
	ID3D11VertexShader* m_VertexShader = nullptr;

	/// @brief Puntero al objeto nativo ID3D11PixelShader de DirectX.
	ID3D11PixelShader* m_PixelShader = nullptr;

	/// @brief Objeto que gestiona el layout de entrada de los vértices asociado a este programa.
	InputLayout m_inputLayout;

private:
	/// @brief Nombre del archivo desde el que se cargaron los shaders.
	std::string m_shaderFileName;

	/// @brief Búfer de datos (blob) del Vertex Shader compilado. Necesario para crear el InputLayout.
	ID3DBlob* m_vertexShaderData = nullptr;

	/// @brief Búfer de datos (blob) del Pixel Shader compilado.
	ID3DBlob* m_pixelShaderData = nullptr;
};