/**
 * @file MeshComponent.h
 * @brief Define la clase MeshComponent, que encapsula los datos de geometría (malla) de un objeto 3D.
 *
 * Esta clase almacena los vértices e índices de un modelo y define los métodos para
 * inicializar, actualizar y renderizar esa geometría.
 */
#pragma once

#include "Prerequisites.h"

class DeviceContext;

/**
 * @class MeshComponent
 * @brief Componente que contiene la geometría (malla) de un objeto para ser renderizado.
 *
 * Almacena los búferes de vértices y los índices, y proporciona la interfaz para
 * interactuar con ellos en el bucle de renderizado.
 */
class MeshComponent {
public:
	/**
	 * @brief Constructor por defecto.
	 *
	 * Inicializa el número de vértices y el número de índices a cero.
	 */
	MeshComponent() : m_numVertex(0), m_numIndex(0) {}

	/**
	 * @brief Destructor virtual por defecto.
	 *
	 * Asegura la correcta limpieza si la clase es utilizada como clase base.
	 */
	virtual
		~MeshComponent() = default;

	/**
	 * @brief Inicializa el componente de malla.
	 *
	 * Este método podría encargarse de cargar la geometría del disco o de inicializar
	 * los búferes de vértices e índices en la memoria de la GPU (aunque la lógica de
	 * búferes a menudo se maneja en otras clases como 'Buffer').
	 */
	void
		init();

	/**
	 * @brief Actualiza la lógica de la malla (e.g., animación o deformación si aplica).
	 *
	 * @param deltaTime El tiempo transcurrido desde el último fotograma, en segundos.
	 */
	void
		update(float deltaTime);

	/**
	 * @brief Prepara la malla para el dibujado.
	 *
	 * Vincula los búferes de vértices e índices al pipeline de entrada del renderizado.
	 * @param deviceContext Referencia al contexto del dispositivo para establecer los recursos.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Limpia y libera los recursos asociados a la malla.
	 */
	void
		destroy();

public:
	/// @brief Nombre identificador de la malla.
	std::string m_name;

	/// @brief Vector de estructuras que contienen los datos de los vértices (e.g., posición, normal, UV).
	std::vector<SimpleVertex> m_vertex;

	/// @brief Vector de índices que definen la conectividad de los vértices para formar triángulos.
	std::vector<unsigned int> m_index;

	/// @brief Número total de vértices en el búfer de vértices.
	int m_numVertex;

	/// @brief Número total de índices en el búfer de índices.
	int m_numIndex;
};