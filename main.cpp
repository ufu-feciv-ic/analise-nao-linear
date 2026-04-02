// #include "raylib.h"
// #include <iostream>
// #include "raymath.h"
// #include <vector>
// #include <array>
// #include <cstring>
// #include <chrono>
// #include "DesenhoUtils.h"
// #include "Estrutura.h"
// #include "RenderizadorEstrutura.h"
// #include "eigenpch.h"
// #include "imgui.h"
// #include "implot.h"
// #include "rlImGui.h"

// int main()
// {
//     float screenWidth = 800;
//     float screenHeight = 600;
//     SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
//     InitWindow(screenWidth, screenHeight, "Análise Não Linear");
//     SetTargetFPS(60);

//     Camera2D camera = {0};
//     camera.zoom = 25.0f;
//     camera.offset = {screenWidth / 2, screenHeight / 2};
//     camera.target = {0, 0};

//     rlImGuiSetup(true);
//     ImPlot::CreateContext();

//     // std::vector<No> nos;
//     // nos.emplace_back(0.0f, 0.0f,0, 0, 0, true, false, true, camera);
//     // nos.emplace_back(2.0f, 8.0f, -2.3f, 3.8f, 15.0f, true, false, false, camera);
//     // nos.emplace_back(6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, true, false, camera);
//     // nos.emplace_back(8.0f, 0.0f, -3.0f, -2.2f, -5.0f, false, false, true, camera);

//     // std::vector<std::array<int, 2>> conexoes;
//     // conexoes.push_back({0, 1});
//     // conexoes.push_back({1, 2});
//     // conexoes.push_back({2, 3});

//     // for (const auto &conexao : conexoes)
//     // {
//     //     std::cout << "Conexão entre nós: " << conexao[0] << " e " << conexao[1] << std::endl;
//     // }

//     // Estrutura est(nos, conexoes);

//     Estrutura est;
//     RenderizadorEstrutura renderizador;

//     // // Viga engastada
//     // est.adicionarNo({0.0f, 0.0f,0, 0, 0, true, true, true});
//     // est.adicionarNo({2.0f, 0.0f, -2.3f, 3.8f, 0.0f, false, false, false});

//     // // Pórtico
//     // est.adicionarNo({0.0f, 0.0f, 0, 5, 0, true, false, true});
//     // est.adicionarNo({2.0f, 8.0f, -2.3f, 3.8f, 5.0f, true, false, false});
//     // est.adicionarNo({6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, true, false});
//     // est.adicionarNo({8.0f, 0.0f, -3.0f, -2.2f, -5.0f, false, false, true});

//     // // Pórtico
//     // est.adicionarNo({0.0f, 0.0f,0, 0, 0, true, true, false});
//     // est.adicionarNo({2.0f, 8.0f, -2.3f, 3.8f, 15.0f, false, false, false});
//     // est.adicionarNo({6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, false, false});
//     // est.adicionarNo({8.0f, 0.0f, -3.0f, -2.2f, -5.0f, true, true, true});


//     // // Viga biapoiada com carga no meio do vão
//     // est.adicionarNo({0.0f, 0.0f, 0, 0, 0, true, true, false});
//     // est.adicionarNo({5.0f, 3.0f, 0.0f, 0.0f, 0.0f, false, false, false});
//     // est.adicionarNo({10.0f, 6.0f, 1000.0f, 0, 0, false, true, false});

//     // float base = 0.1;
//     // float altura = 0.2;
//     // float area = base * altura;
//     // float inercia = (base * pow(altura, 3)) / 12.0f;
//     // float modElast = 200E9;
//     // float espessura = 6;
//     // float forcaHorizontal = 100.0f;

//     // // // Cria as conexões entre os nós em sequência
//     // // for (size_t i = 0; i < est.nos.size() - 1; i++)
//     // // {
//     // //     est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
//     // // }

//     // // est.adicionarBarra(est.nos[0], est.nos[1], 0, 1, modElast, area, inercia, espessura);
//     // // est.adicionarBarra(est.nos[1], est.nos[2], 1, 2, modElast, area, inercia, espessura);
//     // // est.adicionarBarra(est.nos[2], est.nos[3], 2, 3, modElast, area, inercia, espessura);
//     // // est.adicionarBarra(3, 0, modElast, area, inercia, espessura);

//     // int numPilares = 15;
//     // int numAndares = 55;
//     // int espacamentPilares = 5;
//     // int alturaAndar = 3;

