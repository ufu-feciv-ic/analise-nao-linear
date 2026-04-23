# Project Overview: Analise Nao Linear (2D Structural Editor)

This project is a professional-grade 2D structural engineering editor designed for creating and analyzing structural models. It features a clean, pass-based rendering system, a robust tool-based interaction model, and a comprehensive undo/redo system.

## Core Technologies
- **Language:** C++17
- **Graphics & Windowing:** [Raylib](https://www.raylib.com/)
- **UI Framework:** [Dear ImGui](https://github.com/ocornut/imgui) (integrated via [rlImGui](https://github.com/raylib-extras/rlImGui))
- **Linear Algebra:** [Eigen](https://eigen.tuxfamily.org/)
- **Serialization:** [RapidJSON](https://rapidjson.org/)
- **Build System:** Makefile (MinGW/w64devkit on Windows) or Emscripten for Web.

## Architecture

The project follows a decoupled architecture:

### 1. Data Model (`src/model/`)
- `ProjectDocument`: The primary container for persistent project data (Nodes, Beams, Materials, Sections, Loads).
- `Node`, `Beam`, `StructuralMaterial`, `Section`: Core entities representing structural components.

### 2. Editor Logic (`src/editor/`)
- `Editor`: Orchestrates the interactive state, camera, selection, and history. It delegates tool-specific logic to controllers.
- `EditorState`: Holds transient interaction state (hovered items, selection set, active tool status).
- `FrameRequests`: A bridge for UI-to-Editor communication, ensuring the UI remains decoupled from the core logic.
- `ProjectDerivedData`: A cache for data computed from the document (e.g., bounding boxes, lookup indices) to optimize rendering and hit-testing.

### 3. Rendering (`src/editor/render/` & `EditorRenderer`)
- Uses a multi-pass approach: Background -> Structure -> Results -> Overlays -> Dimensions -> Previews.
- Results rendering is designed to be integrated as a specific pass reading from an `AnalysisResult` (as described in `guia_integracao_editor_analise.md`).

### 4. Application Layer (`src/app/`)
- `Application`: Coordinates the main loop, ImGui frame building, and maps UI requests to editor operations.

## Building and Running

### Native (Windows)
Requires `raylib` and `w64devkit` (or a similar MinGW-w64 environment) installed at standard paths (default `C:/raylib`).

- **Build:** `make`
- **Run:** `run.bat` or `make run`
- **Clean:** `make clean`
- **Debug/Release:** `make BUILD_MODE=DEBUG` or `make BUILD_MODE=RELEASE` (default)

### Web (Emscripten)
- Use `build-web.bat` (requires Emscripten SDK configured).

## Development Conventions

- **Surgical Updates:** When modifying `ProjectDocument`, ensure you update persistence (`Editor.Persistence.cpp`) and history (`Editor.DocumentHistory.cpp`).
- **UI Decoupling:** UI components (`TopToolbar`, `LeftPanel`) should not modify the document directly. Use `FrameRequests` to signal intent to the `Application` layer.
- **Units:** Internal calculations must use SI units (m, N, Pa, etc.). Unit conversion for display is handled in the UI layer using `UnitConversion.h`.
- **Naming:** Follow the existing PascalCase for classes and methods, camelCase for variables.

## Integration of Structural Analysis
The project is currently evolving to include non-linear structural analysis. Refer to `guia_integracao_editor_analise.md` for the architectural blueprint of the `src/analysis/` module and how to hook results into the `EditorRenderer`.
