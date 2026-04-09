#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

// Raylib, ImGui e ImPlot
#include <raylib.h>
#include "imgui.h"
#include "implot.h"
#include "rlImGui.h" 

// Eigen
// #include <Eigen/Dense>
#include "eigenpch.h"

// --- ESTRUTURAS DE DADOS ---

struct ModelData {
    int NTEL, NTNOS, NTGL, NNOSCC;
    Eigen::MatrixXd coord;
    Eigen::MatrixXi inci;
    Eigen::MatrixXi dofno;
    Eigen::VectorXd E, A, I, Fr;
    std::vector<int> NOCC;
};

struct StepResult {
    Eigen::VectorXd udesl;     
    double lambda;      
    double u_apex;      
    double f_apex;      
};

enum class CalcType { Force, Stiffness };

// --- SUB-ROTINAS MATEMÁTICAS ---

double normalize_angle(double angle) {
    double a = std::fmod(angle + M_PI, 2.0 * M_PI);
    if (a < 0.0) a += 2.0 * M_PI;
    return a - M_PI;
}

ModelData krenk(double P_val) {
    ModelData md;
    md.NTNOS = 11;
    md.NTEL = 10;
    md.NTGL = md.NTNOS * 3;
    md.NNOSCC = 6;

    md.coord = Eigen::MatrixXd(md.NTNOS, 2);
    md.coord << 0.0, 0.0,
                2.5872, 0.0736,
                5.1744, 0.1472,
                7.7616, 0.2208,
                10.3488, 0.2944,
                12.936, 0.368,   
                15.5232, 0.2944,
                18.1104, 0.2208,
                20.6976, 0.1472,
                23.2848, 0.0736,
                25.872, 0.0;

    md.inci = Eigen::MatrixXi(md.NTEL, 2);
    md.inci << 0, 1,   1, 2,   2, 3,   3, 4,   4, 5,
               5, 6,   6, 7,   7, 8,   8, 9,   9, 10;

    md.E = Eigen::VectorXd::Constant(md.NTEL, 1.0);
    md.A = Eigen::VectorXd::Constant(md.NTEL, 1.885e6);
    md.I = Eigen::VectorXd::Constant(md.NTEL, 9.274e3);

    md.dofno = Eigen::MatrixXi(md.NTEL, 6);
    for (int i = 0; i < md.NTEL; ++i) {
        int no1 = md.inci(i, 0);
        int no2 = md.inci(i, 1);
        md.dofno(i, 0) = no1 * 3;     md.dofno(i, 1) = no1 * 3 + 1; md.dofno(i, 2) = no1 * 3 + 2;
        md.dofno(i, 3) = no2 * 3;     md.dofno(i, 4) = no2 * 3 + 1; md.dofno(i, 5) = no2 * 3 + 2;
    }

    md.Fr = Eigen::VectorXd::Zero(md.NTGL);
    md.Fr(16) = P_val; 

    md.NOCC = {0, 1, 2, 30, 31, 32}; 

    return md;
}