//     // std::vector<std::vector<int>> idsNos(numAndares + 1, std::vector<int>(numPilares));

//     // // for (int i = 0; i < numPilares; i++)
//     // // {
//     // //     float xPos = i * espacamentPilares;

//     // //     for (int j = 0; j < numAndares; j++)
//     // //     {
//     // //         float yPos = j * alturaAndar;

//     // //         bool fixoX = (j == 0); // Fixar X apenas no primeiro andar
//     // //         bool fixoY = (j == 0); // Fixar Y apenas no primeiro andar
//     // //         bool rotaZ = (j == 0); // Fixar rotação apenas no primeiro andar

//     // //         est.adicionarNo({xPos, yPos, 0, 0, 0, fixoX, fixoY, rotaZ});
//     // //     }
//     // // }

//     // for (int j = 0; j < numAndares; j++)
//     // {
//     //     float yPos = j * alturaAndar;

//     //     for (int i = 0; i < numPilares; i++)
//     //     {
//     //         float xPos = i * espacamentPilares;

//     //         bool fixoX = (j == 0); // Fixar X apenas no primeiro andar
//     //         bool fixoY = (j == 0); // Fixar Y apenas no primeiro andar
//     //         bool rotaZ = (j == 0); // Fixar rotação apenas no primeiro andar

//     //         float fx = 0.0f;
//     //         float fy = 0.0f;
//     //         float mz = 0.0f;

//     //         if (j == numAndares-1 && i == 0)
//     //         {
//     //             fx = forcaHorizontal;
//     //         }

//     //         No novoNo = No(xPos, yPos, fx, fy, mz, fixoX, fixoY, rotaZ);
//     //         est.adicionarNo(novoNo);

//     //         idsNos[j][i] = novoNo.id;
//     //     }
//     // }

//     // // for (size_t i = 0; i < est.nos.size(); i++)
//     // // {
//     // //     const No& no = est.nos[i];
//     // //     std::cout << "Nó ID: " << no.id << " - Posição: (" << no.x << ", " << no.y << ") - Fixações: (X: " << no.fixoX << ", Y: " << no.fixoY << ", Z: " << no.rotaZ << ")" << std::endl;
//     // //     std::cout << "     Forças: (Fx: " << no.fx << ", Fy: " << no.fy << ", Mz: " << no.mz << ")" << std::endl;
//     // // }

//     // for (int i = 0; i < numPilares; i++)
//     // {
//     //     for (int j = 0; j < numAndares - 1; j++)
//     //     {
//     //         int noInferiorId = idsNos[j][i];
//     //         int noSuperiorId = idsNos[j + 1][i];

//     //         const No& noInferior = est.getNoById(noInferiorId);
//     //         const No& noSuperior = est.getNoById(noSuperiorId);

//     //         est.adicionarBarra(noInferior, noSuperior, noInferiorId, noSuperiorId, modElast, area, inercia, espessura);
//     //     }
//     // }

//     // for (int j = 1; j < numAndares; j++)
//     // {
//     //     for (int i = 0; i < numPilares - 1; i++)
//     //     {
//     //         int noEsquerdoId = idsNos[j][i];
//     //         int noDireitoId = idsNos[j][i + 1];

//     //         const No& noEsquerdo = est.getNoById(noEsquerdoId);
//     //         const No& noDireito = est.getNoById(noDireitoId);

//     //         est.adicionarBarra(noEsquerdo, noDireito, noEsquerdoId, noDireitoId, modElast, area, inercia, espessura);
//     //     }
//     // }

//     // // est.calcularMatrizRigidezEstrutura();
//     // // est.montarVetorForcas();
//     // // est.aplicarCondicoesDeContorno();
//     // //est.resolverSistema();
//     // //est.calcularPontosDeformadaEstrutura(20e4);

//     // auto startDenso = std::chrono::high_resolution_clock::now();
//     // est.resolverSistema();
//     // auto endDenso = std::chrono::high_resolution_clock::now();
//     // std::chrono::duration<double> durationDenso = endDenso - startDenso;
//     // std::cout << "Tempo de resolução do sistema denso: " << durationDenso.count() << " ms." << std::endl;

//     // auto startEsparso = std::chrono::high_resolution_clock::now();
//     // est.resolverSistemaEsparsa();
//     // auto endEsparso = std::chrono::high_resolution_clock::now();
//     // std::chrono::duration<double> durationEsparso = endEsparso - startEsparso;
//     // std::cout << "Tempo de resolução do sistema esparso: " << durationEsparso.count() << " ms." << std::endl;

