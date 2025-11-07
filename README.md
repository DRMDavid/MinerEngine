<p align="center">
  <img src="https://img.shields.io/badge/Direct3D-11-1155BA?style=for-the-badge&logo=windows&logoColor=white" alt="D3D11"/>
  <img src="https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/Win32-API-1f6feb?style=for-the-badge" alt="Win32 API"/>
</p>

<h1 align="center">⛏️MinerEngine – Motor Gráfico Direct3D 11 (C++)</h1>

<p align="center">
  <i>Proyecto académico – Segundo Parcial | Materia: Gráficas 3D</i>
</p>

---

## 📘 Resumen

**⛏️MinerEngine** implementa el pipeline base de render en **Direct3D 11** (C++), gestionando: ventana (**Win32**), dispositivo y contexto (**ID3D11Device/Context**), **swap chain**, **RTV/DSV**, buffers, shaders y texturas.

**Mejoras Recientes (OBJ Loading, Normal Mapping y Anisotropic Filtering):**
El motor ahora soporta la **carga dinámica de modelos OBJ** con re-indexación, la **iluminación con Normal Mapping** (usando mapas de difusas y normales) y aplica **Filtro Anisotrópico (8x)** para mejorar la calidad de las texturas.

Se renderiza un **modelo 3D cargado (`Desert.obj`)** con rotación, iluminación y multi-texturizado con constantes por frame.

---

## 🧭 Índice

