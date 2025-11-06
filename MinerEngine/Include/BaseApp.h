#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "ModelLoader.h" 

/**
 * @class BaseApp
 * @brief Clase base para una aplicación de gráficos 3D (probablemente DirectX 11).
 * * Esta clase encapsula la inicialización, el bucle principal y la gestión de los recursos
 * fundamentales del pipeline de renderizado, incluyendo la ventana, el dispositivo gráfico,
 * la cadena de intercambio y los recursos de escena (shaders, geometría, texturas).
 */
class
	BaseApp {
public:
	/**
	 * @brief Constructor de la aplicación.
	 * * Inicializa los miembros de la clase.
	 * @param hInst Handle de la instancia de la aplicación.
	 * @param nCmdShow Indicador de cómo se debe mostrar la ventana.
	 */
	BaseApp(HINSTANCE hInst, int nCmdShow);
	/**
	 * @brief Destructor de la aplicación.
	 * * Llama a la función destroy() para liberar los recursos.
	 */
	~BaseApp() { destroy(); }
	/**
	 * @brief Ejecuta el bucle principal de la aplicación.
	 * * Contiene el bucle de mensajes de Windows y llama a los métodos update y render.
	 * @param hInst Handle de la instancia de la aplicación.
	 * @param nCmdShow Indicador de cómo se debe mostrar la ventana.
	 * @return Código de salida de la aplicación.
	 */
	int
		run(HINSTANCE hInst, int nCmdShow);
	/**
	 * @brief Inicializa los componentes de DirectX y los recursos de la escena.
	 * * Se encarga de crear la ventana, el dispositivo, la cadena de intercambio,
	 * los buffers de renderizado, shaders, buffers constantes y cargar el modelo.
	 * @return HRESULT Código de resultado de la operación (S_OK si es exitoso).
	 */
	HRESULT
		init();
	/**
	 * @brief Actualiza la lógica del juego o de la escena.
	 * * Maneja la lógica de movimiento, animación, y actualización de las matrices de transformación.
	 * @param deltaTime El tiempo transcurrido desde el último frame (en segundos).
	 */
	void
		update(float deltaTime);
	/**
	 * @brief Dibuja la escena en el back buffer.
	 * * Establece el estado de renderizado, realiza las llamadas de dibujo y presenta el resultado.
	 */
	void
		render();
	/**
	 * @brief Libera todos los recursos de DirectX y la ventana.
	 * * Debe ser llamada antes de que la aplicación finalice.
	 */
	void
		destroy();

private:
	/**
	 * @brief Procedimiento de ventana estático para manejar los mensajes del SO.
	 * @param hWnd Handle de la ventana.
	 * @param message Mensaje de Windows.
	 * @param wParam Parámetro del mensaje.
	 * @param lParam Parámetro del mensaje.
	 * @return Resultado del procesamiento del mensaje.
	 */
	static LRESULT CALLBACK
		WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	// --- Componentes Core del Renderizado ---
	Window                              m_window;
	Device															m_device;
	DeviceContext												m_deviceContext;
	SwapChain                           m_swapChain;
	Texture                             m_backBuffer;
	RenderTargetView									  m_renderTargetView;
	Texture                             m_depthStencil;
	DepthStencilView									  m_depthStencilView;
	Viewport                            m_viewport;

	// --- Recursos de Escena y Gráficos ---
	ShaderProgram												m_shaderProgram;
	MeshComponent												m_mesh;
	Buffer															m_vertexBuffer;
	Buffer															m_indexBuffer;
	Buffer															m_cbNeverChanges;
	Buffer															m_cbChangeOnResize;
	Buffer															m_cbChangesEveryFrame;
	Texture 														m_textureCube;
	SamplerState												m_samplerState;
  ModelLoader                         m_modelLoader;
  Texture                             m_diffuseTexture;
  Texture                             m_normalTexture;

	// --- Matrices y Constantes de Escena ---
	XMMATRIX                            m_World;
	XMMATRIX                            m_View;
	XMMATRIX                            m_Projection;
	XMFLOAT4                            m_vMeshColor;// (0.7f, 0.7f, 0.7f, 1.0f);

	// --- Estructuras de Datos de Buffers Constantes ---
  CBChangeOnResize										cbChangesOnResize;
	CBNeverChanges											cbNeverChanges;
	CBChangesEveryFrame									cb;
};