//     // float escalaMaxima = 10e4;
//     // const float velocidadeAnimacao = 1.0f;

//     // // Parâmetros numéricos do solver
//     // float Pmax = 82904.67 * 0.95; // [N]
//     // int passos = 60; // número de passos de carga até chegar em Pmax
//     // int maxIter = 200; // máximo de iterações de Newton por passo
//     // float tol = 1.0f; // tolerância para norma de resíduo 
//     // float deslocMax = 10.0f; // [m] deslocamento máximo para o nó monitorado
//     // float espessura = 6.0f; // espessura para o desenho da linha 

//     // // Propriedades do material/seção
//     // float modElast = 210E9; // [Pa] módulo de elasticidade
//     // float area = 1e-4; // [m²] área
//     // float inercia = 1e-6; // [m4] momento de inércia

//     // // Geometria 
//     // float comprimento = 5.0f; // [m] comprimento total da coluna
//     // float delta0 = 0.01f; // [m] amplitude de imperfeição inicial lateral
//     // int numDivBarra = 20; // numero de elementos ao longo da coluna

//     // for (int i = 0; i <= numDivBarra; i++)
//     // {
//     //     float t = (float)i / numDivBarra;
//     //     float y = comprimento * t;

//     //     float x = 0.0f;
//     //     float w = delta0 * sinf(3.14159265f * t); // imperfeição senoidal

//     //     bool fixoX = (i == 0) || (i == numDivBarra); // Fixar X
//     //     bool fixoY = (i == 0); // Fixar Y apenas na base

//     //     float fy = (i == numDivBarra) ? -Pmax : 0.0f; // carga concentrada na ponta

//     //     est.adicionarNo({x + w, y, 0, fy, 0, fixoX, fixoY, 0});
//     // }

//     // Eigen::VectorXf desl;

//     // // Cria as conexões entre os nós em sequência
//     // for (size_t i = 0; i < est.nos.size() - 1; i++)
//     // {
//     //     est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
//     //     desl.resize(est.nos.size() * 3);
//     //     desl.setZero();
//     // }


//     // int idNoMonitorado = est.nos.back().id; // nó monitorado (ponta da coluna)

//     // =========================================================================
//     // IMPLEMENTAÇÃO: BARRA (SNAP-THROUGH 1-DOF) - Ref: BarraNLGCA.py
//     // =========================================================================

//     // // Parâmetros numéricos do solver (Método do Comprimento de Arco)
//     // int nmax = 22;          // Número de passos (nmax no Python)
//     // int kmax = 150;         // Iterações máximas por passo
//     // float tol = 1e-10f;     // Tolerância
//     // float deltal0 = 1.2f;   // Comprimento de arco inicial
//     // int kd = 5;             // Iterações desejadas para ajuste do passo

//     // // Propriedades do material/seção
//     // float modElast = 5e7f;  // E0
//     // float area = 1.0f;      // A
//     // // Usamos uma inércia minúscula para simular uma treliça (sem resistência à flexão)
//     // float inercia = 1e-12f; 
//     // float espessura = 4.0f; // Espessura para o desenho

//     // // Geometria inicial
//     // float L0_x = 2500.0f;
//     // float z_y = 25.0f;
    
//     // // Carga de referência (negativa para baixo, conforme Python)
//     // float Fr = -7.0f;

//     // // 1. Criando os Nós
//     // // Nó 0: Base (0, 0) - Rotulado (Fixo em X e Y, Livre para girar em Z)
//     // est.adicionarNo({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false});

//     // // Nó 1: Ponta (2500, 25) - Fixo em X, Livre em Y (onde aplica a carga), Livre em Z
//     // // Como queremos que ele desça verticalmente, fixoX = true
//     // est.adicionarNo({L0_x, z_y, 0.0f, Fr, 0.0f, true, false, false});

//     // // 2. Criando a Barra
//     // est.adicionarBarra(est.nos[0], est.nos[1], est.nos[0].id, est.nos[1].id, modElast, area, inercia, espessura);

//     // // 3. Monitoramento
//     // int idNoMonitorado = est.nos[1].id; // Monitorando o nó da ponta
//     // int grauLiberdade = 1; // 1 representa o deslocamento vertical (Y)


