#include "raylib.h"
#include <iostream>
#include "raymath.h"
#include <vector>
#include <array>
#include <cstring>
#include "DesenhoUtils.h"
#include "Estrutura.h"
#include "RenderizadorEstrutura.h"

int main()
{
    float screenWidth = 800;
    float screenHeight = 600;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Análise Não Linear");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.zoom = 25.0f;
    camera.offset = {screenWidth / 2, screenHeight / 2};
    camera.target = {0, 0};

    // std::vector<No> nos;
    // nos.emplace_back(0.0f, 0.0f,0, 0, 0, true, false, true, camera);
    // nos.emplace_back(2.0f, 8.0f, -2.3f, 3.8f, 15.0f, true, false, false, camera);
    // nos.emplace_back(6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, true, false, camera);
    // nos.emplace_back(8.0f, 0.0f, -3.0f, -2.2f, -5.0f, false, false, true, camera);

    // std::vector<std::array<int, 2>> conexoes;
    // conexoes.push_back({0, 1});
    // conexoes.push_back({1, 2});
    // conexoes.push_back({2, 3});

    // for (const auto &conexao : conexoes)
    // {
    //     std::cout << "Conexão entre nós: " << conexao[0] << " e " << conexao[1] << std::endl;
    // }

    // Estrutura est(nos, conexoes);

    Estrutura est;
    RenderizadorEstrutura renderizador;

    est.adicionarNo({0.0f, 0.0f,0, 0, 0, true, false, true});
    est.adicionarNo({2.0f, 8.0f, -2.3f, 3.8f, 15.0f, true, false, false});
    est.adicionarNo({6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, true, false});
    est.adicionarNo({8.0f, 0.0f, -3.0f, -2.2f, -5.0f, false, false, true});

    float base = 0.1;
    float altura = 0.2;
    float area = base * altura;
    float inercia = (base * pow(altura, 3)) / 12.0f;
    float modElast = 200E9;
    float espessura = 6;

    est.adicionarBarra(0, 1, modElast, area, inercia, espessura);
    est.adicionarBarra(1, 2, modElast, area, inercia, espessura);
    est.adicionarBarra(2, 3, modElast, area, inercia, espessura);
    // est.adicionarBarra(3, 0, modElast, area, inercia, espessura);

    est.calcularMatrizRigidezEstrutura();

    while (!WindowShouldClose())
    {
        { // CAMERA
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f / camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }
            {
                // Zoom based on mouse wheel
                float wheel = GetMouseWheelMove();
                if (wheel != 0)
                {
                    // Get the world point that is under the mouse
                    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

                    // Set the offset to where the mouse is
                    camera.offset = GetMousePosition();

                    // Set the target to match, so that the camera maps the world space point
                    // under the cursor to the screen space point under the cursor at any zoom
                    camera.target = mouseWorldPos;

                    // Zoom increment
                    // Uses log scaling to provide consistent zoom speed
                    float scale = 0.2f * wheel;
                    camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 640.0f);
                }
            }
        }

        BeginDrawing();
            ClearBackground({40, 40, 40, 255});
            BeginMode2D(camera);
                DrawLineEx({ -(float)GetScreenWidth()/2, 0.0f}, { (float)GetScreenWidth()/2, 0.0f}, 3.0/camera.zoom, { 30, 30, 30, 255});
                DrawLineEx({0.0f, -(float)GetScreenWidth()/2}, {0.0f, (float)GetScreenWidth()/2}, 3.0/camera.zoom, { 30, 30, 30, 255});

                // camada controladora do desenho 
                renderizador.desenhaEstrutura(est, camera);

            EndMode2D();
        EndDrawing();

    }
    CloseWindow();
    return 0;
}