void element_routine(const Eigen::VectorXd& udesl, const Eigen::MatrixXd& coord0, const ModelData& md, 
                     CalcType calc_type, Eigen::VectorXd& out_force, Eigen::MatrixXd& out_stiffness) {
    
    if (calc_type == CalcType::Force) {
        out_force = Eigen::VectorXd::Zero(md.NTGL);
    } else {
        out_stiffness = Eigen::MatrixXd::Zero(md.NTGL, md.NTGL);
    }

    for (int m = 0; m < md.NTEL; ++m) {
        int no1 = md.inci(m, 0);
        int no2 = md.inci(m, 1);
        double X1 = coord0(no1, 0); double Y1 = coord0(no1, 1);
        double X2 = coord0(no2, 0); double Y2 = coord0(no2, 1);

        Eigen::Vector<double, 6> u_el;
        for (int i = 0; i < 6; ++i) u_el(i) = udesl(md.dofno(m, i));

        double L0 = std::sqrt(std::pow(X2 - X1, 2) + std::pow(Y2 - Y1, 2));

        double dx = (X2 + u_el(3)) - (X1 + u_el(0));
        double dy = (Y2 + u_el(4)) - (Y1 + u_el(1));
        double L = std::sqrt(dx * dx + dy * dy);
        if (L < 1e-12) L = 1e-12;

        double C = dx / L;
        double S = dy / L;

        double beta0 = std::atan2(Y2 - Y1, X2 - X1);
        double beta = std::atan2(dy, dx);

        double theta1_g = u_el(2);
        double theta2_g = u_el(5);

        double teta1 = normalize_angle(theta1_g + beta0 - beta);
        double teta2 = normalize_angle(theta2_g + beta0 - beta);

        double ul = (L * L - L0 * L0) / (L + L0);

        double Em = md.E(m); double Am = md.A(m); double Im = md.I(m);

        double N = Em * Am * ul / L0;
        double k_bend = 2.0 * Em * Im / L0;
        double M1 = k_bend * (2.0 * teta1 + teta2);
        double M2 = k_bend * (teta1 + 2.0 * teta2);

        Eigen::Matrix<double, 3, 6> B;
        B.row(0) << -C, -S, 0, C, S, 0;
        B.row(1) << -S/L, C/L, 1.0, S/L, -C/L, 0.0;
        B.row(2) << -S/L, C/L, 0.0, S/L, -C/L, 1.0;

        if (calc_type == CalcType::Force) {
            Eigen::Vector3d f_local(N, M1, M2);
            Eigen::Vector<double, 6> Fel = B.transpose() * f_local;
            for (int i = 0; i < 6; ++i) {
                out_force(md.dofno(m, i)) += Fel(i);
            }
        } else {
            Eigen::Matrix3d D;
            D << Em*Am/L0, 0, 0,
                 0, 4.0*Em*Im/L0, 2.0*Em*Im/L0,
                 0, 2.0*Em*Im/L0, 4.0*Em*Im/L0;

            Eigen::Matrix<double, 6, 6> KM = B.transpose() * D * B;

            Eigen::Vector<double, 6> r_vec, z_vec;
            r_vec << -C, -S, 0, C, S, 0;
            z_vec << S, -C, 0, -S, C, 0;

            Eigen::Matrix<double, 6, 6> K1 = (N / L) * (z_vec * z_vec.transpose());
            Eigen::Matrix<double, 6, 6> K2 = ((M1 + M2) / (L * L)) * (r_vec * z_vec.transpose() + z_vec * r_vec.transpose());

            Eigen::Matrix<double, 6, 6> Kel = KM + K1 + K2;

            for (int i = 0; i < 6; ++i) {
                for (int j = 0; j < 6; ++j) {
                    out_stiffness(md.dofno(m, i), md.dofno(m, j)) += Kel(i, j);
                }
            }
        }
    }
}

void apply_bc(Eigen::MatrixXd* K, Eigen::VectorXd* R, const std::vector<int>& NOCC) {
    for (int dof : NOCC) {
        if (K != nullptr) {
            K->row(dof).setZero();
            K->col(dof).setZero();
            (*K)(dof, dof) = 1.0;
        }
        if (R != nullptr) {
            (*R)(dof) = 0.0;
        }
    }
}

// --- SIMULAÇÃO ---

