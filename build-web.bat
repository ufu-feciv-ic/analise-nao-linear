@echo off
setlocal enabledelayedexpansion

echo Gathering source files...
set SRCS=
for /R src %%F in (*.cpp) do (
    set SRCS=!SRCS! "%%F"
)

REM Add external ImGui files
set SRCS=!SRCS! "external\imgui\imgui.cpp"
set SRCS=!SRCS! "external\imgui\imgui_draw.cpp"
set SRCS=!SRCS! "external\imgui\imgui_tables.cpp"
set SRCS=!SRCS! "external\imgui\imgui_widgets.cpp"
set SRCS=!SRCS! "external\rlimgui\rlImGui.cpp"

set INCLUDES=-I. -Isrc -Iexternal -Iexternal\imgui -Iexternal\rlimgui -Iexternal\eigen -IC:\raylib\raylib\src -IC:\raylib\raylib\src\external

set LIBS=C:\raylib\raylib\src\web\libraylib.a

REM Loop mechanism is successfully handled natively by emscripten_set_main_loop
set FLAGS=-Os -Wall -std=c++17 -DPLATFORM_WEB -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1

REM Shell file to use (Raylib default shell)
set SHELL_FILE=custom_shell.html

if not exist build\web mkdir build\web

echo Compiling for WebAssembly using Emscripten...
em++ !SRCS! !INCLUDES! !LIBS! !FLAGS! --shell-file "!SHELL_FILE!" --preload-file resources -o build\web\index.html

if %ERRORLEVEL% EQU 0 (
    echo.
    echo -------------------------------------------------------------
    echo Compilation successful! 
    echo To test, you MUST run a local web server in the "build\web" folder.
    echo Example: cd build\web ^&^& python -m http.server
    echo -------------------------------------------------------------
) else (
    echo.
    echo Compilation failed.
)
