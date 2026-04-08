#pragma once

#include <vector>
#include <array>

#include "raylib.h"
#include "eigenpch.h"

class No
{
public:
    float x;
    float y;
    float fx;
    float fy;
    float mz;
    bool fixoX;
    bool fixoY;
    bool rotaZ;
    int id;

    static int nextid;

    No() = default;
    No(float x_, float y_);
    No(float x_, float y_, float fx_, float fy_, float mz_, bool fixoX_, bool fixoY_, bool rotaZ_);
};

class Barra
{
public:
    No noi;
    No nof;
    int noInicialId;
    int noFinalId;
    float comprimento;
    float comprimentoInicial;
    float modElast;
    float area;
    float inercia;
    float esp;
    float cos;
    float sen;
    Eigen::Matrix<float, 6, 6> kLocal; // matriz de rigidez local
    Eigen::Matrix<float, 6, 6> KGlobal; // matriz de rigidez global
    Eigen::Matrix<float, 6, 6> T; // matriz de transformação
    // Eigen::VectorXf vGlobal; // vetor de deslocamentos globais
    // Eigen::VectorXf Fglobal; // vetor de forças globais
    // Eigen::VectorXf uLocal; // vetor de deslocamentos locais
    // Eigen::VectorXf fLocal; // vetor de forças locais

    Eigen::Matrix<float, 6, 1> vGlobal; // vetor de deslocamentos globais
    Eigen::Matrix<float, 6, 1> Fglobal; // vetor de forças globais
    Eigen::Matrix<float, 6, 1> uLocal; // vetor de deslocamentos locais
    Eigen::Matrix<float, 6, 1> fLocal; // vetor de forças locais
    std::vector<std::pair<float, float>> pontosDeformada; // vetor com os pontos da deformada

    Barra() = default;
    // Barra(No noi_, No nof_, float modElast_, float area_, float inercia_, float esp_);
    Barra(const No& noi_, const No& nof_, float modElast_, float area_, float inercia_, float esp_);

    void calculaMatrizRigidezLocal();
    void calcularMatrizTransformacao();
    void calcularMatrizTransformacaoNaoLinear();
    void calcularDeslocamentosGlobais(const Eigen::VectorXf& d, const std::array<int, 6>& bcn);
    void calcularForcasGlobais();
    void calcularEsforcosLocais();
    void calculaDeformadaLocal(float fatorEscala);
};

class Estrutura
{
public:
    std::vector<No> nos;
    // possível uso de estrutura chave-valor para os nós
    std::map<int, No> mapaNos;
    std::vector<std::array<int, 2>> conexoes;
    std::vector<Barra> barras;
    std::vector<std::array<int, 6>> BCN;
    Eigen::MatrixXf S;
    Eigen::SparseMatrix<float> Sesparsa;
    Eigen::VectorXf P;
    Eigen::VectorXf Pu;
    Eigen::VectorXf d;
    Eigen::VectorXf R;
    Eigen::VectorXf Fint;
    Eigen::VectorXf Residuo;
    std::vector<std::pair<float, float>> historicoDeslocamentos;

    Estrutura() = default;
    // Estrutura(std::vector<No> nos_, std::vector<std::array<int, 2>> conexoes_);

    void adicionarNo(const No& no);
    void adicionarBarra(No noi_, No nof_, int noiId, int nofId, float modElast_, float area_, float inercia_, float esp_);
    const No& getNoById(int id) const;

    void montarBCN();
    void calcularMatrizRigidezEstrutura();
    void montarVetorForcas();
    void aplicarCondicoesDeContorno();
    void calcularPontosDeformadaEstrutura(float fatorEscala);
    void resolverSistema();
    void montarMatrizRigidezeForcasInternas(Eigen::VectorXf d);
    void montarMatrizTangente(Eigen::VectorXf d);
    void resolverSistemaNaoLinear(int passos, int maxIteracoes, float tolerancia, float deslocamentoMaximo, 
    int noMonitoradoId, int grauLiberdade, float cargaTotalRef);
    void resolverSistemaNaoLinearArco(int nmax, int kmax, float tol, float delta0, int kd, int noMonitoradoId, int grauLiberdade);

    void calcularMatrizRigidezEstruturaEsparsa();
    void aplicarCondicoesDeContornoEsparsa();
    void resolverSistemaEsparsa();
};