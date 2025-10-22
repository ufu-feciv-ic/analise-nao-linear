#include "Estrutura.h"
#include "DesenhoUtils.h"
#include <iostream>
#include <cmath>

// Implementação da classe No
int No::nextid = 0;

No::No(float x_, float y_) : x(x_), y(y_), fx(0), fy(0), mz(0), fixoX(false), fixoY(false), rotaZ(false)
{
    id = nextid++;
}

No::No(float x_, float y_, float fx_, float fy_, float mz_, bool fixoX_, bool fixoY_, bool rotaZ_) : x(x_), y(y_), fx(fx_), fy(fy_), mz(mz_), fixoX(fixoX_), fixoY(fixoY_), rotaZ(rotaZ_)
{
    id = nextid++;
}


// Implementação da classe Barra
Barra::Barra(const No& noi_, const No& nof_, float modElast_, float area_, float inercia_, float esp_)
: noi(noi_), nof(nof_), noInicialId(noi_.id), noFinalId(nof_.id), modElast(modElast_), area(area_), inercia(inercia_), esp(esp_)
{
    comprimento = sqrt(pow(nof.x - noi.x, 2) + pow(nof.y - noi.y, 2));
    
    kLocal.setZero();
    T.setZero();
    KGlobal.setZero();

    vGlobal.setZero();
    Fglobal.setZero();
    uLocal.setZero();
    fLocal.setZero();

    calculaMatrizRigidezLocal();
    calcularMatrizTransformacao();

    KGlobal = T.transpose() * kLocal * T;

    std::cout << "--- Nova Barra criada ---" << std::endl;
    std::cout << "modElas: " << modElast << std::endl;
    std::cout << "area: " << area << std::endl;
    std::cout << "inercia:" << inercia << std::endl;
    std::cout << "espessura: " << esp << std::endl;
    std::cout << "Comprimeto = " << comprimento << std::endl;
    std::cout << std::endl;
    std::cout << "Teste matriz kLocal = \n"
              << kLocal << std::endl;
    std::cout << std::endl;
    std::cout << "Matriz de transformação T = \n"
              << T << std::endl;
    std::cout << std::endl;
    std::cout << "Matriz de rigidez global KGlobal = \n"
              << KGlobal << std::endl;
    std::cout << std::endl;
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

void Barra::calcularMatrizTransformacao()
{
    float dx = nof.x - noi.x;
    float dy = nof.y - noi.y;
    cos = dx / comprimento;
    sen = dy / comprimento;

    T << cos, sen, 0, 0, 0, 0,
         -sen, cos, 0, 0, 0, 0,
         0, 0, 1, 0, 0, 0,
         0, 0, 0, cos, sen, 0,
         0, 0, 0, -sen, cos, 0,
         0, 0, 0, 0, 0, 1;
}

void Barra::calcularDeslocamentosGlobais(const Eigen::VectorXf &d, const std::array<int, 6> &bcn)
{
    int gln = 3;
    int nos = 2;

    for (int i = 0; i < gln*nos; i++)
    {
        vGlobal(i) = d(bcn[i]);
    }
}

void Barra::calcularForcasGlobais()
{
    Fglobal = KGlobal * vGlobal;
}

void Barra::calcularEsforcosLocais()
{
    uLocal = T * vGlobal;
    fLocal = kLocal * uLocal;
}

void Barra::calculaDeformadaLocal(float fatorEscala)
{
    pontosDeformada.clear();

    int numPontos = 20;

    if (comprimento <= 0)
    {
        std::cout << "Comprimento da barra inválido!" << std::endl;
        return;
    }

    if (uLocal.size() != 6)
    {
        std::cout << "Vetor de deslocamentos locais não calculado!" << std::endl;
        return;
    }

    float ui = uLocal(0); // deslocamento inicial no eixo x local
    float vi = uLocal(1); // deslocamento inicial no eixo y local
    float ti = uLocal(2); // rotação inicial no nó inicial
    float uf = uLocal(3); // deslocamento final no eixo x local
    float vf = uLocal(4); // deslocamento final no eixo y local
    float tf = uLocal(5); // rotação final no nó final

    for (int i = 0; i <= numPontos; i++)
    {
        float x = ((float) i / numPontos) * comprimento;

        std::cout << "x = " << x << std::endl;

        float xl = x / comprimento;

        float N0 = 1.0f - xl;
        float N1 = 1 - 3 * xl * xl + 2 * xl * xl * xl;
        float N2 = x * (1 - xl) * (1 - xl);
        float N3 = xl; 
        float N4 = 3 * xl * xl - 2 * xl * xl * xl;
        float N5 = x * x / comprimento * (xl - 1);

        float ux = N0 * ui + N3 * uf;
        float uy = N1 * vi + N2 * ti + N4 * vf + N5 * tf;

        ux *= fatorEscala;
        uy *= fatorEscala;

        float xDefLocal = x + ux;
        float yDefLocal = uy;

        float xDefGlobal = noi.x + (cos * xDefLocal - sen * yDefLocal);
        float yDefGlobal = noi.y + (sen * xDefLocal + cos * yDefLocal);

        pontosDeformada.push_back({xDefGlobal, yDefGlobal});
    }

    std::cout << "Verificar fator de escala" << std::endl;
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

void Estrutura::adicionarNo(const No& no)
{
    nos.push_back(no);
    // para cada chava (nesse caso armazenada com o mesmo id do nó) tem-se o nó correspondente
    mapaNos[no.id] = no;
}

void Estrutura::adicionarBarra(No noi_, No nof_, int noiId, int nofId, float modElast_, float area_, float inercia_, float esp_)
{
    // poderíamos usar mapaNos[ID] para pegar o nó, mas aí teríamos que garantir que o nó existe
    // usando o at, uma exceção é lançada se o nó não existir

    // for (enquanto o usuário clica)
    // barra(nos[n do click], no2, );
    // barras(no2, no3);

    // const No& noi = mapaNos.at(noiId);
    // const No& nof = mapaNos.at(nofId);
    // barras.emplace_back(noi, nof, modElast_, area_, inercia_, esp_);

    barras.emplace_back(noi_, nof_, modElast_, area_, inercia_, esp_);
}

// const No&, uma referência para o No e garantia que o nó não será modificado
// se o nó não existir, uma exceção é lançada
// a função não modificará a instância de Estrutura no escopo da função, por isso é const 
// função tem caráter read-only sobre a instância de Estrutura 
const No& Estrutura::getNoById(int id) const
{
    return mapaNos.at(id);
}

void Estrutura::montarBCN()
{
    BCN.clear();

    std::cout << "construir BCN sem usar o ID do nó, usar o índice do vetor" << std::endl;

    for (const auto& barra : barras)
    {
        BCN.push_back({barra.noInicialId * 3, barra.noInicialId * 3 + 1, barra.noInicialId * 3 + 2,
                       barra.noFinalId * 3, barra.noFinalId * 3 + 1, barra.noFinalId * 3 + 2});
    }

    std::cout << "BCN = \n";
    for (const auto& linha : BCN)
    {
        for (const auto& valor : linha)
        {
            std::cout << valor << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Estrutura::calcularMatrizRigidezEstrutura()
{
    // Determina a dimensão da matriz de rigidez global da estrutura (n,n)
    S.resize(nos.size() * 3, nos.size() * 3);
    S.setZero();

    // Monta o Beam Code Number (BCN)
    montarBCN();

    // Monta a matriz de rigidez global da estrutura
    for (size_t n = 0; n < barras.size(); n++)
    {
        const auto& barra = barras[n];
        const auto& bcn = BCN[n];

        // for (const auto& val : bcn)
        // {
        //     std::cout << val << " ";
        // }

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                S(bcn[i], bcn[j]) += barra.KGlobal(i, j);
            }
        }
    }

    std::cout << "Matriz de rigidez global da estrutura S = \n"
              << S << std::endl;

    std::cout << std::endl;
}

void Estrutura::montarVetorForcas()
{
    P.resize(nos.size() * 3);
    P.setZero();
    R.resize(nos.size() * 3);
    R.setZero();

    std::cout << "Montando vetor de forças P..." << std::endl;
    std::cout << P << std::endl;

    // for (size_t i = 0; i < nos.size(); i++)
    // {
    //     P(i * 3) = nos[i].fx;
    //     P(i * 3 + 1) = nos[i].fy;
    //     P(i * 3 + 2) = nos[i].mz;
    // }

    for (const auto& no : nos)
    {
        P(no.id * 3) = no.fx;
        P(no.id * 3 + 1) = no.fy;
        P(no.id * 3 + 2) = no.mz;
    }

    Pu = P;

    std::cout << "\nVetor de forças P = \n"
              << P << std::endl;
}

void Estrutura::aplicarCondicoesDeContorno()
{

    std::cout << "Parar de usar o ID do nó, usar o índice do vetor" << std::endl;
    std::cout << "Nomes dos vetores e matrizes com e sem vínculos (0 e 1)" << std::endl; 

    for (const auto& no : nos)
    {
        if (no.fixoX)
        {
            int gln = no.id * 3;
            S.row(gln).setZero();
            S.col(gln).setZero();
            S(gln, gln) = 1.0f;
            P(gln) = 0.0f;
        }
        if (no.fixoY)
        {
            int gln = no.id * 3 + 1;
            S.row(gln).setZero();
            S.col(gln).setZero();
            S(gln, gln) = 1.0f;
            P(gln) = 0.0f;
        }
        if (no.rotaZ)
        {
            int gln = no.id * 3 + 2;
            S.row(gln).setZero();
            S.col(gln).setZero();
            S(gln, gln) = 1.0f;
            P(gln) = 0.0f;
        }
    }

    std::cout << "Matriz de rigidez global da estrutura S (após aplicar CC) = \n"
              << S << std::endl;

    std::cout << "\nVetor de forças P (após aplicar CC) = \n"
                << P << std::endl;
}

void Estrutura::calcularPontosDeformadaEstrutura(float fatorEscala)
{
    for (auto& barra : barras)
    {
        barra.calculaDeformadaLocal(fatorEscala);;
    }
}

void Estrutura::resolverSistema()
{
    calcularMatrizRigidezEstrutura();
    montarVetorForcas();
    aplicarCondicoesDeContorno();

    Eigen::LLT<Eigen::MatrixXf> llt(S);

    std::cout << "\nResolvendo sistema S * d = P..." << std::endl;

    Eigen::MatrixXf L = llt.matrixL();
    std::cout << "\nFatoração LLT de S = \n" << L << std::endl;

    if (llt.info() == Eigen::Success)
    {
        d = llt.solve(P);
        std::cout << "\nDeslocamentos d = \n"
                  << d << std::endl;

        for (size_t n = 0; n < barras.size(); n++)
        {
            barras[n].calcularDeslocamentosGlobais(d, BCN[n]);
            std::cout << "\nDeslocamentos globais da barra " << n << " = \n"
                      << barras[n].vGlobal << std::endl;

            barras[n].calcularForcasGlobais();
            std::cout << "\nForças globais da barra " << n << " = \n"
                      << barras[n].Fglobal << std::endl;

            barras[n].calcularEsforcosLocais();
            std::cout << "\nEsforços locais da barra " << n << " = \n"
                      << barras[n].fLocal << std::endl;

            for (int i = 0; i < 6; i++)
            {
                R(BCN[n][i]) += barras[n].Fglobal(i);
            }              
        }

        R = R - P; // Este P não pode ter as forças entre os nós, somente aplicadas nos nos 
        
        std::cout << "\nReações de apoio da estrutura" << " = \n"
                      << R << std::endl;     
    }
    else
    {
        std::cout << "Decomposição LLT falhou. A matriz pode não ser positiva definida." << std::endl;
    }
}
