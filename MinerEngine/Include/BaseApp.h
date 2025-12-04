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
#include "Model3D.h"
#include "ECS/Actor.h"

/**
 * @class BaseApp
 * @brief Núcleo de ejecución y orquestador del ciclo de vida del motor.
 *
 * Esta clase encapsula la infraestructura fundamental de la aplicación.
 * Es responsable de establecer la ventana del sistema operativo, inicializar
 * el dispositivo gráfico (DirectX), y mantener el bucle principal ("Game Loop")
 * que alterna entre la simulación lógica (Update) y el dibujado (Render).
 */
class
	BaseApp {
public:
	/**
	 * @brief Constructor predeterminado.
	 */
	BaseApp() = default;

	/**
	 * @brief Destructor.
	 * * Asegura la llamada explícita a la rutina de limpieza al salir del ámbito.
	 */
	~BaseApp() { destroy(); }

	/**
	 * @brief Punto de entrada al bucle de mensajes (Message Pump).
	 * * Inicia la ventana y entra en el ciclo infinito de procesamiento hasta
	 * recibir una señal de terminación del SO.
	 * @param hInst Manejador de la instancia de la aplicación (Win32).
	 * @param nCmdShow Estado de visualización inicial de la ventana (minimizada/maximizada).
	 * @return Código de salida del sistema.
	 */
	int
		run(HINSTANCE hInst, int nCmdShow);

	/**
	 * @brief Configuración del entorno gráfico.
	 * * Instancia los subsistemas críticos: Device, SwapChain, Buffers de profundidad
	 * y Vistas de renderizado (RTV/DSV).
	 * @return S_OK si la inicialización de DirectX fue exitosa.
	 */
	HRESULT
		init();

	/**
	 * @brief Paso de simulación (CPU).
	 * * Calcula los cambios de estado del mundo, física, input y animaciones
	 * antes de generar el frame visual.
	 * @param deltaTime Tiempo transcurrido en segundos desde el último frame.
	 */
	void
		update(float deltaTime);

	/**
	 * @brief Paso de presentación (GPU).
	 * * Limpia los buffers de vídeo, emite los comandos de dibujo de la escena
	 * y realiza el intercambio de buffers (Swap) para mostrar la imagen en pantalla.
	 */
	void
		render();

	/**
	 * @brief Desmantelamiento del sistema.
	 * * Libera los recursos COM, texturas, shaders y cierra la ventana del sistema
	 * para evitar fugas de memoria.
	 */
	void
		destroy();

private:
	/**
	 * @brief Callback de procedimientos de ventana (Win32).
	 * * Método estático que intercepta los eventos de hardware (teclado, mouse, cierre)
	 * enviados por Windows y los redirige a la instancia de la aplicación.
	 */
	static LRESULT CALLBACK
		WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Window                              m_window;           ///< Abstracción de la ventana nativa del SO.
	Device                              m_device;           ///< Interfaz virtual del adaptador gráfico.
	DeviceContext                       m_deviceContext;    ///< Generador de comandos de renderizado.
	SwapChain                           m_swapChain;        ///< Gestor de doble búfer (Front/Back Buffer).
	Texture                             m_backBuffer;       ///< Textura destino donde se dibuja el frame actual.
	RenderTargetView                    m_renderTargetView; ///< Vista de recursos para escribir en el BackBuffer.
	Texture                             m_depthStencil;     ///< Textura para información de profundidad y stencil.
	DepthStencilView                    m_depthStencilView; ///< Vista para pruebas de profundidad (Z-Buffering).
	Viewport                            m_viewport;         ///< Configuración del área de rasterización.
	ShaderProgram                       m_shaderProgram;    ///< Gestor de Vertex y Pixel Shaders activos.
	//MeshComponent                       m_mesh;
	//Buffer                              m_vertexBuffer;
	//Buffer                              m_indexBuffer;
	Buffer                              m_cbNeverChanges;   ///< Constant Buffer para datos estáticos (ej. Proyección).
	Buffer                              m_cbChangeOnResize; ///< Constant Buffer actualizado al redimensionar.
	//Buffer                              m_cbChangesEveryFrame;
	Texture                             m_PrintstreamAlbedo;///< Textura de prueba cargada en memoria.
	//SamplerState                        m_samplerState;

	//XMMATRIX                          m_World;
	XMMATRIX                            m_View;             ///< Matriz de Cámara (Vista).
	XMMATRIX                            m_Projection;       ///< Matriz de Proyección (Perspectiva/Ortográfica).
	//XMFLOAT4                          m_vMeshColor;// (0.7f, 0.7f, 0.7f, 1.0f);

	std::vector<EU::TSharedPointer<Actor>> m_actors;        ///< Lista de entidades activas en la escena.
	EU::TSharedPointer<Actor> m_Printstream;                ///< Actor específico para pruebas.


	Model3D* m_model;                                       ///< Puntero al recurso de modelo cargado.


	CBChangeOnResize                    cbChangesOnResize;  ///< Estructura de datos CPU para el buffer de redimensionado.
	CBNeverChanges                      cbNeverChanges;     ///< Estructura de datos CPU para el buffer estático.
	//CBChangesEveryFrame                 cb;
};