- [📘 Resumen](#-resumen)
- [🎯 Objetivos del Proyecto](#-objetivos-del-proyecto)
- [🏗️ Arquitectura General](#️-arquitectura-general)
  - [Componentes Principales](#componentes-principales)
  - [Relaciones Operativas](#relaciones-operativas)
- [🖥️ Pipeline Gráfico Implementado](#️-pipeline-gráfico-implementado)
- [🚀 Flujo de Inicialización](#-flujo-de-inicialización)
- [🔁 Flujo de Render (por frame)](#-flujo-de-render-por-frame)
- [🧩 API Clave por Clase](#-api-clave-por-clase)
- [🧪 Requisitos / Ejecución](#-requisitos--ejecución)


---

## 🎯 Objetivos del Proyecto

| Objetivo | Descripción | Nuevo Estado |
|---|---|---|
| Arquitectura base D3D11 | Separación de responsabilidades (ventana, dispositivo, contexto, recursos, presentación). | ✅ |
| Comprensión del pipeline | Recorrer creación de recursos, configuración del pipeline y ciclo de render. | ✅ |
| Gestión de recursos | Uso de RTV/DSV, buffers, SRV y texturas; limpieza y presentación cada frame. | ✅ |
| **Carga de Modelos 3D** | **Implementar OBJ Parser manual con re-indexación y triangulación.** | 🆕 ✅ |
| **Normal Mapping** | **Actualizar el formato de vértice (Pos/Normal/Tex) e implementar la vinculación de texturas de normales.** | 🆕 ✅ |
| **Calidad de Texturas** | **Uso de Filtro Anisotrópico (8x) para mejorar la calidad visual.** | 🆕 ✅ |
| Base para extensiones | Puntos de extensión (`update()`/`render()`) y TODOs. | ✅ |

---

## 🏗️ Arquitectura General

> **Nota rápida:** Diseño modular que favorece mantenibilidad, escalabilidad y reutilización.

### Componentes Principales

| Componente | Responsabilidad | API/Recursos Clave | Archivos |
|---|---|---|---|
| Window | Crear/administrar ventana Win32 (HWND, dimensiones), registrar clase y WndProc | `init(hInstance,nCmdShow,WndProc)`, `m_hWnd`, `m_width`, `m_height` | `Window.h/.cpp` |
| SwapChain | Crear `ID3D11Device`, `ID3D11DeviceContext` y `IDXGISwapChain`; configurar MSAA; `present` | `init(device,context,backBuffer,window)`, `present()` | `SwapChain.h/.cpp` |
| Device | Fábrica de recursos D3D11 | `Create{RenderTargetView,DepthStencilView,Texture2D,Buffer,Sampler,VertexShader,PixelShader,InputLayout}` | `Device.h/.cpp` |
| DeviceContext | Comandos de render y estado del pipeline | `RSSetViewports`, `OMSetRenderTargets`, `DrawIndexed`, `Clear*`, `VS/PSSet*`, `IASet*`, `UpdateSubresource` | `DeviceContext.h/.cpp` |
| **ModelLoader** | **Carga, parseo (OBJ) y optimización (re-indexación) de geometría.** | `init(mesh, fileName)` | `ModelLoader.h/.cpp` |
| Texture | Crear texturas (en memoria o desde otra), SRV | `init(...)` (soporte DDS), `m_texture`, `m_textureFromImg` | `Texture.h/.cpp` |
| RenderTargetView | RTV desde back buffer o textura; limpieza y bind | `init(...)`, `render(...)` | `RenderTargetView.h/.cpp` |
| DepthStencilView | DSV desde textura depth; limpieza y bind | `init(...)`, `render(context)` | `DepthStencilView.h/.cpp` |
| App / Main | Orquestación (inicialización, shaders, buffers, loop) | `InitDevice()`, `Render()`, `CleanupDevice()` | `MinerEngine.cpp`, `BaseApp.cpp` |

### Relaciones Operativas

| Origen → | Acción | → Destino | Resultado |
|---|---|---|---|
| App | **`ModelLoader::init("Desert.obj")`** | MeshComponent | Geometría compleja cargada, re-indexada y lista. |
| App | `Window::init()` | Window | Crea ventana y obtiene `HWND`, `width`, `height`. |
| App | `SwapChain::init(device,context,backBuffer,window)` | DXGI/D3D11 | Crea Device + Context + SwapChain; obtiene back buffer. |
| Device | `CreateRenderTargetView(backBuffer)` | RTV | RTV válido para OM (Output Merger). |
| Device | `CreateTexture2D(D24S8)` | Depth Texture | Textura depth para DSV. |
| Device | `CreateDepthStencilView(depth)` | DSV | DSV válido para OM. |
| DeviceContext | `OMSetRenderTargets(RTV, DSV)` | Pipeline | Vistas activas para dibujar. |
| DeviceContext | `RSSetViewports`, `IASet*`, `VS/PSSet*`, `DrawIndexed` | Pipeline | Configuración y draw call. |
| SwapChain | `present()` | DXGI | Intercambia buffers y muestra el frame. |

---

## 🖥️ Pipeline Gráfico Implementado

| Etapa | Descripción | APIs / Acciones |
|---|---|---|
| 1. Inicialización del SO/Win | Registro de clase, creación de ventana, show/update | Win32 API |
| 2. Dispositivo/Contexto/SwapChain | Crea `ID3D11Device`, `ID3D11DeviceContext`, `IDXGISwapChain`; configura MSAA | `D3D11CreateDevice`, `IDXGIFactory::CreateSwapChain` |
| 3. BackBuffer RTV | Crear RTV desde back buffer | `CreateRenderTargetView` |
| 4. Depth/Stencil | Crear textura depth y DSV | `CreateTexture2D`, `CreateDepthStencilView` |
| 5. Model Loading | **Carga la geometría (vértices y normales) y los índices a partir de un archivo OBJ.** | **`ModelLoader::init()`** |
| 6. Viewport y Shaders | Definir viewport; compilar VS/PS; **crear input layout con POSITION, NORMAL y TEXCOORD** | `RSSetViewports`, *CompileFromFile*, `Create{Vertex,Pixel}Shader`, `CreateInputLayout` |
| 7. Buffers y Recursos | Crear vertex/index/constant buffers; **SRV para Difusa y Normales**; **Sampler Anisotrópico** | `CreateBuffer`, `PSSetShaderResources`, `CreateSamplerState` |
| 8. Render Loop | Clear RTV/DSV, actualizar constantes, draw indexed, present | `Clear*`, `UpdateSubresource`, `DrawIndexed(m_mesh.m_numIndex,...)`, `present()` |

---

## 🔁 Flujo de Render (por frame)

| Orden | Acción | API/Clase | Efecto |
|---:|---|---|---|
| 1 | Limpiar RTV | `DeviceContext::ClearRenderTargetView` | Fondo uniforme; RT listo |
| 2 | Limpiar DSV | `DeviceContext::ClearDepthStencilView` | Z y stencil reiniciados |
| 3 | Set RTV/DSV | `OMSetRenderTargets(RTV, DSV)` | OM preparado para dibujar |
| 4 | Actualizar constantes | `UpdateSubresource(CBs)` | `World/View/Proj` y color por frame |
| 5 | Bind shaders/recursos | `VSSetShader`, `PSSetShader` | VS/PS activos |
| **6** | **Bind Texturas (Multi-slot)** | **`m_diffuseTexture.render(..., 0, 1)`, `m_normalTexture.render(..., 1, 1)`** | **Textura Difusa (t0) y Normal (t1) activas** |
| **7** | **Bind Sampler** | **`m_samplerState.render(..., 0, 1)`** | **Filtro Anisotrópico activado** |
| 8 | Dibujar | `IASet*`, **`DrawIndexed(m_mesh.m_numIndex,...)`** | Render del modelo cargado |
| 9 | Presentar | `SwapChain::present()` | Intercambio de buffers; imagen en pantalla |

---

## 🧩 API Clave por Clase

| Clase | Métodos/Funciones Clave | Resumen |
|---|---|---|
| `Window` | `init()`, `destroy()` | Crea y administra la ventana (`HWND`, ancho/alto). |
| `SwapChain` | `init()`, `present()`, `destroy()` | Crea Device/Context/SwapChain; MSAA; `Present`. |
| `Device` | `Create{...}` | Fábrica de recursos sobre `ID3D11Device`. |
| `DeviceContext` | `RSSetViewports`, `OMSetRenderTargets`, `Clear*`, `VS/PSSet*`, `IASet*`, `DrawIndexed`, `UpdateSubresource`, `destroy()` | Comandos a GPU y estados del pipeline. |
| **`ModelLoader`** | **`init(MeshComponent&, const std::string&)`** | **Parser de archivos OBJ (Pos/Tex/Normal), triangulación y deduplicación de vértices (re-indexación).** |
| `Texture` | `init(...)`, `render(...)`, `destroy()` | Texturas y SRV; ahora soporta **DDS** (carga de textura y normales). |
| `SamplerState` | **`init(Device&)`**, `render(...)` | **Configura y enlaza el Sampler con filtro Anisotrópico (8x)**. |
| `RenderTargetView` | `init(...)`, `render(context, dsv, numViews, clearColor)`, `destroy()` | RTV (back buffer o textura); clean + bind. |
| `DepthStencilView` | `init(...)`, `render(context)`, `destroy()` | DSV (depth/stencil); limpieza por frame. |
| `App/Main` | `InitDevice()`, `Render()`, `CleanupDevice()` | Orquesta inicialización, ciclo y liberación. |

---

## 🧪 Requisitos / Ejecución

| Ítem | Detalle |
|---|---|
| SO | Windows 10/11 |
| IDE | Visual Studio 2019 o superior |
| SDK | DirectX (D3D11) vía Windows SDK (útil: utilidades D3DX del ejemplo) |
| GPU | Compatible con Direct3D 11 |

| Pasos | Comando / Acción |
|---|---|
| Clonar | `git clone https://github.com/tu-usuario/MinerEngine.git` |
| Abrir | Cargar solución `.sln` en Visual Studio |
| Configurar | **Debug** o **Release** |
| Ejecutar | `Ctrl + F5` |


<p align="center">
  <img src="https://img.shields.io/badge/Estado-Estudiante-6f42c1?style=for-the-badge" alt="Estado Estudiante"/>
  <img src="https://img.shields.io/badge/Objetivo-Aprendizaje-success?style=for-the-badge" alt="Objetivo Aprendizaje"/>
  <img src="https://img.shields.io/badge/Render-OBJ%20%2B%20Normal%20Map-blueviolet?style=for-the-badge" alt="Render OBJ + Normal Map"/>
</p>
