#include "raylib.h"
#include <iostream>
#include "raymath.h"
#include <vector>
#include <array>
#include <cstring>

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

void drawForca(float x, float y, float fx, float fy, float zoom)
{
    float angulo = atan2(fy, fx);
    Vector2 psf = {x - cosf(angulo) * (8 / zoom), y - sinf(angulo) * (8 / zoom)};
    Vector2 psi = {-fx + x - cosf(angulo) * (8 / zoom), -fy + y - sinf(angulo) * (8 / zoom)};
    drawSeta(psi, psf, 3.0f, 30.0f, zoom, RED);

    if (fx != 0 && fy != 0)
    {
        if (fx > 0 && fy < 0)
        {
            DrawTextEx(GetFontDefault(), TextFormat("(%.2f kN, %.2f kN)", fx, fy), {psi.x + (8.0f / zoom), -psi.y - (8.0f / zoom) - (20.0f / zoom)}, 20.0f / zoom, 2.0f / zoom, RED);
        }
        else
        {
            DrawTextEx(GetFontDefault(), TextFormat("(%.2f kN, %.2f kN)", fx, fy), {psi.x + (8.0f / zoom), -psi.y + (8.0f / zoom)}, 20.0f / zoom, 2.0f / zoom, RED);
        }
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

    No(float x_, float y_, Camera2D camera_) : x(x_), y(y_), fx(0), fy(0), mz(0), fixoX(false), fixoY(false), rotaZ(false), camera(camera_)
    {
        id = nextid++;
    }

    No(float x_, float y_, float fx_, float fy_, float mz_, bool fixoX_, bool fixoY_, bool rotaZ_, Camera2D camera_) : x(x_), y(y_), fx(fx_), fy(fy_), mz(mz_), fixoX(fixoX_), fixoY(fixoY_), rotaZ(rotaZ_), camera(camera_)
    {
        id = nextid++;
    }

    void drawNo(Camera2D camera)
    {
        drawApoios(camera.zoom);
        drawPonto(camera.zoom);
        drawForca(x, y, fx, fy, camera.zoom);
        drawFixedSizeAnnotadedMoment({x, y}, mz, 18.0f, 10.0f, 6.0f, 3.0f, ORANGE, camera);
        drawIndice(camera.zoom);
    }

    void drawPonto(float zoom)
    {
        DrawCircleV({x, -y}, 6 / zoom, {0, 235, 255, 255});
    }

    void drawIndice(float zoom)
    {
        DrawTextEx(GetFontDefault(), TextFormat("%i", id), {x + (10.0f / zoom), -y + (10.0f / zoom)}, 20.0f / zoom, 2.0f, {0, 235, 255, 255});
    }

    void drawApoios(float zoom)
    {
        float compt = 22 / zoom;
        float compq = 32 / zoom;
        float largurat = (tanf(35 * PI / 180) * compt);
        float linhas = 4;

        if (fixoX && fixoY && rotaZ) // Fixo em Tudo
        {
            DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, { 90, 90, 90, 255});
            float distlinhas = (compq-(2/zoom))/linhas;

            for(int i=0; i <=linhas; i++)
            {
                Vector2 posicaoi = {x-(compq/2)+(1/zoom)+(i*distlinhas), y-(compq/2)+(2/zoom)};
                Vector2 posicaof = {x-(compq/2)-(11/zoom)+(i*distlinhas), y-(compq/2)-(12/zoom)};
                DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, { 90, 90, 90, 255});
            }

            for(int i=0; i <=linhas; i++)
            {
                Vector2 posicaoi = {x+(compq/2)-(1/zoom), y-(compq/2)+(1/zoom)+(i*distlinhas)};
                Vector2 posicaof = {x+(compq/2)+(11/zoom), y-(compq/2)-(11/zoom)+(i*distlinhas)};
                DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, { 90, 90, 90, 255});
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
            DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, { 90, 90, 90, 255});
            DrawLineEx({-compt+x , -largurat-(10/zoom)-y}, {-compt+x , largurat+(10/zoom)-y}, 3.0f / zoom, { 90, 90, 90, 255});
            DrawCircleV({-compt+x-(7.0f/zoom), -(y+(8.0f/zoom))}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawCircleV({-compt+x-(7.0f/zoom), -(y-(8.0f/zoom))}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawLineEx({-compt+x-(14.0f/zoom) , -largurat-(10/zoom)-y}, {-compt+x-(14.0f/zoom) , largurat+(10/zoom)-y}, 3.0f / zoom, { 90, 90, 90, 255});
        }
        
        if (!fixoX && fixoY && !rotaZ) // Fixo só em Y
        {
            Vector2 pt1 = {0, 0};
            Vector2 pt2 = {-largurat, -compt};
            Vector2 pt3 = {largurat, -compt};
            pt1 = Vector2Add(pt1, {x, y});
            pt2 = Vector2Add(pt2, {x, y});
            pt3 = Vector2Add(pt3, {x, y});
            DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, { 90, 90, 90, 255});
            DrawLineEx({-largurat-(10/zoom)+x, +compt-y }, { largurat+(10/zoom)+x, +compt-y }, 3.0f / zoom, { 90, 90, 90, 255});
            DrawCircleV({x+(8.0f/zoom), compt-y+(7.0f/zoom)}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawCircleV({x-(8.0f/zoom), compt-y+(7.0f/zoom)}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawLineEx({-largurat-(10/zoom)+x, compt-y+(14.0f/zoom)}, {largurat+(10/zoom)+x , compt-y+(14.0f/zoom)}, 3.0f / zoom, { 90, 90, 90, 255});
            
        }
        if (!fixoX && !fixoY && rotaZ) // Fixo só em Z
        {
            DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, { 90, 90, 90, 255});
        }

        if (fixoX && fixoY && !rotaZ) // Fixo em X e Y
        {
            Vector2 pt1 = {0, 0};
            Vector2 pt2 = {-largurat, -compt};
            Vector2 pt3 = {largurat, -compt};
            pt1 = Vector2Add(pt1, {x, y});
            pt2 = Vector2Add(pt2, {x, y});
            pt3 = Vector2Add(pt3, {x, y});
            DrawTriangle({pt1.x, -pt1.y}, {pt2.x, -pt2.y}, {pt3.x, -pt3.y}, { 90, 90, 90, 255});
            DrawLineEx({-largurat-(10/zoom)+x, +compt-y }, { largurat+(10/zoom)+x, +compt-y }, 3.0f / zoom, { 90, 90, 90, 255});

            float distlinhas = (2*largurat+(18/zoom))/linhas;

            for(int i=0; i <=linhas; i++)
            {
                Vector2 posicaoi = {x-(largurat)-(8/zoom)+(i*distlinhas), y-(compt)};
                Vector2 posicaof = {x-(largurat)-(18/zoom)+(i*distlinhas), y-(compt)-(10/zoom)};
                DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, { 90, 90, 90, 255});
            }
            
        }

        if (fixoX && !fixoY && rotaZ) // Fixo em X e Z
        {
            DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, { 90, 90, 90, 255});
            DrawCircleV({x+(8.0f/zoom), (compq/2)-y+(6.0f/zoom)}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawCircleV({x-(8.0f/zoom), (compq/2)-y+(6.0f/zoom)}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawLineEx({-(compq/2)-(10/zoom)+x, (compq/2)-y+(13.0f/zoom)}, {(compq/2)+(10/zoom)+x , (compq/2)-y+(13.0f/zoom)}, 3.0f / zoom, { 90, 90, 90, 255});

            float distlinhas = (compq+(18/zoom))/linhas;

            for(int i=0; i <=linhas; i++)
            {
                Vector2 posicaoi = {x-(compq/2)-(8/zoom)+(i*distlinhas), y-(compq/2)-(14/zoom)};
                Vector2 posicaof = {x-(compq/2)-(18/zoom)+(i*distlinhas), y-(compq/2)-(24/zoom)};
                DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, { 90, 90, 90, 255});
            }

        }

        if (!fixoX && fixoY && rotaZ) // Fixo em Y e Z
        {
            DrawRectangleV({x - compq / 2, -y - compq / 2}, {compq, compq}, { 90, 90, 90, 255});
            DrawCircleV({-(compq/2)+x-(6.0f/zoom), -(y+(8.0f/zoom))}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawCircleV({-(compq/2)+x-(6.0f/zoom), -(y-(8.0f/zoom))}, 6.0f / zoom, { 90, 90, 90, 255});
            DrawLineEx({x-(compq/2)-(13.0f/zoom), -(compq/2)-(10/zoom)-y}, {x-(compq/2)-(13.0f/zoom), (compq/2)+(10/zoom)-y}, 3.0f / zoom, { 90, 90, 90, 255});
       
            float distlinhas = (compq+(18/zoom))/linhas;

            for(int i=0; i <=linhas; i++)
            {
                Vector2 posicaoi = {x-(compq/2)-(13/zoom), y-(compq/2)-(9/zoom)+(i*distlinhas)};
                Vector2 posicaof = {x-(compq/2)-(23/zoom), y-(compq/2)-(19/zoom)+(i*distlinhas)};
                DrawLineEx({posicaoi.x, -posicaoi.y}, {posicaof.x, -posicaof.y}, 2.0f / zoom, { 90, 90, 90, 255});
            }
        }       
    }
};