std::vector<StepResult> run_simulation(ModelData& md, double P_inc, int nmax) {
    std::vector<StepResult> history;
    
    double tol = 1e-6; int kmax = 50;
    double deltal0 = 0.025; double Nd = 5.0;
    
    Eigen::VectorXd udesl = Eigen::VectorXd::Zero(md.NTGL);
    Eigen::VectorXd DELTAU = Eigen::VectorXd::Zero(md.NTGL);
    double lambda_val = 0.0;
    double deltal = deltal0;
    Eigen::MatrixXd coord0 = md.coord;
    
    double h_apex = 0.368;
    
    // Passo 0
    history.push_back({udesl, 0.0, 0.0, 0.0});

    Eigen::VectorXd out_force;
    Eigen::MatrixXd K;

    for (int np_step = 1; np_step <= nmax; ++np_step) {
        
        // --- Preditor ---
        element_routine(udesl, coord0, md, CalcType::Stiffness, out_force, K);
        apply_bc(&K, nullptr, md.NOCC);

        Eigen::VectorXd deltaur = K.colPivHouseholderQr().solve(md.Fr);

        double norm_dur = deltaur.norm();
        if (norm_dur == 0) norm_dur = 1e-10;
        double Dlambda = deltal / norm_dur;

        if (np_step > 1) {
            if (DELTAU.dot(deltaur) < 0) {
                Dlambda = -Dlambda;
            }
        }

        Eigen::VectorXd deltau = Dlambda * deltaur;
        Eigen::VectorXd DELTAU0 = deltau;
        DELTAU = deltau;

        double lambda_temp = lambda_val + Dlambda;

        // --- Corretor ---
        element_routine(udesl + DELTAU, coord0, md, CalcType::Force, out_force, K);
        Eigen::VectorXd g = (lambda_temp * md.Fr) - out_force;
        apply_bc(nullptr, &g, md.NOCC);

        int k = 0;
        while (k < kmax) {
            k++;

            element_routine(udesl + DELTAU, coord0, md, CalcType::Stiffness, out_force, K);
            apply_bc(&K, nullptr, md.NOCC);

            Eigen::VectorXd deltaug = K.colPivHouseholderQr().solve(g);
            deltaur = K.colPivHouseholderQr().solve(md.Fr);

            double top = DELTAU0.dot(deltaug);
            double bot = DELTAU0.dot(deltaur);

            double dlambda = (std::abs(bot) < 1e-12) ? 0.0 : -top / bot;

            DELTAU += (deltaug + dlambda * deltaur);
            Dlambda += dlambda;
            lambda_temp = lambda_val + Dlambda;

            element_routine(udesl + DELTAU, coord0, md, CalcType::Force, out_force, K);
            g = (lambda_temp * md.Fr) - out_force;
            apply_bc(nullptr, &g, md.NOCC);

            if (g.norm() <= tol) {
                break;
            }
        }

        udesl += DELTAU;
        lambda_val = lambda_temp;

        if (k > 0) deltal = deltal0 * std::sqrt(Nd / k);

        double u_y_apex = udesl(16); 
        
        StepResult res;
        res.udesl = udesl;
        res.lambda = lambda_val;
        res.u_apex = -u_y_apex / h_apex;
        res.f_apex = -lambda_val * P_inc;
        
        history.push_back(res);

        // Imprime o passo, o deslocamento normalizado (v/h) e a força P no console
        std::cout << "Passo " << std::setw(2) << np_step 
                  << " | Desloc (v/h): " << std::fixed << std::setprecision(4) << std::setw(8) << res.u_apex 
                  << " | Força P: "      << std::fixed << std::setprecision(4) << std::setw(8) << res.f_apex 
                  << "\n";
    }
    return history;
}

// --- INTERFACE E RENDERIZAÇÃO ---

Vector2 WorldToScreen(double x, double y, float screenW, float screenH) {
    float scale = 35.0f; 
    float offsetX = 100.0f;
    float offsetY = screenH * 0.7f; 
    return { static_cast<float>(offsetX + x * scale), static_cast<float>(offsetY - y * scale) };
}

// ==============================================================================
// MAIN
// ==============================================================================