//     // est.historicoDeslocamentos.clear();
//     // est.historicoDeslocamentos.push_back({0.0f, 0.0f});

//     // auto startLinear = std::chrono::high_resolution_clock::now();
//     // est.resolverSistema();
//     // auto endLinear = std::chrono::high_resolution_clock::now();

//     // std::chrono::duration<double> durationLinear = endLinear - startLinear;
//     // std::cout << "Tempo de resolução do sistema linear: " << durationLinear.count() << " ms." << std::endl;

//     // int indiceGlobal = idNoMonitorado * 3 + grauLiberdade;
//     // float uFinal = est.d(indiceGlobal);

//     // est.historicoDeslocamentos.push_back({std::abs(uFinal), std::abs(Fr)});

//     // est.montarMatrizRigidezeForcasInternas(desl);

//     // est.resolverSistemaNaoLinear(passos, maxIter, tol, deslocMax, idNoMonitorado, 1, Pmax);
//     // est.resolverSistema();

//     // auto startDenso = std::chrono::high_resolution_clock::now();
//     // est.resolverSistema();
//     // auto endDenso = std::chrono::high_resolution_clock::now();
//     // std::chrono::duration<double> durationDenso = endDenso - startDenso;
//     // std::cout << "Tempo de resolução do sistema denso: " << durationDenso.count() << " ms." << std::endl;

//     // auto startEsparso = std::chrono::high_resolution_clock::now();
//     // est.resolverSistemaEsparsa();
//     // auto endEsparso = std::chrono::high_resolution_clock::now();
//     // std::chrono::duration<double> durationEsparso = endEsparso - startEsparso;
//     // std::cout << "Tempo de resolução do sistema esparso: " << durationEsparso.count() << " ms." << std::endl;

//     // auto startNaolinear = std::chrono::high_resolution_clock::now();
//     // est.resolverSistemaNaoLinear(passos, maxIter, tol, deslocMax, idNoMonitorado, 1, Pmax);
//     // auto endNaoLinear = std::chrono::high_resolution_clock::now();
//     // std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
//     // std::cout << "Tempo de resolução da analise nao linear: " << durationNaoLinear.count() << " s." << std::endl;

//     // =========================================================================
//     // IMPLEMENTAÇÃO: PÓRTICO DE WILLIAMS (SNAP-THROUGH)
//     // =========================================================================

//     // Parâmetros numéricos do solver (Método do Comprimento de Arco)
//     int nmax = 33;          // Número de passos (nmax no Python)
//     int kmax = 50;          // Iterações máximas por passo
//     float tol = 0.5;      // Tolerância para o resíduo
//     float deltal0 = 0.025f; // Comprimento de arco inicial
//     int kd = 5;             // Iterações desejadas (Nd no Python) para ajuste do passo

//     // Propriedades equivalentes (E = 1.0 no Python)
//     float modElast = 1.0f;
//     float area = 1.885e6f;    // Corresponde a EA
//     float inercia = 9.274e3f; // Corresponde a EI
//     float espessura = 4.0f;   // Espessura para renderização

//     // Coordenadas exatas do Pórtico
//     std::vector<std::pair<float, float>> coords = {
//         {0.0f, 0.0f}, {2.5872f, 0.0736f}, {5.1744f, 0.1472f}, {7.7616f, 0.2208f},
//         {10.3488f, 0.2944f}, {12.936f, 0.368f}, {15.5232f, 0.2944f}, {18.1104f, 0.2208f},
//         {20.6976f, 0.1472f}, {23.2848f, 0.0736f}, {25.872f, 0.0f}
//     };

//     // 1. Criando os Nós
//     for (size_t i = 0; i < coords.size(); i++)
//     {
//         // Engaste perfeito no primeiro e no último nó
//         bool engaste = (i == 0 || i == coords.size() - 1); 
        
//         // Força de referência (Fr = -1.0) aplicada no ápice (Nó central, índice 5)
//         float fy = (i == 5) ? -1.0f : 0.0f;

//         est.adicionarNo({coords[i].first, coords[i].second, 0.0f, fy, 0.0f, engaste, engaste, engaste});
//     }

//     // 2. Criando as Barras
//     for (size_t i = 0; i < est.nos.size() - 1; i++)
//     {
//         est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
//     }

//     // 3. Monitoramento
//     int idNoMonitorado = est.nos[5].id; // Monitorando o nó do ápice
//     int grauLiberdade = 1; // 1 representa o deslocamento vertical (Y)