class Barra
{
public:
    No noi;
    No nof;
    float L;
    float esp;

    Barra() = default;

    Barra(No noi_, No nof_, float esp_) : noi(noi_), nof(nof_), esp(esp_)
    {
        L = sqrt(pow(nof.x - noi.x, 2) + pow(nof.y - noi.y, 2));
        std::cout << "Comprimeto = " << L << std::endl;
    }

    void draw(Camera2D camera)
    {
        DrawLineEx({noi.x, -noi.y}, {nof.x, -nof.y}, esp / camera.zoom, BLUE);
    }
};

int No::nextid = 0;

class Estrutura
{
public:
    std::vector<No> nos;
    std::vector<std::array<int, 2>> conexoes;
    std::vector<Barra> barras;

    Estrutura() = default;

    Estrutura(std::vector<No> nos_, std::vector<std::array<int, 2>> conexoes_) : nos(nos_), conexoes(conexoes_)
    {
        for (size_t i = 0; i < conexoes.size(); i++)
        {
            barras.emplace_back(nos[conexoes[i][0]], nos[conexoes[i][1]], 6.0f);
        }
    }

    void drawNos(Camera2D camera)
    {
        for (auto &no : nos)
            no.drawNo(camera);
    }

    void drawBarras(Camera2D camera)
    {
        for (auto &barra : barras)
            barra.draw(camera);
    }

