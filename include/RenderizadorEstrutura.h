#pragma once

#include "raylib.h"
#include "Estrutura.h"
#include "DesenhoUtils.h"
#include <cmath>

class RenderizadorEstrutura
{
public:
    RenderizadorEstrutura() = default;

    void desenhaEstrutura(const Estrutura& est, Camera2D camera);
    void desenhaDeformada(const Estrutura& est, float escala, Color cor);

private:
    void desenhaNo(const No& no, Camera2D camera);
    void desenhaPonto(const No& no, float zoom);
    void desenhaIndice(const No& no, float zoom);
    void desenhaApoios(const No& no, float zoom);
    void desenhaBarras(const Estrutura& est, Camera2D camera);
};