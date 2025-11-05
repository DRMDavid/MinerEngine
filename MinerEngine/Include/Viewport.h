/**
 * @file Viewport.h
 * @brief Define la clase Viewport, que gestiona el estado del viewport para el pipeline de renderizado.
 *
 * Un viewport define qué porción del Render Target se mapea al espacio de coordenadas de clip
 * normalizado (NDC) después de la etapa de transformación de la perspectiva.
 */
#pragma once

#include "Prerequisites.h"

 // Declaraciones adelantadas (Forward Declarations)
class
	Window;

class
	DeviceContext;

/**
 * @class Viewport
 * @brief Encapsula la estructura D3D11_VIEWPORT y proporciona métodos para su inicialización y vinculación.
 *
 * Define la región rectangular de la superficie de destino a la que se renderizan los primitivos.
 */
class
	Viewport {
public:

	/**
	 * @brief Constructor por defecto.
	 */
	Viewport() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Viewport() = default;

	/**
	 * @brief Inicializa el viewport basándose en las dimensiones de una ventana existente.
	 *
	 * Utiliza el ancho y alto del cliente de la ventana para configurar el viewport.
	 *
	 * @param window Referencia constante al objeto Window cuyas dimensiones se utilizarán.
	 * @return HRESULT Siempre devuelve S_OK ya que es una operación de configuración local.
	 */
	HRESULT
		init(const Window& window);

	/**
	 * @brief Inicializa el viewport con dimensiones específicas.
	 *
	 * Configura el ancho, el alto, el rango de profundidad (MinDepth/MaxDepth) y la esquina superior izquierda (TopLeftX/Y).
	 *
	 * @param width El ancho del viewport.
	 * @param height La altura del viewport.
	 * @return HRESULT Siempre devuelve S_OK ya que es una operación de configuración local.
	 */
	HRESULT
		init(unsigned int width, unsigned int height);

	/**
	 * @brief Lógica de actualización (generalmente vacía para un viewport estático).
	 *
	 * Podría usarse para manejar cambios de tamaño dinámicos si no se usa el método init(Window&).
	 */
	void
		update();

	/**
	 * @brief Vincula el viewport al pipeline de renderizado.
	 *
	 * Llama a RSSetViewports en el DeviceContext para establecer el viewport activo.
	 * @param deviceContext Referencia al contexto del dispositivo para establecer el recurso.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Limpia y libera recursos.
	 *
	 * Implementación vacía ya que esta clase solo contiene una estructura de datos local (D3D11_VIEWPORT)
	 * y no gestiona recursos de DirectX que deban ser liberados explícitamente.
	 */
	void
		destroy() {}

public:
	/// @brief Estructura nativa de DirectX que contiene la definición del viewport.
	D3D11_VIEWPORT m_viewport;
};