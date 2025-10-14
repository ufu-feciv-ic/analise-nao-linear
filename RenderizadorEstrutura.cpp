#include "RenderizadorEstrutura.h"

void RenderizadorEstrutura::desenhaEstrutura(const Estrutura& est, Camera2D camera)
{
    desenhaBarras(est, camera);

    for (const auto& no : est.nos)
    {
        desenhaNo(no, camera);
    }
}

void RenderizadorEstrutura::desenhaDeformada(const Estrutura &est, float escala, Color cor)
{
    if (est.d.size() == 0)
        return;
    
    for (const auto& barra : est.barras)
    {
        float dxi = barra.vGlobal(0);
        float dyi = barra.vGlobal(1);
        float dxf = barra.vGlobal(3);
        float dyf = barra.vGlobal(4);

        Vector2 posNoInicialDef = {barra.noi.x + dxi * escala, barra.noi.y + dyi * escala};
        Vector2 posNoFinalDef = {barra.nof.x + dxf * escala, barra.nof.y + dyf * escala};

        DrawLineV(posNoInicialDef, posNoFinalDef, cor);
    }
}

void RenderizadorEstrutura::desenhaNo(const No& no, Camera2D camera)
{
    desenhaApoios(no, camera.zoom);
    desenhaPonto(no, camera.zoom);
    drawForca(no.x, no.y, no.fx, no.fy, camera.zoom);
    drawFixedSizeAnnotadedMoment({no.x, no.y}, no.mz, 18.0f, 10.0f, 6.0f, 3.0f, ORANGE, camera);
    desenhaIndice(no, camera.zoom);
}

void RenderizadorEstrutura::desenhaPonto(const No& no, float zoom)
{
    DrawCircleV({no.x, -no.y}, 6 / zoom, {0, 235, 255, 255});    
}

void RenderizadorEstrutura::desenhaIndice(const No& no, float zoom)
{
    DrawTextEx(GetFontDefault(), TextFormat("%i", no.id), {no.x + (10.0f / zoom), -no.y + (10.0f / zoom)}, 20.0f / zoom, 2.0f, {0, 235, 255, 255});
}

