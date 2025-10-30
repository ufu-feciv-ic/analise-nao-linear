#include "DesenhoUtils.h"
#include <cmath>
#include <cstring>
#include <iostream>

Vector2 RotacaoPonto(Vector2 posicao, float angulo_)
{
    float x_ = posicao.x * cosf(angulo_) - posicao.y * sinf(angulo_);
    float y_ = posicao.x * sinf(angulo_) + posicao.y * cosf(angulo_);
    return {x_, y_};
}

Vector2 RotatePoint(Vector2 pivot, Vector2 point, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    // Translate point back to origin
    point.x -= pivot.x;
    point.y -= pivot.y;

    // Rotate point
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;

    // Translate point back
    point.x = xnew + pivot.x;
    point.y = ynew + pivot.y;
    return point;
}

Vector2 FindIntersection(Vector2 rectSize, Vector2 size)
{
    if (size.x == 0)
    {
        return {0, std::copysign(rectSize.y / 2, size.y)};
    }
    else if (size.y == 0)
    {
        return {std::copysign(rectSize.x / 2, size.x), 0};
    }

    float intersectionY = rectSize.x / 2 * fabs(size.y) / fabs(size.x);
    float intersectionLimitY = rectSize.y / 2;

    if (intersectionY < intersectionLimitY)
    {
        return {std::copysign(rectSize.x / 2, size.x), std::copysign(intersectionY, size.y)};
    }
    else if (intersectionY > intersectionLimitY)
    {
        return {std::copysign((rectSize.y / 2) / (fabs(size.y) / fabs(size.x)), size.x), std::copysign(rectSize.y / 2, size.y)};
    }
    else
    {
        return {std::copysign(rectSize.x / 2, size.x), std::copysign(rectSize.y / 2, size.y)};
    }
}

void drawSeta(Vector2 psi, Vector2 psf, float esp, float compt, float zoom, Color corSeta)
{
    float angulo = atan2(psf.y - psi.y, psf.x - psi.x);
    float L = sqrt(pow(psf.x - psi.x, 2) + pow(psf.y - psi.y, 2));

    compt = compt / zoom;

    if (L <= compt)
    {
        compt = L;
    }

    float largurat = (tanf(15 * PI / 180) * compt);

    Vector2 porigem1 = {0, 0};
    Vector2 porigem2 = {-compt, largurat};
    Vector2 porigem3 = {-compt, -largurat};
    Vector2 prota1 = RotacaoPonto(porigem1, angulo);
    Vector2 prota2 = RotacaoPonto(porigem2, angulo);
    Vector2 prota3 = RotacaoPonto(porigem3, angulo);
    Vector2 pt1 = Vector2Add(prota1, psf);
    Vector2 pt2 = Vector2Add(prota2, psf);
    Vector2 pt3 = Vector2Add(prota3, psf);

    float compt_x = cosf(angulo) * compt;
    float compt_y = sinf(angulo) * compt;

    psf.x = psf.x - compt_x;
    psf.y = psf.y - compt_y;

    DrawLineEx({psi.x, -psi.y}, {psf.x, -psf.y}, esp / zoom, corSeta);
    DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, corSeta);
}

// void drawForca(float x, float y, float fx, float fy, float zoom, Color cor)
// {
//     float angulo = atan2(fy, fx);
//     Vector2 psf = {x - cosf(angulo) * (8 / zoom), y - sinf(angulo) * (8 / zoom)};
//     Vector2 psi = {-fx + x - cosf(angulo) * (8 / zoom), -fy + y - sinf(angulo) * (8 / zoom)};
//     drawSeta(psi, psf, 3.0f, 30.0f, zoom, cor);

//     if (fx != 0 && fy != 0)
//     {
//         if (fx > 0 && fy < 0)
//         {
//             DrawTextEx(GetFontDefault(), TextFormat("(%.2f kN, %.2f kN)", fx, fy), {psi.x + (8.0f / zoom), -psi.y - (8.0f / zoom) - (20.0f / zoom)}, 20.0f / zoom, 2.0f / zoom, cor);
//         }
//         else
//         {
//             DrawTextEx(GetFontDefault(), TextFormat("(%.2f kN, %.2f kN)", fx, fy), {psi.x + (8.0f / zoom), -psi.y + (8.0f / zoom)}, 20.0f / zoom, 2.0f / zoom, cor);
//         }
//     }
// }