    void draw(Camera2D camera)
    {
        drawBarras(camera);
        drawNos(camera);
    }
};

int main()
{
    float screenWidth = 800;
    float screenHeight = 600;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Análise Não Linear");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.zoom = 25.0f;
    camera.offset = {screenWidth / 2, screenHeight / 2};
    camera.target = {0, 0};

    std::vector<No> nos;
    nos.emplace_back(0.0f, 0.0f,0, 0, 0, true, false, true, camera);
    nos.emplace_back(2.0f, 8.0f, -2.3f, 3.8f, 15.0f, true, false, false, camera);
    nos.emplace_back(6.0f, 8.0f, 1.5f, -3.2f, 0.0f, false, true, false, camera);
    nos.emplace_back(8.0f, 0.0f, -3.0f, -2.2f, -5.0f, false, false, true, camera);

    std::vector<std::array<int, 2>> conexoes;
    conexoes.push_back({0, 1});
    conexoes.push_back({1, 2});
    conexoes.push_back({2, 3});

    Estrutura est(nos, conexoes);

    while (!WindowShouldClose())
    {
        { // CAMERA
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f / camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }
            {
                // Zoom based on mouse wheel
                float wheel = GetMouseWheelMove();
                if (wheel != 0)
                {
                    // Get the world point that is under the mouse
                    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

                    // Set the offset to where the mouse is
                    camera.offset = GetMousePosition();

                    // Set the target to match, so that the camera maps the world space point
                    // under the cursor to the screen space point under the cursor at any zoom
                    camera.target = mouseWorldPos;

                    // Zoom increment
                    // Uses log scaling to provide consistent zoom speed
                    float scale = 0.2f * wheel;
                    camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 640.0f);
                }
            }
        }

        BeginDrawing();
            ClearBackground({40, 40, 40, 255});
            BeginMode2D(camera);
                DrawLineEx({ -(float)GetScreenWidth()/2, 0.0f}, { (float)GetScreenWidth()/2, 0.0f}, 3.0/camera.zoom, { 30, 30, 30, 255});
                DrawLineEx({0.0f, -(float)GetScreenWidth()/2}, {0.0f, (float)GetScreenWidth()/2}, 3.0/camera.zoom, { 30, 30, 30, 255});
                est.draw(camera);

            EndMode2D();
        EndDrawing();

    }
    CloseWindow();
    return 0;
}