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
    void desenhaDeformada(const Estrutura& est, Color cor, Camera2D camera);
    void desenhaDeformadaAnimada(Estrutura est, float fatorEscala, Color cor, Camera2D camera);
    void desenhaPontoDeformada(Estrutura est, float zoom);

private:
    void desenhaNo(const No& no, Camera2D camera);
    void desenhaPonto(const No& no, float zoom);
    void desenhaIndice(const No& no, float zoom);
    void desenhaApoios(const No& no, float zoom);
    void desenhaBarras(const Estrutura& est, Camera2D camera);
    void desenhaReacoes(const Estrutura& est, Camera2D camera);
    
};