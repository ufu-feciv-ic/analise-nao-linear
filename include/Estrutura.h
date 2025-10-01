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
    Camera2D camera;

    static int nextid;

    No() = default;
    No(float x_, float y_, Camera2D camera_);
    No(float x_, float y_, float fx_, float fy_, float mz_, bool fixoX_, bool fixoY_, bool rotaZ_, Camera2D camera_);

    void drawNo(Camera2D camera);
    void drawPonto(float zoom);
    void drawIndice(float zoom);
    void drawApoios(float zoom);
};

class Barra
{
public:
    No noi;
    No nof;
    float comprimento;
    float modElast;
    float area;
    float inercia;
    float esp;
    Eigen::Matrix<float, 6, 6> kLocal;

    Barra() = default;
    Barra(No noi_, No nof_, float modElast_, float area_, float inercia_, float esp_);

    void draw(Camera2D camera);
    void calculaMatrizRigidezLocal();
};

class Estrutura
{
public:
    std::vector<No> nos;
    std::vector<std::array<int, 2>> conexoes;
    std::vector<Barra> barras;

    Estrutura() = default;
    Estrutura(std::vector<No> nos_, std::vector<std::array<int, 2>> conexoes_);

    void drawNos(Camera2D camera);
    void drawBarras(Camera2D camera);
    void draw(Camera2D camera);
};