// void drawForca(float x, float y, float fx, float fy, float zoom, Color cor)
// {
//     // --- 1. Cálculo da Norma (Magnitude) ---
//     // Usamos o Teorema de Pitágoras para encontrar o "comprimento" do vetor.
//     // A função sqrtf() (square root float) vem da biblioteca <math.h>.
//     float norma = sqrtf(fx * fx + fy * fy);

//     // --- 2. Lógica Original (Cálculo da Seta) ---
//     // Mantemos os seus cálculos originais para os pontos da seta.
//     float angulo = atan2(fy, fx);
//     Vector2 psf = {x - cosf(angulo) * (8 / zoom), y - sinf(angulo) * (8 / zoom)};
//     Vector2 psi = {-fx + x - cosf(angulo) * (8 / zoom), -fy + y - sinf(angulo) * (8 / zoom)};
    
//     // (Assumindo que esta função já existe no seu código)
//     drawSeta(psi, psf, 3.0f, 12.0f, zoom, cor);

//     // --- 3. Correção do Texto (Desenhar a Norma) ---
    
//     // Condição corrigida:
//     // Vamos desenhar o texto se a norma for maior que um valor muito pequeno
//     // (para evitar desenhar "0.00 kN" se a força for nula).
//     // A sua condição original (fx != 0 && fy != 0) não mostrava forças 
//     // puramente horizontais ou verticais.
//     if (norma > 0.001f)
//     {
//         // Texto corrigido:
//         // Formatamos o texto para mostrar a 'norma' com 2 casas decimais.
//         const char *textoNorma = TextFormat("%.2f kN", norma);

//         // Posição do Texto (Lógica Original):
//         // Mantive a sua lógica original para posicionar o texto,
//         // que parece ajustar a posição dependendo do quadrante
//         // (e parece assumir um eixo Y invertido, com o uso de -psi.y).
//         if (fx > 0 && fy < 0)
//         {
//             DrawTextEx(GetFontDefault(), 
//                        textoNorma, // <- MUDANÇA AQUI
//                        {psi.x + (8.0f / zoom), -psi.y - (8.0f / zoom) - (10.0f / zoom)}, 
//                        10.0f / zoom, 
//                        2.0f / zoom, 
//                        cor);
//         }
//         else
//         {
//             DrawTextEx(GetFontDefault(), 
//                        textoNorma, // <- MUDANÇA AQUI
//                        {psi.x + (8.0f / zoom), -psi.y + (8.0f / zoom)}, 
//                        10.0f / zoom, 
//                        2.0f / zoom, 
//                        cor);
//         }
//     }
// }

void drawForca(float x, float y, float fx, float fy, float zoom, Color cor)
{
    // --- 1. Cálculo da Norma (Magnitude) ---
    // Usamos o Teorema de Pitágoras para encontrar o "comprimento" do vetor.
    float norma = sqrtf(fx * fx + fy * fy);

    // Se a força for (quase) zero, não desenha nada
    if (norma < 0.001f) return;

    // --- 2. [NOVO] Fator de Escala Visual ---
    // Este fator converte a magnitude (ex: 100.0) para unidades do mundo (ex: 2.0)
    // Ajuste este valor para mudar o comprimento das setas
    float escalaVisual = 0.02f; 
    float fx_vis = fx * escalaVisual; // Força visual em X
    float fy_vis = fy * escalaVisual; // Força visual em Y

    // --- 3. Cálculo da Seta (Modificado) ---
    // Usamos o ângulo original da força para manter a direção correta
    float angulo = atan2(fy, fx);
    
    // Ponto final da seta (psf) - perto do nó
    Vector2 psf = {x - cosf(angulo) * (8 / zoom), y - sinf(angulo) * (8 / zoom)};
    
    // Ponto inicial da seta (psi) - USA AS FORÇAS VISUAIS (escaladas)
    Vector2 psi = {-fx_vis + x - cosf(angulo) * (8 / zoom), -fy_vis + y - sinf(angulo) * (8 / zoom)};
    
    // Desenha a seta (com a ponta de 12.0f, como sugerido na resposta anterior)
    drawSeta(psi, psf, 3.0f, 12.0f, zoom, cor); //

    // --- 4. Desenho do Texto (Corrigido para 'norma' e tamanho 10) ---
    // O texto mostra a 'norma' (valor real), não a força visual.
    
    const char *textoNorma = TextFormat("%.2f kN", norma);
    float tamanhoFonte = 10.0f / zoom;
    float espacamento = 1.0f / zoom;

    // Lógica original de posicionamento do texto
    if (fx > 0 && fy < 0)
    {
        DrawTextEx(GetFontDefault(), 
                   textoNorma,
                   // Offset ajustado para a nova fonte (10.0f)
                   {psi.x + (8.0f / zoom), -psi.y - (8.0f / zoom) - (10.0f / zoom)}, 
                   tamanhoFonte, 
                   espacamento, 
                   cor);
    }
    else
    {
        DrawTextEx(GetFontDefault(), 
                   textoNorma,
                   {psi.x + (8.0f / zoom), -psi.y + (8.0f / zoom)}, 
                   tamanhoFonte, 
                   espacamento, 
                   cor);
    }
}

