#pragma once

#include "raylib.h"
#include "Estrutura.h"
#include "DesenhoUtils.h"
#include <cmath>
#include <algorithm> // para std::max
#include <string>
#include <cstdio> // para sprintf ou use TextFormat do Raylib

class RenderizadorEstrutura
{
public:
    RenderizadorEstrutura() = default;

    void desenhaEstrutura(const Estrutura& est, Camera2D camera);
    void desenhaDeformada(const Estrutura& est, Color cor, Camera2D camera);
    void desenhaDeformadaAnimada(Estrutura est, float fatorEscala, Color cor, Camera2D camera);
    void desenhaPontoDeformada(Estrutura est, float fatorEscala, float zoom);
    void desenhaReacoes(const Estrutura& est, Camera2D camera);
    void desenhaGraficoPxU(const std::vector<std::pair<float, float>>& dados, Rectangle area, const char* titulo);

private:
    void desenhaNo(const No& no, Camera2D camera);
    void desenhaPonto(const No& no, float zoom);
    void desenhaIndice(const No& no, float zoom);
    void desenhaApoios(const No& no, float zoom);
    void desenhaBarras(const Estrutura& est, Camera2D camera);
};