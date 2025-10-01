#include "Estrutura.h"
#include "DesenhoUtils.h"
#include <iostream>
#include <cmath>

// Implementação da classe No
int No::nextid = 0;

No::No(float x_, float y_, Camera2D camera_) : x(x_), y(y_), fx(0), fy(0), mz(0), fixoX(false), fixoY(false), rotaZ(false), camera(camera_)
{
    id = nextid++;
}

No::No(float x_, float y_, float fx_, float fy_, float mz_, bool fixoX_, bool fixoY_, bool rotaZ_, Camera2D camera_) : x(x_), y(y_), fx(fx_), fy(fy_), mz(mz_), fixoX(fixoX_), fixoY(fixoY_), rotaZ(rotaZ_), camera(camera_)
{
    id = nextid++;
}

void No::drawNo(Camera2D camera)
{
    drawApoios(camera.zoom);
    drawPonto(camera.zoom);
    drawForca(x, y, fx, fy, camera.zoom);
    drawFixedSizeAnnotadedMoment({x, y}, mz, 18.0f, 10.0f, 6.0f, 3.0f, ORANGE, camera);
    drawIndice(camera.zoom);
}

void No::drawPonto(float zoom)
{
    DrawCircleV({x, -y}, 6 / zoom, {0, 235, 255, 255});
}

void No::drawIndice(float zoom)
{
    DrawTextEx(GetFontDefault(), TextFormat("%i", id), {x + (10.0f / zoom), -y + (10.0f / zoom)}, 20.0f / zoom, 2.0f, {0, 235, 255, 255});
}