void drawMoment(Vector2 position, float radius, bool isAnticlockwise, float arrowLength, float arrowWidth, float lineWidth, Color color, Camera2D camera, const char *annotation = "")
{
    Vector2 p1;
    Vector2 p2;
    Vector2 p3;
    int nSteps = 18;
    float arcBegin = .4f * 2 * PI;
    float arcEnd = .1f * 2 * PI;

    std::vector<float> x(nSteps + 1);
    std::vector<float> y(nSteps + 1);

    float angleStep = (arcEnd - arcBegin) / (nSteps);

    for (int i = 0; i < nSteps + 1; ++i)
    {
        float angle = arcBegin + i * angleStep;
        x[i] = position.x + radius * cos(angle);
        y[i] = position.y + radius * sin(angle);
    }

    float xS;
    float yS;
    float angleS;

    if (isAnticlockwise)
    {
        xS = x.back();
        yS = y.back();
        angleS = arcBegin + PI;
    }
    else
    {
        xS = x.front();
        yS = y.front();
        angleS = arcEnd + PI;
    }

    p1 = {xS, yS};
    p2 = {xS - arrowLength, yS - arrowWidth};
    p3 = {xS - arrowLength, yS + arrowWidth};

    p2 = RotatePoint(p1, p2, angleS);
    p3 = RotatePoint(p1, p3, angleS);

    for (int i = 0; i < nSteps; ++i)
    {
        Vector2 startPoint = {x[i], y[i]};
        Vector2 endPoint = {x[i + 1], y[i + 1]};
        DrawLineEx({startPoint.x, -startPoint.y}, {endPoint.x, -endPoint.y}, lineWidth, color);
    }
    DrawTriangle({p1.x, -p1.y}, {p2.x, -p2.y}, {p3.x, -p3.y}, color);
    DrawTriangle({p3.x, -p3.y}, {p2.x, -p2.y}, {p1.x, -p1.y}, color);

    if (strcmp(annotation, "") != 0)
    {

        /*
        int intOffset = 3;
        Vector2 offset = Vector2Scale(Vector2{0, 1}, intOffset / camera.zoom);
        Vector2 textSizeOri = MeasureTextEx(GetFontDefault(), annotation, 10, 1);
        Vector2 textSize = Vector2{textSizeOri.x, textSizeOri.y * .6f};
        Vector2 textPositionWorld = Vector2{position.x + offset.x, -position.y - radius - offset.y};
        Vector2 textPosition = Vector2Subtract(GetWorldToScreen2D(textPositionWorld, camera), Vector2Scale(textSize, .5f));
        Vector2 textCenterOffset = FindIntersection(textSize, Vector2{0, -radius / camera.zoom}); // Não entendi o por que do sinal de menos
        textPosition = Vector2{textPosition.x + textCenterOffset.x, textPosition.y + textCenterOffset.y};
        DrawTextEx(GetFontDefault(), annotation, Vector2{textPosition.x/camera.zoom, textPosition.y - (textSizeOri.y - textSize.y)/2 - textSizeOri.y*.07f}, 10, 1, color);
        */

        DrawTextEx(GetFontDefault(), annotation, {position.x + radius, -(position.y + radius + (15.0f / camera.zoom))}, 20.0f / camera.zoom, 2.0f / camera.zoom, ORANGE);
    }
}

void drawFixedSizeAnnotadedMoment(Vector2 position, float moment, float radius, float arrowLength, float arrowWidth, float lineWidth, Color color, Camera2D cam)
{
    if (fabsf(moment) < 0.0000000001f)
        return;

    char annotation[256];
    snprintf(annotation, sizeof(annotation), "%.2f kg.m", fabsf(moment));

    drawMoment(position, radius / cam.zoom, moment > 0 ? true : false, arrowLength / cam.zoom, arrowWidth / cam.zoom, lineWidth / cam.zoom, color, cam, annotation);
}