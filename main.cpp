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
#include "eigenpch.h"
#include "imgui.h"
#include "implot.h"
#include "rlImGui.h"

int main()
{
    float screenWidth = 1024;
    float screenHeight = 768;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Análise Não Linear - Pórtico de Williams");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.zoom = 35.0f; 
    camera.offset = {50.0f, screenHeight / 2.0f + 100.0f}; 
    camera.target = {0, 0};

    rlImGuiSetup(true);
    ImPlot::CreateContext();

    Estrutura est;
    RenderizadorEstrutura renderizador;
    
    // =========================================================================
    // IMPLEMENTAÇÃO: PÓRTICO DE WILLIAMS (CONTROLE DE CARGA / NR PADRÃO)
    // Equivalente ao portico2dNLG (1).py
    // =========================================================================

    // Parâmetros idênticos ao Python
    int passos = 20;            // num_passos = 20
    int maxIter = 50;           // kmax = 50
    float tol = 0.01f;          // tol = 1e-6
    float deslocMax = 100.0f;   // Limite de segurança genérico
    
    // Carga total desejada
    float forcaTotal = -70.0f;  // P_total = -70 lb
    
    // Propriedades da seção transversal
    float modElast = 1.0f;
    float area = 1.885e6f;      // EA
    float inercia = 9.274e3f;   // EI
    float espessura = 6.0f;

    // Coordenadas exatas do Pórtico
    std::vector<std::pair<float, float>> coords = {
        {0.0f, 0.0f}, {2.5872f, 0.0736f}, {5.1744f, 0.1472f}, {7.7616f, 0.2208f},
        {10.3488f, 0.2944f}, {12.936f, 0.368f}, {15.5232f, 0.2944f}, {18.1104f, 0.2208f},
        {20.6976f, 0.1472f}, {23.2848f, 0.0736f}, {25.872f, 0.0f}
    };

    // 1. Criando os Nós
    for (size_t i = 0; i < coords.size(); i++)
    {
        bool engaste = (i == 0 || i == coords.size() - 1); 
        
        // No controle de carga, colocamos a força total diretamente no nó.
        // O algoritmo (resolverSistemaNaoLinear) vai multiplicar isso pelo fator lambda (0.0 até 1.0)
        float fy = (i == 5) ? forcaTotal : 0.0f; 
        
        est.adicionarNo({coords[i].first, coords[i].second, 0.0f, fy, 0.0f, engaste, engaste, engaste});
    }

    // 2. Criando as Barras
    for (size_t i = 0; i < est.nos.size() - 1; i++)
    {
        est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
    }

    // 3. Monitoramento
    int idNoMonitorado = est.nos[5].id; // Monitorando o nó do ápice
    int grauLiberdade = 1;              // Eixo Y

    // =========================================================================
    // PROCESSAMENTO: SOLVER NÃO-LINEAR (CONTROLE DE CARGA)
    // =========================================================================
    std::cout << "\nIniciando NR Controle de Carga. Carga Total: " << forcaTotal << " em " << passos << " passos.\n";
    
    auto startNaolinear = std::chrono::high_resolution_clock::now();
    
    // Chama o solver padrão de Newton-Raphson
    // A função tentará convergir. Como é controle de carga, ela deve avisar 
    // "Decomposição LLT falhou" (Matriz Singular) quando atingir o Snap-Through.
    est.resolverSistemaNaoLinearArco(passos, maxIter, tol, deslocMax, idNoMonitorado, grauLiberdade, forcaTotal);
    
    auto endNaoLinear = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
    
    float escalaVisualizacao = 1.0f;
    est.calcularPontosDeformadaEstrutura(escalaVisualizacao);

    // // =========================================================================
    // // IMPLEMENTAÇÃO: VIGA EM BALANÇO (GRANDES DESLOCAMENTOS - NR PADRÃO)
    // // =========================================================================

    // // Parâmetros numéricos do solver (Newton-Raphson Padrão - Controle de Carga)
    // int passos = 20;            // Número de incrementos de carga
    // int maxIter = 50;           // Iterações máximas por passo
    // float tol = 1e-6f;          // Tolerância para o resíduo
    // float deslocMax = 100.0f;   // Limite de segurança genérico

    // // Carga total aplicada na ponta livre da viga (ajustada para gerar grande deformação)
    // float forcaTotal = -1.5e6f; // [N] Força vertical para baixo

    // // Propriedades do material e seção (Ex: Viga de Concreto/Aço genérica)
    // float modElast = 200E9f;    // [Pa] Módulo de Elasticidade (200 GPa)
    // float base = 0.2f;          // [m] 
    // float altura = 0.4f;        // [m] 
    // float area = base * altura;
    // float inercia = (base * std::pow(altura, 3)) / 12.0f;
    // float espessura = 6.0f;     // Apenas para renderização visual

    // // Geometria da Viga
    // float comprimentoTotal = 10.0f; // [m] Viga longa para facilitar a visualização
    // int numDivisoes = 10;           // Discretização (10 elementos)

    // // 1. Criando os Nós
    // for (int i = 0; i <= numDivisoes; i++)
    // {
    //     float x = (comprimentoTotal / numDivisoes) * i;
    //     float y = 0.0f;

    //     // Engaste perfeito apenas no primeiro nó (x = 0)
    //     bool engaste = (i == 0); 
        
    //     // A carga total é colocada apenas no último nó. 
    //     // O solver multiplicará isso pelo fator de carga 'lambda' a cada passo.
    //     float fy = (i == numDivisoes) ? forcaTotal : 0.0f; 
        
    //     est.adicionarNo({x, y, 0.0f, fy, 0.0f, engaste, engaste, engaste});
    // }

    // // 2. Criando as Barras
    // for (size_t i = 0; i < est.nos.size() - 1; i++)
    // {
    //     est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
    // }

    // // 3. Monitoramento
    // int idNoMonitorado = est.nos.back().id; // Monitorando a ponta livre da viga
    // int grauLiberdade = 1;                  // 1 representa o eixo Y (deslocamento vertical)

    // // =========================================================================
    // // PROCESSAMENTO: SOLVER NÃO-LINEAR (CONTROLE DE CARGA)
    // // =========================================================================
    // std::cout << "\n--- Iniciando Analise: Viga em Balanço (Newton-Raphson) ---\n";
    
    // auto startNaolinear = std::chrono::high_resolution_clock::now();
    
    // // Chama o solver padrão de Newton-Raphson que você já tem no Estrutura.cpp
    // est.resolverSistemaNaoLinear(passos, maxIter, tol, deslocMax, idNoMonitorado, grauLiberdade, forcaTotal);
    
    // auto endNaoLinear = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
    
    // // Configuração para renderização 1:1 na Raylib
    // float escalaVisualizacao = 1.0f; 
    // est.calcularPontosDeformadaEstrutura(escalaVisualizacao);

    // =========================================================================
    // LOOP PRINCIPAL DE RENDERIZAÇÃO
    // =========================================================================
    while (!WindowShouldClose())
    {
        // Controles de Câmera
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }
        
        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            float scale = 0.2f * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 640.0f);
        }

        // --- RENDERIZAÇÃO DA RAYLIB ---
        BeginDrawing();
        ClearBackground({35, 35, 40, 255}); 

        BeginMode2D(camera);
            DrawLineEx({ -(float)GetScreenWidth(), 0.0f}, { (float)GetScreenWidth(), 0.0f}, 2.0/camera.zoom, { 60, 60, 60, 255});
            DrawLineEx({0.0f, -(float)GetScreenWidth()}, {0.0f, (float)GetScreenWidth()}, 2.0/camera.zoom, { 60, 60, 60, 255});

            // Renderiza estrutura base, nós e reações
            renderizador.desenhaEstrutura(est, camera);

            // A deformada final capturada após os 33 passos
            renderizador.desenhaDeformada(est, RED, camera);
            renderizador.desenhaPontoDeformada(est, escalaVisualizacao, camera.zoom);
        EndMode2D();

        // --- RENDERIZAÇÃO DA INTERFACE IMGUI / IMPLOT ---
        rlImGuiBegin();
        
        // Processamento dos dados para plotagem
        std::vector<float> xData, yData;
        float h_apex = 0.368f; // Altura do pórtico para normalização

        for (const auto& par : est.historicoDeslocamentos)
        {
            // Par.first = u (deslocamento)
            // Par.second = lambda (fator de carga). Como nossa força base é -1, a carga real é o próprio lambda.
            xData.push_back(std::abs(par.first) / h_apex); 
            yData.push_back(std::abs(par.second));
        }

        ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(screenWidth - 370, 20), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Painel de Controle NLG");

        if (ImGui::CollapsingHeader("Trajetória de Equilíbrio", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImPlot::BeginPlot("##CurvaPxU", ImVec2(-1, 300)))
            {
                ImPlot::SetupAxes("Deslocamento no ápice (v/h)", "Força Aplicada P (lb)");
                ImPlot::SetupAxesLimits(0, 2.0, -0.5, 1.2, ImPlotCond_Once);

                if (!xData.empty() && !yData.empty())
                {
                    ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), 2.0f);
                    ImPlot::PlotLine("Arc-Length", xData.data(), yData.data(), (int)xData.size());
                    
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.0f, ImVec4(1.0f, 0.8f, 0.2f, 1.0f), -1.0f, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                    ImPlot::PlotScatter("Passos", xData.data(), yData.data(), (int)xData.size());
                }
                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("Dados da Simulação", ImGuiTreeNodeFlags_DefaultOpen)) 
        {
            // ImGui::Text("Nós: %d | Barras: %d", (int)est.nos.size(), (int)est.barras.size());
            // ImGui::Separator();
            // ImGui::Text("Método: Arc-Length (Comprimento de Arco)");
            // ImGui::Text("Passos realizados: %d", (int)est.historicoDeslocamentos.size() - 1);
            // ImGui::Text("Tolerância: %.1e", tol);
            // ImGui::Separator();
            // ImGui::Text("Tempo do Solver: %.4f s", durationNaoLinear.count());
            // ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✅ Curva Snap-Through capturada com sucesso!");
        }

        ImGui::End();
        rlImGuiEnd();

        DrawFPS(10, 10);
        EndDrawing();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}