void No::drawApoios(float zoom)
{
    float compt = 22 / zoom;
    float compq = 32 / zoom;
    float largurat = (tanf(35 * PI / 180) * compt);
    float linhas = 4;

    if (fixoX && fixoY && rotaZ) // Fixo em Tudo
    {
        DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
        float distlinhas = (compq - (2 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {x - (compq / 2) + (1 / zoom) + (i * distlinhas), y - (compq / 2) + (2 / zoom)};
            Vector2 posicaof = {x - (compq / 2) - (11 / zoom) + (i * distlinhas), y - (compq / 2) - (12 / zoom)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {x + (compq / 2) - (1 / zoom), y - (compq / 2) + (1 / zoom) + (i * distlinhas)};
            Vector2 posicaof = {x + (compq / 2) + (11 / zoom), y - (compq / 2) - (11 / zoom) + (i * distlinhas)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }

    if (fixoX && !fixoY && !rotaZ) // Fixo só em X
    {
        Vector2 pt1 = {0, 0};
        Vector2 pt2 = {-compt, largurat};
        Vector2 pt3 = {-compt, -largurat};
        pt1 = Vector2Add(pt1, {x, y});
        pt2 = Vector2Add(pt2, {x, y});
        pt3 = Vector2Add(pt3, {x, y});
        DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, {90, 90, 90, 255});
        DrawLineEx({-compt + x, -largurat - (10 / zoom) - y}, {-compt + x, largurat + (10 / zoom) - y}, 3.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({-compt + x - (7.0f / zoom), -(y + (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({-compt + x - (7.0f / zoom), -(y - (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({-compt + x - (14.0f / zoom), -largurat - (10 / zoom) - y}, {-compt + x - (14.0f / zoom), largurat + (10 / zoom) - y}, 3.0f / zoom, {90, 90, 90, 255});
    }

    if (!fixoX && fixoY && !rotaZ) // Fixo só em Y
    {
        Vector2 pt1 = {0, 0};
        Vector2 pt2 = {-largurat, -compt};
        Vector2 pt3 = {largurat, -compt};
        pt1 = Vector2Add(pt1, {x, y});
        pt2 = Vector2Add(pt2, {x, y});
        pt3 = Vector2Add(pt3, {x, y});
        DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, {90, 90, 90, 255});
        DrawLineEx({-largurat - (10 / zoom) + x, +compt - y}, {largurat + (10 / zoom) + x, +compt - y}, 3.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({x + (8.0f / zoom), compt - y + (7.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({x - (8.0f / zoom), compt - y + (7.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({-largurat - (10 / zoom) + x, compt - y + (14.0f / zoom)}, {largurat + (10 / zoom) + x, compt - y + (14.0f / zoom)}, 3.0f / zoom, {90, 90, 90, 255});
    }
    if (!fixoX && !fixoY && rotaZ) // Fixo só em Z
    {
        DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
    }

    if (fixoX && fixoY && !rotaZ) // Fixo em X e Y
    {
        Vector2 pt1 = {0, 0};
        Vector2 pt2 = {-largurat, -compt};
        Vector2 pt3 = {largurat, -compt};
        pt1 = Vector2Add(pt1, {x, y});
        pt2 = Vector2Add(pt2, {x, y});
        pt3 = Vector2Add(pt3, {x, y});
        DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, {90, 90, 90, 255});
        DrawLineEx({-largurat - (10 / zoom) + x, +compt - y}, {largurat + (10 / zoom) + x, +compt - y}, 3.0f / zoom, {90, 90, 90, 255});

        float distlinhas = (2 * largurat + (18 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {x - (largurat) - (8 / zoom) + (i * distlinhas), y - (compt)};
            Vector2 posicaof = {x - (largurat) - (18 / zoom) + (i * distlinhas), y - (compt) - (10 / zoom)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }

    if (fixoX && !fixoY && rotaZ) // Fixo em X e Z
    {
        DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
        DrawCircleV({x + (8.0f / zoom), (compq / 2) - y + (6.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({x - (8.0f / zoom), (compq / 2) - y + (6.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({-(compq / 2) - (10 / zoom) + x, (compq / 2) - y + (13.0f / zoom)}, {(compq / 2) + (10 / zoom) + x, (compq / 2) - y + (13.0f / zoom)}, 3.0f / zoom, {90, 90, 90, 255});

        float distlinhas = (compq + (18 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {x - (compq / 2) - (8 / zoom) + (i * distlinhas), y - (compq / 2) - (14 / zoom)};
            Vector2 posicaof = {x - (compq / 2) - (18 / zoom) + (i * distlinhas), y - (compq / 2) - (24 / zoom)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }

    if (!fixoX && fixoY && rotaZ) // Fixo em Y e Z
    {
        DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
        DrawCircleV({-(compq / 2) + x - (6.0f / zoom), -(y + (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({-(compq / 2) + x - (6.0f / zoom), -(y - (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({x - (compq / 2) - (13.0f / zoom), -(compq / 2) - (10 / zoom) - y}, {x - (compq / 2) - (13.0f / zoom), (compq / 2) + (10 / zoom) - y}, 3.0f / zoom, {90, 90, 90, 255});

        float distlinhas = (compq + (18 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {x - (compq / 2) - (13 / zoom), y - (compq / 2) - (9 / zoom) + (i * distlinhas)};
            Vector2 posicaof = {x - (compq / 2) - (23 / zoom), y - (compq / 2) - (19 / zoom) + (i * distlinhas)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }
}


// Implementação da classe Barra
Barra::Barra(No noi_, No nof_, float modElast_, float area_, float inercia_, float esp_) 
: noi(noi_), nof(nof_), modElast(modElast_), area(area_), inercia(inercia_), esp(esp_)
{
    comprimento = sqrt(pow(nof.x - noi.x, 2) + pow(nof.y - noi.y, 2));
    kLocal = Eigen::Matrix<float, 6, 6>::Zero();
    calculaMatrizRigidezLocal();

    std::cout << modElast << std::endl;
    std::cout << area << std::endl;
    std::cout << inercia << std::endl;
    std::cout << esp << std::endl;
    std::cout << "Comprimeto = " << comprimento << std::endl;
    std::cout << "Teste matriz kLocal = \n"
              << kLocal << std::endl;
}

void Barra::draw(Camera2D camera)
{
    DrawLineEx({noi.x, -noi.y}, {nof.x, -nof.y}, esp / camera.zoom, BLUE);
}

void Barra::calculaMatrizRigidezLocal()
{
    double EAL = modElast * area / comprimento;
    double EIL = modElast * inercia / comprimento;
    double EIL2 = modElast * inercia / (comprimento * comprimento);
    double EIL3 = modElast * inercia / (comprimento * comprimento * comprimento);

    kLocal << EAL, 0, 0, -EAL, 0, 0,
              0, 12*EIL3, 6*EIL2, 0, -12*EIL3, 6*EIL2, 
              0, 6*EIL2, 4*EIL, 0, -6*EIL2, 2*EIL,
              -EAL, 0, 0, EAL, 0, 0,
              0, -12*EIL3, -6*EIL2, 0, 12*EIL3, -6*EIL2,
              0, 6*EIL2, 2*EIL, 0, -6*EIL2, 4*EIL;
}

// Implementação da classe Estrutura
Estrutura::Estrutura (std::vector<No> nos_, std::vector<std::array<int, 2>> conexoes_) : nos(nos_), conexoes(conexoes_)
{
    float base = 0.1;
    float altura = 0.2;
    float area = base * altura;
    float inercia = (base * pow(altura, 3)) / 12.0f;
    float modElast = 2500E6;
    float esp = 6;

    for (size_t i = 0; i < conexoes.size(); i++)
    {
        barras.emplace_back(nos[conexoes[i][0]], nos[conexoes[i][1]], modElast, area, inercia, esp);
    }
}

void Estrutura::drawNos(Camera2D camera)
{
    for (auto &no : nos)
        no.drawNo(camera);
}

void Estrutura::drawBarras(Camera2D camera)
{
    for (auto &barra : barras)
        barra.draw(camera);
}

void Estrutura::draw(Camera2D camera)
{
    drawBarras(camera);
    drawNos(camera);
}