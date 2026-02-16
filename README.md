# ⛏️ MinerEngine – Motor Gráfico & ECS (C++)

<p align="center">
  <img src="https://img.shields.io/badge/Direct3D-11-1155BA?style=for-the-badge&logo=windows&logoColor=white" alt="D3D11"/>
  <img src="https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/Win32-API-1f6feb?style=for-the-badge" alt="Win32 API"/>
  <img src="https://img.shields.io/badge/ECS-Architecture-orange?style=for-the-badge" alt="ECS"/>
</p>

---

## 📘 Resumen Actualizado
**MinerEngine** es un motor gráfico y de videojuegos de alto rendimiento desarrollado en C++ y **Direct3D 11**. Implementa una arquitectura **Entity-Component-System (ECS)** para una gestión modular de entidades y lógica de juego.

Esta versión ha evolucionado para incluir un pipeline de renderizado más avanzado, con soporte para mapeo de cubos (Skyboxes), carga de texturas multiformato y un sistema de optimización de mallas.

---

## ✨ Características Principales

| Característica | Descripción |
|---|---|
| **Arquitectura ECS** | Gestión de `Actors` y `Components` para desacoplar datos de comportamiento. |
| **Skybox (Cubemaps)** | Soporte para texturas de 6 caras con generación automática de Mipmaps para reflejos y fondos. |
| **Carga de Texturas** | Integración de **stb_image** para soportar `.png` y `.jpg`, además del soporte nativo para `.dds`. |
| **Model Loader (OBJ/FBX)** | Carga y re-indexación de geometría para eliminar vértices duplicados y optimizar memoria de video. |
| **Custom Memory** | Gestión de memoria mediante punteros inteligentes propios: `TSharedPointer`, `TWeakPointer`, etc. |
| **ImGui Tooling** | Interfaz de depuración integrada para inspección de entidades y recursos en tiempo real. |

---

## 🏗️ Arquitectura del Motor

### Core & Utilities
El motor prioriza el control total sobre el hardware evitando la STL estándar en áreas críticas:
* **Memory:** Implementación de punteros inteligentes para evitar fugas de memoria.
* **Math:** Librería de álgebra lineal (`EngineMath.h`) con soporte para Cuaterniones y Matrices 4x4.

### Sistema de Gráficos
* **Texture Class:** Ahora permite inicializar texturas desde archivo, memoria o crear vistas específicas para Cubemaps.
* **ResourceManager:** Singleton encargado de cachear recursos (`IResource`) para evitar cargas redundantes.
* **Renderer:** Pipeline basado en D3D11 que gestiona estados de renderizado, buffers constantes y sombreadores.

---

## 🖥️ Tecnologías Integradas

* **Direct3D 11:** API principal de renderizado.
* **Win32 API:** Manejo de ventanas e inputs nativos de Windows.
* **STB Image:** Decodificación de imágenes `.png` y `.jpg` integrada en el cargador de texturas.
* **FBX SDK:** Soporte para modelos de grado industrial.

---

## 🧪 Requisitos / Ejecución

1. **IDE:** Visual Studio 2019/2022.
2. **SDKs:** DirectX SDK y FBX SDK configurados en el proyecto.
3. **Compilación:** x64 recomendado en modo Debug o Release.
