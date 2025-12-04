<p align="center">
  <img src="https://img.shields.io/badge/Direct3D-11-1155BA?style=for-the-badge&logo=windows&logoColor=white" alt="D3D11"/>
  <img src="https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/Win32-API-1f6feb?style=for-the-badge" alt="Win32 API"/>
  <img src="https://img.shields.io/badge/ECS-Architecture-orange?style=for-the-badge" alt="ECS"/>
</p>

<h1 align="center">⛏️MinerEngine – Motor Gráfico & ECS (C++)</h1>



---

## 📘 Resumen

**⛏️MinerEngine** es un motor gráfico y de videojuegos desarrollado en C++ y **Direct3D 11**. 
Esta versión implementa una arquitectura **Entity-Component-System (ECS)**, permitiendo la gestión modular de entidades (`Actors`). Cuenta con su propia biblioteca de utilidades (Math & Memory), integración de **ImGui** para herramientas de depuración, un **ResourceManager** robusto y carga de modelos 3D complejos vía **FBX SDK**.

---

## 🧭 Índice

- [📘 Resumen](#-resumen)
- [✨ Nuevas Características (ECS)](#-nuevas-características-ecs)
- [🏗️ Arquitectura del Motor](#️-arquitectura-del-motor)
  - [Core & Utilities](#core--utilities)
  - [Sistema ECS](#sistema-ecs)
  - [Gráficos & Recursos](#gráficos--recursos)
- [🖥️ Tecnologías Integradas](#️-tecnologías-integradas)
- [🚀 Flujo de Ejecución](#-flujo-de-ejecución)
- [🧪 Requisitos / Ejecución](#-requisitos--ejecución)

---

## ✨ Nuevas Características (ECS)

El motor ha evolucionado de un renderizador básico a una arquitectura de componentes completa:

| Característica | Descripción |
|---|---|
| **Arquitectura ECS** | Implementación de `Entity`, `Component` y `Actor` para desacoplar lógica y datos. |
| **Custom Memory** | Gestión de memoria propia con `TSharedPointer`, `TWeakPointer`, `TUniquePtr` y `TStaticPtr`. |
| **Custom Containers** | Estructuras de datos optimizadas propias: `TArray`, `TMap`, `TSet`, `TPair`. |
| **Math Library** | Librería matemática: `Vector2/3/4`, `Matrix3x3/4x4`, `Quaternion`. |
| **Model Loader** | Carga de modelos 3D complejos (mallas, texturas) utilizando **FBX SDK**. |
| **Resource Manager** | Sistema centralizado para gestionar la vida útil de recursos (Texturas, Shaders, Modelos). |
| **ImGui Integration** | Interfaz gráfica inmediata para depuración y visualización de datos en tiempo real. |

---

## 🏗️ Arquitectura del Motor

### Core & Utilities
El núcleo del motor evita el uso excesivo de la STL estándar en favor de implementaciones personalizadas para mayor control de memoria y rendimiento.

* **Memory:** Punteros inteligentes (`TSharedPointer`, etc.) para el manejo automático de referencias.
* **Structures:** Contenedores dinámicos como `TArray` y diccionarios como `TMap`.
* **Math:** `EngineMath.h` y clases de álgebra lineal para transformaciones 3D.

### Sistema ECS
La lógica del juego se estructura mediante composición:

| Clase | Responsabilidad | Archivo |
|---|---|---|
| **Actor** | Entidad base que existe en el mundo. Contiene una lista de componentes. | `Actor.h` |
| **Component** | Clase base para comportamientos. Se adjunta a los actores. | `Component.h` |
| **Transform** | Componente vital que define posición, rotación y escala (`Vector3`, `Quaternion`). | `Transform.h` |
| **MeshComponent** | Componente encargado de enlazar la geometría (Model3D) con el Actor para ser renderizada. | `MeshComponent.h` |

### Gráficos & Recursos

| Sistema | Descripción |
|---|---|
| **ResourceManager** | Singleton que carga y cachea recursos (`IResource`) para evitar duplicidad en memoria. |
| **ModelLoader** | Parsea archivos `.fbx` y extrae vértices, índices y coordenadas UV. |
| **Model3D** | Representación en memoria de un objeto 3D listo para ser dibujado. |
| **Renderer** | Pipeline D3D11 gestionando `SwapChain`, `RenderTargetView` y `DepthStencilView`. |

---

## 🖥️ Tecnologías Integradas

| Tech / Lib | Uso |
|---|---|
| **Direct3D 11** | API Gráfica principal. |
| **Win32 API** | Creación de ventana y manejo de inputs (WndProc). |
| **FBX SDK** | Carga de assets 3D formato industrial (.fbx). |
| **ImGui** | GUI para herramientas de desarrollo (Docking, Inspection). |
| **STB Image** | Carga de texturas (integrado en el loader). |

---

## 🚀 Flujo de Ejecución

1.  **Inicialización (`MinerEngine.cpp`):**
    * Se crea la `Window` y el `Device` (D3D11).
    * Se inicializa **ImGui** (Contextos Win32 y DX11).
    * El **ResourceManager** carga shaders y modelos iniciales.
2.  **Bucle de Juego (Game Loop):**
    * **Input:** Se procesan mensajes de Windows.
    * **Update:** Se recorren los `Actors` y se actualizan sus `Components` (lógica, transformaciones).
    * **Render:**
        * Limpieza de buffers (RTV/DSV).
        * Renderizado de geometría (MeshComponents) usando el pipeline configurado.
        * Renderizado de la interfaz **ImGui** (sobreimpreso).
        * `SwapChain::Present()`.
3.  **Shutdown:**
    * Limpieza de memoria mediante los punteros inteligentes propios y liberación de COM Objects.

---

## 🧪 Requisitos / Ejecución

| Ítem | Detalle |
|---|---|
| IDE | Visual Studio 2019/2022 |
| SDKs Requeridos | **DirectX SDK**, **FBX SDK** (debe estar linkeado en el proyecto) |
| Configuración | Debug / Release (x64 recomendado) |

**Pasos de compilación:**
1. Clonar el repositorio.
2. Asegurarse de que las rutas a los `Include` y `Lib` del **FBX SDK** estén configuradas en las propiedades del proyecto (`MinerEngine_2010.vcxproj` o el `.sln` actualizado).
3. Compilar y ejecutar.

<p align="center">

</p>