//     auto startNaolinear = std::chrono::high_resolution_clock::now();
    
//     // ATENÇÃO: Use a função resolverSistemaNaoLinearArco que adaptamos!
//     // Se você ainda não implementou, substitua o nome pela sua função atual, 
//     // mas lembre-se que o Arc-Length é obrigatório para passar do ponto limite.
//     est.resolverSistemaNaoLinearArco(nmax, kmax, tol, deltal0, kd, idNoMonitorado, grauLiberdade);
    
//     auto endNaoLinear = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
//     std::cout << "Tempo de resolução da analise nao linear: " << durationNaoLinear.count() << " s." << std::endl;

//     float escalaVisualizacao = 0.0f;

//     bool solver = true;

//     if (solver)
//     {
//         escalaVisualizacao = 1.0f;
//     }
//     else
//     {
//         escalaVisualizacao = 2000.0f;
//     }

//     est.calcularPontosDeformadaEstrutura(escalaVisualizacao);

//     // std::cout << "Reaçoes" << std::endl;
//     // std::cout << est.R << std::endl;

//     // est.resolverSistemaEsparsa();
//     // est.calcularPontosDeformadaEstrutura(10e1);

//     while (!WindowShouldClose())
//     {
//         { // CAMERA
//             if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
//             {
//                 Vector2 delta = GetMouseDelta();
//                 delta = Vector2Scale(delta, -1.0f / camera.zoom);
//                 camera.target = Vector2Add(camera.target, delta);
//             }
//             {
//                 // Zoom based on mouse wheel
//                 float wheel = GetMouseWheelMove();
//                 if (wheel != 0)
//                 {
//                     // Get the world point that is under the mouse
//                     Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

//                     // Set the offset to where the mouse is
//                     camera.offset = GetMousePosition();

//                     // Set the target to match, so that the camera maps the world space point
//                     // under the cursor to the screen space point under the cursor at any zoom
//                     camera.target = mouseWorldPos;

//                     // Zoom increment
//                     // Uses log scaling to provide consistent zoom speed
//                     float scale = 0.2f * wheel;
//                     camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 640.0f);
//                 }
//             }
//         }

//         // float tempoAtual = (float)GetTime();

//         // float fatorSenoidal = sinf(tempoAtual * velocidadeAnimacao);

//         // float escalaAnimada = escalaMaxima * fatorSenoidal;

//         BeginDrawing();
//             ClearBackground({40, 40, 40, 255});
//             BeginMode2D(camera);
//                 DrawLineEx({ -(float)GetScreenWidth()/2, 0.0f}, { (float)GetScreenWidth()/2, 0.0f}, 3.0/camera.zoom, { 30, 30, 30, 255});
//                 DrawLineEx({0.0f, -(float)GetScreenWidth()/2}, {0.0f, (float)GetScreenWidth()/2}, 3.0/camera.zoom, { 30, 30, 30, 255});

//                 //camada controladora do desenho 
//                 renderizador.desenhaEstrutura(est, camera);

//                 // if (solver) renderizador.desenhaReacoes(est, camera);

//                 if (IsKeyDown(KEY_SPACE))
//                 {
//                     if (solver) renderizador.desenhaDeformada(est, RED, camera);
//                 }
                
//                 if (IsKeyDown(KEY_D))
//                 {
//                     if (solver) renderizador.desenhaPontoDeformada(est, escalaVisualizacao, camera.zoom);
//                 }
            
//                 if (solver) renderizador.desenhaPontoDeformada(est, escalaVisualizacao, camera.zoom);
//                 //renderizador.desenhaDeformadaAnimada(est, escalaAnimada, RED, camera);

//             EndMode2D();

//             // Rectangle areaGrafico = { screenWidth - 320, 20, 300, 200 };
//             // renderizador.desenhaGraficoPxU(est.historicoDeslocamentos, areaGrafico, "Curva P x u");

//             rlImGuiBegin();
            
//                 std::vector<float> xData, yData;

//                 for (const auto& par : est.historicoDeslocamentos)
//                 {
//                     xData.push_back(par.first);
//                     yData.push_back(par.second);
//                 }

//                 ImGui::Begin("Analise de Resultados");

//                 if (ImGui::CollapsingHeader("Curva P x u", ImGuiTreeNodeFlags_DefaultOpen))
//                 {
//                     if (ImPlot::BeginPlot("##CurvaPxU", ImVec2(-1, 300)))
//                     {
//                         ImPlot::SetupAxes("Deslocamento u (m)", "Carga P (N)");

