#include "raylib.h"
#include <iostream>
#include "raymath.h"
#include <vector>
#include <array>
#include <cstring>
#include <chrono>
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

    // // Viga engastada
    // est.adicionarNo({0.0f, 0.0f,0, 0, 0, true, true, true});
    // est.adicionarNo({2.0f, 0.0f, -2.3f, 3.8f, 0.0f, false, false, false});

    // // Pórtico
    // est.adicionarNo({0.0f, 0.0f, 0, 5, 0, true, false, true});
    // est.adicionarNo({2.0f, 8.0f, -2.3f, 3.8f, 5.0f, true, false, false});
    // est.adicionarNo({6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, true, false});
    // est.adicionarNo({8.0f, 0.0f, -3.0f, -2.2f, -5.0f, false, false, true});

    // // Pórtico
    // est.adicionarNo({0.0f, 0.0f,0, 0, 0, true, true, false});
    // est.adicionarNo({2.0f, 8.0f, -2.3f, 3.8f, 15.0f, false, false, false});
    // est.adicionarNo({6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, false, false});
    // est.adicionarNo({8.0f, 0.0f, -3.0f, -2.2f, -5.0f, true, true, true});


    // // Viga biapoiada com carga no meio do vão
    // est.adicionarNo({0.0f, 0.0f, 0, 0, 0, true, true, false});
    // est.adicionarNo({5.0f, 3.0f, 0.0f, 0.0f, 0.0f, false, false, false});
    // est.adicionarNo({10.0f, 6.0f, 1000.0f, 0, 0, false, true, false});

    float base = 0.1;
    float altura = 0.2;
    float area = base * altura;
    float inercia = (base * pow(altura, 3)) / 12.0f;
    float modElast = 200E9;
    float espessura = 6;
    float forcaHorizontal = 100.0f;

    // // Cria as conexões entre os nós em sequência
    // for (size_t i = 0; i < est.nos.size() - 1; i++)
    // {
    //     est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
    // }

    // est.adicionarBarra(est.nos[0], est.nos[1], 0, 1, modElast, area, inercia, espessura);
    // est.adicionarBarra(est.nos[1], est.nos[2], 1, 2, modElast, area, inercia, espessura);
    // est.adicionarBarra(est.nos[2], est.nos[3], 2, 3, modElast, area, inercia, espessura);
    // est.adicionarBarra(3, 0, modElast, area, inercia, espessura);

    int numPilares = 15;
    int numAndares = 55;
    int espacamentPilares = 5;
    int alturaAndar = 3;

    std::vector<std::vector<int>> idsNos(numAndares + 1, std::vector<int>(numPilares));

    // for (int i = 0; i < numPilares; i++)
    // {
    //     float xPos = i * espacamentPilares;

    //     for (int j = 0; j < numAndares; j++)
    //     {
    //         float yPos = j * alturaAndar;

    //         bool fixoX = (j == 0); // Fixar X apenas no primeiro andar
    //         bool fixoY = (j == 0); // Fixar Y apenas no primeiro andar
    //         bool rotaZ = (j == 0); // Fixar rotação apenas no primeiro andar

    //         est.adicionarNo({xPos, yPos, 0, 0, 0, fixoX, fixoY, rotaZ});
    //     }
    // }

    for (int j = 0; j < numAndares; j++)
    {
        float yPos = j * alturaAndar;

        for (int i = 0; i < numPilares; i++)
        {
            float xPos = i * espacamentPilares;

            bool fixoX = (j == 0); // Fixar X apenas no primeiro andar
            bool fixoY = (j == 0); // Fixar Y apenas no primeiro andar
            bool rotaZ = (j == 0); // Fixar rotação apenas no primeiro andar

            float fx = 0.0f;
            float fy = 0.0f;
            float mz = 0.0f;

            if (j == numAndares-1 && i == 0)
            {
                fx = forcaHorizontal;
            }

            No novoNo = No(xPos, yPos, fx, fy, mz, fixoX, fixoY, rotaZ);
            est.adicionarNo(novoNo);

            idsNos[j][i] = novoNo.id;
        }
    }

    // for (size_t i = 0; i < est.nos.size(); i++)
    // {
    //     const No& no = est.nos[i];
    //     std::cout << "Nó ID: " << no.id << " - Posição: (" << no.x << ", " << no.y << ") - Fixações: (X: " << no.fixoX << ", Y: " << no.fixoY << ", Z: " << no.rotaZ << ")" << std::endl;
    //     std::cout << "     Forças: (Fx: " << no.fx << ", Fy: " << no.fy << ", Mz: " << no.mz << ")" << std::endl;
    // }

    for (int i = 0; i < numPilares; i++)
    {
        for (int j = 0; j < numAndares - 1; j++)
        {
            int noInferiorId = idsNos[j][i];
            int noSuperiorId = idsNos[j + 1][i];

            const No& noInferior = est.getNoById(noInferiorId);
            const No& noSuperior = est.getNoById(noSuperiorId);

            est.adicionarBarra(noInferior, noSuperior, noInferiorId, noSuperiorId, modElast, area, inercia, espessura);
        }
    }

    for (int j = 1; j < numAndares; j++)
    {
        for (int i = 0; i < numPilares - 1; i++)
        {
            int noEsquerdoId = idsNos[j][i];
            int noDireitoId = idsNos[j][i + 1];

            const No& noEsquerdo = est.getNoById(noEsquerdoId);
            const No& noDireito = est.getNoById(noDireitoId);

            est.adicionarBarra(noEsquerdo, noDireito, noEsquerdoId, noDireitoId, modElast, area, inercia, espessura);
        }
    }

    // est.calcularMatrizRigidezEstrutura();
    // est.montarVetorForcas();
    // est.aplicarCondicoesDeContorno();
    //est.resolverSistema();
    //est.calcularPontosDeformadaEstrutura(20e4);

    auto startDenso = std::chrono::high_resolution_clock::now();
    est.resolverSistema();
    auto endDenso = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationDenso = endDenso - startDenso;
    std::cout << "Tempo de resolução do sistema denso: " << durationDenso.count() << " ms." << std::endl;

    auto startEsparso = std::chrono::high_resolution_clock::now();
    est.resolverSistemaEsparsa();
    auto endEsparso = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationEsparso = endEsparso - startEsparso;
    std::cout << "Tempo de resolução do sistema esparso: " << durationEsparso.count() << " ms." << std::endl;

    float escalaMaxima = 10e4;
    const float velocidadeAnimacao = 1.0f;
    
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

        float tempoAtual = (float)GetTime();

        float fatorSenoidal = sinf(tempoAtual * velocidadeAnimacao);

        float escalaAnimada = escalaMaxima * fatorSenoidal;

        BeginDrawing();
            ClearBackground({40, 40, 40, 255});
            BeginMode2D(camera);
                DrawLineEx({ -(float)GetScreenWidth()/2, 0.0f}, { (float)GetScreenWidth()/2, 0.0f}, 3.0/camera.zoom, { 30, 30, 30, 255});
                DrawLineEx({0.0f, -(float)GetScreenWidth()/2}, {0.0f, (float)GetScreenWidth()/2}, 3.0/camera.zoom, { 30, 30, 30, 255});

                //camada controladora do desenho 
                renderizador.desenhaEstrutura(est, camera);

                if (IsKeyDown(KEY_SPACE))
                {
                    renderizador.desenhaDeformada(est, RED, camera);
                }
                
                if (IsKeyDown(KEY_D))
                {
                    renderizador.desenhaPontoDeformada(est, camera.zoom);
                }
            
                //renderizador.desenhaDeformadaAnimada(est, escalaAnimada, RED, camera);

            EndMode2D();
        EndDrawing();

    }

    // std::cout << "Fazer deformada" << std::endl;
    // std::cout << "Fazer reações de apoio" << std::endl;
    // std::cout << "Ver tabelas de carregamentos padrões" << std::endl;
    
    CloseWindow();
    return 0;
}