int main() {
    std::cout << "Calculando simulação (NLG)...\n";
    ModelData md = krenk(-1.0); 
    std::vector<StepResult> history = run_simulation(md, -1.0, 33);
    std::cout << "Simulação concluída. Abrindo UI...\n";

    // Preparar dados para o ImPlot 
    std::vector<double> plot_u, plot_f;
    for(const auto& step : history) {
        plot_u.push_back(step.u_apex);
        plot_f.push_back(step.f_apex);
    }

    // Setup Raylib
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Williams Frame - Raylib + ImGui + ImPlot");
    SetTargetFPS(60);

    // Setup ImGui & ImPlot
    rlImGuiSetup(true);
    ImPlot::CreateContext();

    int current_step = 0;
    int max_steps = history.size() - 1;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({ 40, 40, 45, 255 });

        // --- DESENHO DA ESTRUTURA (RAYLIB) ---
        StepResult& state = history[current_step];
        for (int i = 0; i < md.NTEL; i++) {
            int n1 = md.inci(i, 0); int n2 = md.inci(i, 1);
            
            // Pega as posições deformadas
            Vector2 p1 = WorldToScreen(md.coord(n1,0) + state.udesl(n1*3), md.coord(n1,1) + state.udesl(n1*3+1), GetScreenWidth(), GetScreenHeight());
            Vector2 p2 = WorldToScreen(md.coord(n2,0) + state.udesl(n2*3), md.coord(n2,1) + state.udesl(n2*3+1), GetScreenWidth(), GetScreenHeight());
            
            DrawLineEx(p1, p2, 3.5f, SKYBLUE);
            DrawCircleV(p1, 5.0f, WHITE);
            DrawCircleV(p2, 5.0f, WHITE);
        }

        // --- UI IMGUI & IMPLOT ---
        rlImGuiBegin();

        // Janela de Controle
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(350, 150), ImGuiCond_Once);
        ImGui::Begin("Painel de Controle");
        ImGui::Text("Pórtico de Williams - Análise Não Linear");
        ImGui::Separator();
        
        ImGui::SliderInt("Passo de Carga", &current_step, 0, max_steps);
        if (ImGui::Button("Anterior") && current_step > 0) current_step--;
        ImGui::SameLine();
        if (ImGui::Button("Próximo") && current_step < max_steps) current_step++;

        ImGui::Separator();
        ImGui::Text("Fator de Carga (Lambda): %.4f", state.lambda);
        ImGui::Text("Desl. Normalizado (v/h): %.4f", state.u_apex);
        ImGui::Text("Força P (lb): %.4f", state.f_apex);
        ImGui::End();

        // Janela do Gráfico com ImPlot
        ImGui::SetNextWindowPos(ImVec2(GetScreenWidth() - 620.0f, 20.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
        ImGui::Begin("Gráfico de Resposta P x v/h");
        
        if (ImPlot::BeginPlot("Curva de Equilíbrio", ImVec2(-1, -1))) {
            ImPlot::SetupAxes("Deslocamento Normalizado (v/h)", "Força P (lb)");
            ImPlot::SetupAxesLimits(0, 3.0, 0, 1.5);

            // Desenha a curva completa em cinza transparente
            ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.5f, 0.5f), 1.0f);
            ImPlot::PlotLine("Caminho Total", plot_u.data(), plot_f.data(), (int)plot_u.size());

            // Desenha o caminho percorrido até o passo atual em vermelho
            ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), 2.5f);
            ImPlot::PlotLine("Resposta Atual", plot_u.data(), plot_f.data(), current_step + 1);

            // Marcador no ponto atual
            double curr_u = state.u_apex;
            double curr_f = state.f_apex;
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f, ImVec4(1, 1, 0, 1), 1.0f, ImVec4(1, 1, 0, 1));
            ImPlot::PlotScatter("Ponto de Equilíbrio", &curr_u, &curr_f, 1);

            ImPlot::EndPlot();
        }
        ImGui::End();

        rlImGuiEnd();
        EndDrawing();
    }

    // Limpeza
    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}