void RenderizadorEstrutura::desenhaApoios(const No& no, float zoom)
{
    float compt = 22 / zoom;
    float compq = 32 / zoom;
    float largurat = (tanf(35 * PI / 180) * compt);
    float linhas = 4;

    if (no.fixoX && no.fixoY && no.rotaZ) // Fixo em Tudo
    {
        DrawRectangleV({no.x - compq / 2, -no.y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
        float distlinhas = (compq - (2 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {no.x - (compq / 2) + (1 / zoom) + (i * distlinhas), no.y - (compq / 2) + (2 / zoom)};
            Vector2 posicaof = {no.x - (compq / 2) - (11 / zoom) + (i * distlinhas), no.y - (compq / 2) - (12 / zoom)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {no.x + (compq / 2) - (1 / zoom), no.y - (compq / 2) + (1 / zoom) + (i * distlinhas)};
            Vector2 posicaof = {no.x + (compq / 2) + (11 / zoom), no.y - (compq / 2) - (11 / zoom) + (i * distlinhas)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }

    if (no.fixoX && !no.fixoY && !no.rotaZ) // Fixo só em X
    {
        Vector2 pt1 = {0, 0};
        Vector2 pt2 = {-compt, largurat};
        Vector2 pt3 = {-compt, -largurat};
        pt1 = Vector2Add(pt1, {no.x, no.y});
        pt2 = Vector2Add(pt2, {no.x, no.y});
        pt3 = Vector2Add(pt3, {no.x, no.y});
        DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, {90, 90, 90, 255});
        DrawLineEx({-compt + no.x, -largurat - (10 / zoom) - no.y}, {-compt + no.x, largurat + (10 / zoom) - no.y}, 3.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({-compt + no.x - (7.0f / zoom), -(no.y + (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({-compt + no.x - (7.0f / zoom), -(no.y - (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({-compt + no.x - (14.0f / zoom), -largurat - (10 / zoom) - no.y}, {-compt + no.x - (14.0f / zoom), largurat + (10 / zoom) - no.y}, 3.0f / zoom, {90, 90, 90, 255});
    }

    if (!no.fixoX && no.fixoY && !no.rotaZ) // Fixo só em Y
    {
        Vector2 pt1 = {0, 0};
        Vector2 pt2 = {-largurat, -compt};
        Vector2 pt3 = {largurat, -compt};
        pt1 = Vector2Add(pt1, {no.x, no.y});
        pt2 = Vector2Add(pt2, {no.x, no.y});
        pt3 = Vector2Add(pt3, {no.x, no.y});
        DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, {90, 90, 90, 255});
        DrawLineEx({-largurat - (10 / zoom) + no.x, +compt - no.y}, {largurat + (10 / zoom) + no.x, +compt - no.y}, 3.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({no.x + (8.0f / zoom), compt - no.y + (7.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({no.x - (8.0f / zoom), compt - no.y + (7.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({-largurat - (10 / zoom) + no.x, compt - no.y + (14.0f / zoom)}, {largurat + (10 / zoom) + no.x, compt - no.y + (14.0f / zoom)}, 3.0f / zoom, {90, 90, 90, 255});
    }
    if (!no.fixoX && !no.fixoY && no.rotaZ) // Fixo só em Z
    {
        DrawRectangleV({no.x - compq / 2, -no.y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
    }

    if (no.fixoX && no.fixoY && !no.rotaZ) // Fixo em X e Y
    {
        Vector2 pt1 = {0, 0};
        Vector2 pt2 = {-largurat, -compt};
        Vector2 pt3 = {largurat, -compt};
        pt1 = Vector2Add(pt1, {no.x, no.y});
        pt2 = Vector2Add(pt2, {no.x, no.y});
        pt3 = Vector2Add(pt3, {no.x, no.y});
        DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, {90, 90, 90, 255});
        DrawLineEx({-largurat - (10 / zoom) + no.x, +compt - no.y}, {largurat + (10 / zoom) + no.x, +compt - no.y}, 3.0f / zoom, {90, 90, 90, 255});

        float distlinhas = (2 * largurat + (18 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {no.x - (largurat) - (8 / zoom) + (i * distlinhas), no.y - (compt)};
            Vector2 posicaof = {no.x - (largurat) - (18 / zoom) + (i * distlinhas), no.y - (compt) - (10 / zoom)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }

    if (no.fixoX && !no.fixoY && no.rotaZ) // Fixo em X e Z
    {
        DrawRectangleV({no.x - compq / 2, -no.y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
        DrawCircleV({no.x + (8.0f / zoom), (compq / 2) - no.y + (6.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({no.x - (8.0f / zoom), (compq / 2) - no.y + (6.0f / zoom)}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({-(compq / 2) - (10 / zoom) + no.x, (compq / 2) - no.y + (13.0f / zoom)}, {(compq / 2) + (10 / zoom) + no.x, (compq / 2) - no.y + (13.0f / zoom)}, 3.0f / zoom, {90, 90, 90, 255});

        float distlinhas = (compq + (18 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {no.x - (compq / 2) - (8 / zoom) + (i * distlinhas), no.y - (compq / 2) - (14 / zoom)};
            Vector2 posicaof = {no.x - (compq / 2) - (18 / zoom) + (i * distlinhas), no.y - (compq / 2) - (24 / zoom)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }

    if (!no.fixoX && no.fixoY && no.rotaZ) // Fixo em Y e Z
    {
        DrawRectangleV({no.x - compq / 2, -no.y - compq / 2}, {compq, compq}, {90, 90, 90, 255});
        DrawCircleV({-(compq / 2) + no.x - (6.0f / zoom), -(no.y + (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawCircleV({-(compq / 2) + no.x - (6.0f / zoom), -(no.y - (8.0f / zoom))}, 6.0f / zoom, {90, 90, 90, 255});
        DrawLineEx({no.x - (compq / 2) - (13.0f / zoom), -(compq / 2) - (10 / zoom) - no.y}, {no.x - (compq / 2) - (13.0f / zoom), (compq / 2) + (10 / zoom) - no.y}, 3.0f / zoom, {90, 90, 90, 255});

        float distlinhas = (compq + (18 / zoom)) / linhas;

        for (int i = 0; i <= linhas; i++)
        {
            Vector2 posicaoi = {no.x - (compq / 2) - (13 / zoom), no.y - (compq / 2) - (9 / zoom) + (i * distlinhas)};
            Vector2 posicaof = {no.x - (compq / 2) - (23 / zoom), no.y - (compq / 2) - (19 / zoom) + (i * distlinhas)};
            DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, {90, 90, 90, 255});
        }
    }
}

void RenderizadorEstrutura::desenhaBarras(const Estrutura& est, Camera2D camera)
{
    float esp = 6.0f; 
    for (const auto& barra : est.barras)
    {
        const No& noi = est.getNoById(barra.noInicialId);
        const No& nof = est.getNoById(barra.noFinalId);
        DrawLineEx({noi.x, -noi.y}, {nof.x, -nof.y}, esp / camera.zoom, BLUE);
    }
}