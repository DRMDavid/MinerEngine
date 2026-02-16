# ⛏️ MinerEngine – Motor Gráfico & ECS (C++)

<p align="center">
  <img src="https://img.shields.io/badge/Direct3D-11-1155BA?style=for-the-badge&logo=windows&logoColor=white" alt="D3D11"/>
  <img src="https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/Win32-API-1f6feb?style=for-the-badge" alt="Win32 API"/>
  <img src="https://img.shields.io/badge/ECS-Architecture-orange?style=for-the-badge" alt="ECS"/>
</p>

---

## 📘 Resumen del Proyecto
**MinerEngine** es un ecosistema de desarrollo 3D de alto rendimiento construido sobre **C++** y **DirectX 11**. El motor utiliza una arquitectura modular basada en un sistema de **Entidad-Componente (ECS)** y un **Grafo de Escena** avanzado para la gestión de jerarquías espaciales complejas.

Esta versión representa una evolución significativa, integrando herramientas de edición en tiempo real, navegación profesional y un pipeline de renderizado optimizado para entornos inmersivos.

---

## ✨ Características Técnicas Detalladas

### 🌳 Gestión de Escena y Jerarquías (Scene Graph)
MinerEngine implementa un árbol jerárquico que organiza el mundo virtual de forma lógica, permitiendo relaciones complejas de parentesco entre actores.
* **Cálculo de Matrices en Cascada:** Las transformaciones se propagan de forma descendente. Cada nodo hijo calcula su posición global multiplicando su matriz local por la de su padre ($M_{world} = M_{local} \times M_{parentWorld}$), garantizando coherencia espacial.
* **Validación de Integridad:** Implementación de algoritmos de detección de ancestros (`isAncestor`) para blindar el motor contra dependencias circulares que podrían derivar en un *Stack Overflow*.
* **Arquitectura Orientada a Datos:** El grafo gestiona automáticamente el ciclo de vida de los componentes `Transform` y `Hierarchy`, asegurando que toda entidad registrada sea válida para el pipeline de renderizado.

### 🎥 Sistema de Navegación Profesional (Basis Vectors)
Se ha sustituido el sistema de ángulos de Euler por uno basado en **vectores base**, eliminando por completo el riesgo de **Gimbal Lock** y permitiendo una rotación fluida en cualquier eje.
* **Control por Vectores Ortonormales:** La cámara se orienta mediante los ejes `Right`, `Up` y `Forward`. Al movernos, desplazamos la posición escalando estos vectores, logrando un movimiento natural y preciso.
* **Corrección de Deriva Matemática:** Debido a la pérdida de precisión inherente a los cálculos de punto flotante, el motor ejecuta una **re-ortonormalización** constante mediante productos cruz. Esto asegura que los ejes de la cámara se mantengan siempre perpendiculares entre sí, evitando deformaciones visuales tras sesiones prolongadas.

### 🛠️ Editor y Herramientas de Autoría (ImGui + ImGuizmo)
* **Manipulación Directa:** Integración de **ImGuizmo** para operar matrices de transformación mediante interacción directa en el viewport. El sistema traduce coordenadas de pantalla 2D a operaciones de álgebra lineal 3D en tiempo real.
* **Sincronización de Coordenadas:** Implementación de lógica de conversión para unificar el uso de **grados** en la interfaz de usuario con los **radianes** requeridos por las funciones trigonométricas de DirectX 11.
* **Inspector de Propiedades Avanzado:** Panel de edición detallado con widgets personalizados (`vec3Control`) que utilizan codificación de color estándar (X:Rojo, Y:Verde, Z:Azul) para facilitar el ajuste fino de la escena.

### 🌌 Skybox y Pipeline de Iluminación Global
* **Gestión de Cubemaps:** Soporte para recursos de textura de 6 caras (`TextureCube`). El cargador de texturas valida la resolución y formato de cada cara para asegurar una carga limpia en la VRAM.
* **Optimización de Fondo:** El Skybox se renderiza utilizando un estado de profundidad específico, permitiendo que el entorno envuelva la escena de forma infinita sin interferir con la geometría cercana de los actores.

---

## 🏗️ Arquitectura del Sistema

| Módulo | Implementación Técnica | Objetivo Arquitectónico |
|---|---|---|
| **Memoria** | Punteros inteligentes personalizados (`TSharedPointer`) | Control total del ciclo de vida de objetos sin la latencia de la STL. |
| **Matemáticas** | Librería propia `EngineMath.h` | Operaciones de matrices y cuaterniones optimizadas para hardware x64. |
| **Gráficos** | Pipeline de DirectX 11 | Gestión eficiente de `ConstantBuffers`, `InputLayouts` y estados de renderizado. |
| **Recursos** | Singleton `ResourceManager` | Caché centralizado para evitar la duplicación de mallas y texturas en memoria de video. |



---

## 🖥️ Tecnologías Integradas

* **Direct3D 11:** API principal para el renderizado de gráficos de bajo nivel.
* **Win32 API:** Gestión nativa de ventanas, mensajes de sistema e input de periféricos.
* **stb_image:** Librería integrada para la decodificación de texturas multiformato (.png, .jpg).
* **ImGui & ImGuizmo:** Herramientas de depuración y manipulación de matrices para el entorno de desarrollo.

---

