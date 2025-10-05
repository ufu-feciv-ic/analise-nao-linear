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
    float modElast;
    float area;
    float inercia;
    float esp;
    Eigen::Matrix<float, 6, 6> kLocal;
    Eigen::Matrix<float, 6, 6> KGlobal;
    Eigen::Matrix<float, 6, 6> T;

    Barra() = default;
    // Barra(No noi_, No nof_, float modElast_, float area_, float inercia_, float esp_);
    Barra(const No& noi_, const No& nof_, float modElast_, float area_, float inercia_, float esp_);

    void calculaMatrizRigidezLocal();
    void calcularMatrizTransformacao();
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
    Eigen::MatrixXd S;
    
    Estrutura() = default;
    Estrutura(std::vector<No> nos_, std::vector<std::array<int, 2>> conexoes_);

    void adicionarNo(const No& no);
    void adicionarBarra(int noiId, int nofId, float modElast_, float area_, float inercia_, float esp_);
    const No& getNoById(int id) const;

    void montarBCN();
    void calcularMatrizRigidezEstrutura();
};