//                         if (!xData.empty() && !yData.empty())
//                         {
//                             ImPlot::PlotLine("P x u", xData.data(), yData.data(), (int)xData.size());
//                             ImPlot::PlotScatter("Pontos", xData.data(), yData.data(), (int)xData.size());
//                         }

//                         ImPlot::EndPlot();
//                     }
//                 }

//                 if (ImGui::CollapsingHeader("Performance do Solver")) 
//                 {
//                     // ImGui::Text("Tempo Denso: %.4f ms", durationDenso.count());
//                     // ImGui::Text("Tempo Esparso: %.4f ms", durationEsparso.count());
//                     // ImGui::Text("Tempo Não-Linear: %.4f s", durationNaoLinear.count());
//                 }

//                 ImGui::End();

//             rlImGuiEnd();

//             DrawFPS(10, 10);

//         EndDrawing();

//     }

//     // std::cout << "Fazer deformada" << std::endl;
//     // std::cout << "Fazer reações de apoio" << std::endl;
//     // std::cout << "Ver tabelas de carregamentos padrões" << std::endl;
//     ImPlot::DestroyContext();
//     rlImGuiShutdown();
//     CloseWindow();
//     return 0;
// }

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

    // // =========================================================================
    // // IMPLEMENTAÇÃO: PÓRTICO DE WILLIAMS (ARC-LENGTH / COMPRIMENTO DE ARCO)
    // // =========================================================================

    // // Parâmetros numéricos do solver (Método do Comprimento de Arco)
    // int nmax = 33;          // Número total de passos
    // int kmax = 50;          // Iterações máximas por passo (para o corretor)
    // float tol = 1e-6f;      // Tolerância para o resíduo
    // float deltal0 = 0.025f; // Comprimento de arco inicial
    // int kd = 5;             // Número de iterações desejadas (ajusta o passo)

    // // Propriedades equivalentes (E = 1.0 no Python)
    // float modElast = 1.0f;
    // float area = 1.885e6f;    
    // float inercia = 9.274e3f; 
    // float espessura = 6.0f;   

    // // Coordenadas exatas do Pórtico
    // std::vector<std::pair<float, float>> coords = {
    //     {0.0f, 0.0f}, {2.5872f, 0.0736f}, {5.1744f, 0.1472f}, {7.7616f, 0.2208f},
    //     {10.3488f, 0.2944f}, {12.936f, 0.368f}, {15.5232f, 0.2944f}, {18.1104f, 0.2208f},
    //     {20.6976f, 0.1472f}, {23.2848f, 0.0736f}, {25.872f, 0.0f}
    // };

    // // 1. Criando os Nós
    // for (size_t i = 0; i < coords.size(); i++)
    // {
    //     bool engaste = (i == 0 || i == coords.size() - 1); 
        
    //     // No Arc-Length, a força de referência F_ref é aplicada diretamente no nó.
    //     // O algoritmo vai multiplicar essa referência pelo fator de carga (lambda).
    //     float fy = (i == 5) ? -1.0f : 0.0f; 
        
    //     est.adicionarNo({coords[i].first, coords[i].second, 0.0f, fy, 0.0f, engaste, engaste, engaste});
    // }

    // // 2. Criando as Barras
    // for (size_t i = 0; i < est.nos.size() - 1; i++)
    // {
    //     est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
    // }

    // // 3. Monitoramento
    // int idNoMonitorado = est.nos[5].id; // Monitorando o nó do ápice (índice 5)
    // int grauLiberdade = 1;              // 1 representa o deslocamento vertical (Y)

    // // =========================================================================
    // // PROCESSAMENTO DO SOLVER
    // // =========================================================================
    // auto startNaolinear = std::chrono::high_resolution_clock::now();
    
    // // Chamada do Solver Arc-Length
    // est.resolverSistemaNaoLinearArco(nmax, kmax, tol, deltal0, kd, idNoMonitorado, grauLiberdade);
    
    // auto endNaoLinear = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
    
    // // A escala visual é 1.0 para manter as proporções reais da estrutura
    // float escalaVisualizacao = 1.0f;
    // est.calcularPontosDeformadaEstrutura(escalaVisualizacao);


    // =========================================================================
    // IMPLEMENTAÇÃO: PÓRTICO DE WILLIAMS (CONTROLE DE CARGA / NR PADRÃO)
    // Equivalente ao portico2dNLG (1).py
    // =========================================================================

    // Parâmetros idênticos ao Python
    int passos = 20;            // num_passos = 20
    int maxIter = 50;           // kmax = 50
    float tol = 1e-6f;          // tol = 1e-6
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
    est.resolverSistemaNaoLinear(passos, maxIter, tol, deslocMax, idNoMonitorado, grauLiberdade, forcaTotal);
    
    auto endNaoLinear = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
    
    float escalaVisualizacao = 1.0f;
    est.calcularPontosDeformadaEstrutura(escalaVisualizacao);


    // // =========================================================================
    // // IMPLEMENTAÇÃO: PÓRTICO DE WILLIAMS (SNAP-THROUGH)
    // // =========================================================================

    // // Parâmetros numéricos do solver (Método do Comprimento de Arco)
    // int nmax = 33;          // Número de passos (nmax no Python)
    // int kmax = 50;          // Iterações máximas por passo
    // float tol = 0.5;      // Tolerância para o resíduo
    // float deltal0 = 0.025f; // Comprimento de arco inicial
    // int kd = 5;             // Iterações desejadas (Nd no Python) para ajuste do passo

    // // Propriedades equivalentes (E = 1.0 no Python)
    // float modElast = 1.0f;
    // float area = 1.885e6f;    // Corresponde a EA
    // float inercia = 9.274e3f; // Corresponde a EI
    // float espessura = 4.0f;   // Espessura para renderização

    // // Coordenadas exatas do Pórtico
    // std::vector<std::pair<float, float>> coords = {
    //     {0.0f, 0.0f}, {2.5872f, 0.0736f}, {5.1744f, 0.1472f}, {7.7616f, 0.2208f},
    //     {10.3488f, 0.2944f}, {12.936f, 0.368f}, {15.5232f, 0.2944f}, {18.1104f, 0.2208f},
    //     {20.6976f, 0.1472f}, {23.2848f, 0.0736f}, {25.872f, 0.0f}
    // };

    // // 1. Criando os Nós
    // for (size_t i = 0; i < coords.size(); i++)
    // {
    //     // Engaste perfeito no primeiro e no último nó
    //     bool engaste = (i == 0 || i == coords.size() - 1); 
        
    //     // Força de referência (Fr = -1.0) aplicada no ápice (Nó central, índice 5)
    //     float fy = (i == 5) ? -1.0f : 0.0f;

    //     est.adicionarNo({coords[i].first, coords[i].second, 0.0f, fy, 0.0f, engaste, engaste, engaste});
    // }

    // // 2. Criando as Barras
    // for (size_t i = 0; i < est.nos.size() - 1; i++)
    // {
    //     est.adicionarBarra(est.nos[i], est.nos[i+1], est.nos[i].id, est.nos[i+1].id, modElast, area, inercia, espessura);
    // }

    // // 3. Monitoramento
    // int idNoMonitorado = est.nos[5].id; // Monitorando o nó do ápice
    // int grauLiberdade = 1; // 1 representa o deslocamento vertical (Y)

    // auto startNaolinear = std::chrono::high_resolution_clock::now();
    
    // // ATENÇÃO: Use a função resolverSistemaNaoLinearArco que adaptamos!
    // // Se você ainda não implementou, substitua o nome pela sua função atual, 
    // // mas lembre-se que o Arc-Length é obrigatório para passar do ponto limite.
    // est.resolverSistemaNaoLinearArco(nmax, kmax, tol, deltal0, kd, idNoMonitorado, grauLiberdade);
    
    // auto endNaoLinear = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> durationNaoLinear = endNaoLinear - startNaolinear;
    // std::cout << "Tempo de resolução da analise nao linear: " << durationNaoLinear.count() << " s." << std::endl;

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
            ImGui::Text("Nós: %d | Barras: %d", (int)est.nos.size(), (int)est.barras.size());
            ImGui::Separator();
            ImGui::Text("Método: Arc-Length (Comprimento de Arco)");
            ImGui::Text("Passos realizados: %d", (int)est.historicoDeslocamentos.size() - 1);
            ImGui::Text("Tolerância: %.1e", tol);
            ImGui::Separator();
            ImGui::Text("Tempo do Solver: %.4f s", durationNaoLinear.count());
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✅ Curva Snap-Through capturada com sucesso!");
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