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

    // std::cout << "--- Nova Barra criada ---" << std::endl;
    // std::cout << "modElas: " << modElast << std::endl;
    // std::cout << "area: " << area << std::endl;
    // std::cout << "inercia:" << inercia << std::endl;
    // std::cout << "espessura: " << esp << std::endl;
    // std::cout << "Comprimeto = " << comprimento << std::endl;
    // std::cout << std::endl;
    // std::cout << "Teste matriz kLocal = \n"
    //           << kLocal << std::endl;
    // std::cout << std::endl;
    // std::cout << "Matriz de transformação T = \n"
    //           << T << std::endl;
    // std::cout << std::endl;
    // std::cout << "Matriz de rigidez global KGlobal = \n"
    //           << KGlobal << std::endl;
    // std::cout << std::endl;
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

    // std::cout << "\nVerificar fator de escala" << std::endl;
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

    // std::cout << "construir BCN sem usar o ID do nó, usar o índice do vetor" << std::endl;

    // for (int n = 0; n < (int)nos.size(); n++)
    // {
    //     BCN.push_back({n * 3, n * 3 + 1, n * 3 + 2, (n + 1) * 3, (n + 1) * 3 + 1, (n + 1) * 3 + 2});
    // }

    for (const auto& barra : barras)
    {
        BCN.push_back({barra.noInicialId * 3, barra.noInicialId * 3 + 1, barra.noInicialId * 3 + 2,
                       barra.noFinalId * 3, barra.noFinalId * 3 + 1, barra.noFinalId * 3 + 2});
    }

    // std::cout << "BCN = \n";
    // for (const auto& linha : BCN)
    // {
    //     for (const auto& valor : linha)
    //     {
    //         std::cout << valor << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;
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

    // std::cout << "Matriz de rigidez global da estrutura S = \n"
    //           << S << std::endl;

    // std::cout << std::endl;
}

void Estrutura::montarVetorForcas()
{
    P.resize(nos.size() * 3);
    P.setZero();
    R.resize(nos.size() * 3);
    R.setZero();

    // std::cout << "Montando vetor de forças P..." << std::endl;
    // std::cout << P << std::endl;

    // for (size_t i = 0; i < nos.size(); i++)
    // {
    //     P(i * 3) = nos[i].fx;
    //     P(i * 3 + 1) = nos[i].fy;
    //     P(i * 3 + 2) = nos[i].mz;
    // }

    for (int n = 0; n < (int)nos.size(); n++)
    {
        const auto& no = nos[n];
        P(n * 3) = no.fx;
        P(n * 3 + 1) = no.fy;
        P(n * 3 + 2) = no.mz;
    }

    // for (const auto& no : nos)
    // {
    //     P(no.id * 3) = no.fx;
    //     P(no.id * 3 + 1) = no.fy;
    //     P(no.id * 3 + 2) = no.mz;
    // }

    Pu = P;

    // std::cout << "\nVetor de forças P = \n"
    //           << P << std::endl;
}

void Estrutura::aplicarCondicoesDeContorno()
{

    // std::cout << "Parar de usar o ID do nó, usar o índice do vetor" << std::endl;
    // std::cout << "Nomes dos vetores e matrizes com e sem vínculos (0 e 1)" << std::endl; 

    for (int n = 0; n < (int)nos.size(); n++)
    {
        const auto& no = nos[n];
        if (no.fixoX)
        {
            int gln = n * 3;
            S.row(gln).setZero();
            S.col(gln).setZero();
            S(gln, gln) = 1.0f;
            P(gln) = 0.0f;
        }
        if (no.fixoY)
        {
            int gln = n * 3 + 1;
            S.row(gln).setZero();
            S.col(gln).setZero();
            S(gln, gln) = 1.0f;
            P(gln) = 0.0f;
        }
        if (no.rotaZ)
        {
            int gln = n * 3 + 2;
            S.row(gln).setZero();
            S.col(gln).setZero();
            S(gln, gln) = 1.0f;
            P(gln) = 0.0f;
        }
    }

    // for (const auto& no : nos)
    // {
    //     if (no.fixoX)
    //     {
    //         int gln = no.id * 3;
    //         S.row(gln).setZero();
    //         S.col(gln).setZero();
    //         S(gln, gln) = 1.0f;
    //         P(gln) = 0.0f;
    //     }
    //     if (no.fixoY)
    //     {
    //         int gln = no.id * 3 + 1;
    //         S.row(gln).setZero();
    //         S.col(gln).setZero();
    //         S(gln, gln) = 1.0f;
    //         P(gln) = 0.0f;
    //     }
    //     if (no.rotaZ)
    //     {
    //         int gln = no.id * 3 + 2;
    //         S.row(gln).setZero();
    //         S.col(gln).setZero();
    //         S(gln, gln) = 1.0f;
    //         P(gln) = 0.0f;
    //     }
    // }

    // std::cout << "Matriz de rigidez global da estrutura S (após aplicar CC) = \n"
    //           << S << std::endl;

    // std::cout << "\nVetor de forças P (após aplicar CC) = \n"
    //             << P << std::endl;
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

    // std::cout << "\nResolvendo sistema S * d = P..." << std::endl;

    Eigen::MatrixXf L = llt.matrixL();
    // std::cout << "\nFatoração LLT de S = \n" << L << std::endl;

    if (llt.info() == Eigen::Success)
    {
        d = llt.solve(P);
        // std::cout << "\nDeslocamentos d = \n"
        //           << d << std::endl;

        for (size_t n = 0; n < barras.size(); n++)
        {
            barras[n].calcularDeslocamentosGlobais(d, BCN[n]);
            // std::cout << "\nDeslocamentos globais da barra " << n << " = \n"
            //           << barras[n].vGlobal << std::endl;

            barras[n].calcularForcasGlobais();
            // std::cout << "\nForças globais da barra " << n << " = \n"
            //           << barras[n].Fglobal << std::endl;

            barras[n].calcularEsforcosLocais();
            // std::cout << "\nEsforços locais da barra " << n << " = \n"
            //           << barras[n].fLocal << std::endl;

            for (int i = 0; i < 6; i++)
            {
                R(BCN[n][i]) += barras[n].Fglobal(i);
            }              
        }

        R = R - P; // Este P não pode ter as forças entre os nós, somente aplicadas nos nos 
        
        // std::cout << "\nReações de apoio da estrutura" << " = \n"
        //               << R << std::endl;     
    }
    else
    {
        std::cout << "Decomposição LLT falhou. A matriz pode não ser positiva definida." << std::endl;
    }
}

/**
 * @brief Monta a matriz de rigidez global esparsa da estrutura (SSparse).
 * * Documentação:
 * Este método utiliza uma abordagem de "lista de tripletos" (Eigen::Triplet),
 * que é a forma mais eficiente de construir uma matriz esparsa.
 * 1. Um std::vector<Eigen::Triplet<float>> é criado.
 * 2. Iteramos por todas as barras. Para cada barra, iteramos por sua matriz
 * de rigidez global KGlobal (6x6).
 * 3. Cada elemento (i, j) da KGlobal é mapeado para sua posição (linha, coluna)
 * global na matriz SSparse usando o BCN (Beam Code Number).
 * 4. Um "tripleto" (linha_global, coluna_global, valor) é criado e adicionado
 * ao vetor para cada elemento da KGlobal.
 * 5. Ao final, SSparse.setFromTriplets() é chamada. A Eigen processa essa
 * lista, soma todos os valores que caem na mesma posição (linha, coluna),
 * e constrói a matriz esparsa de forma otimizada.
 */

void Estrutura::calcularMatrizRigidezEstruturaEsparsa()
{
    int numGL = nos.size() * 3;
    Sesparsa.resize(numGL, numGL);

    std::vector<Eigen::Triplet<float>> tripletList;
    tripletList.reserve(barras.size() * 36); // cada barra contribui com

    montarBCN();

    for (size_t n = 0; n < barras.size(); n++)
    {
        const auto& barra = barras[n];
        const auto& bcn = BCN[n];

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                float valor = barra.KGlobal(i, j);
                if (valor != 0.0f)
                {
                    tripletList.emplace_back(Eigen::Triplet<float>(bcn[i], bcn[j], valor));
                }
            }
        }
    }

    Sesparsa.setFromTriplets(tripletList.begin(), tripletList.end());

    // std::cout << "Matriz de rigidez global ESPARSA SSparse (NNZ: " << Sesparsa.nonZeros() << ") = \n"
    //           << Sesparsa << std::endl;
    // std::cout << std::endl;
}

/**
 * @brief Aplica as condições de contorno (vínculos) à matriz esparsa (SSparse)
 * e ao vetor de forças (P).
 *
 * Documentação:
 * Esta função replica a lógica da versão densa, mas com operações em
 * matrizes esparsas, que são mais complexas.
 * 1. Garante que a matriz esteja em formato comprimido (para iteradores).
 * 2. Itera por todos os nós.
 * 3. Se um nó tem um vínculo (ex: fixoX no grau de liberdade 'gln'):
 * a. Zera a força correspondente em P: P(gln) = 0.0f.
 * b. Zera a coluna 'gln': Itera sobre os elementos não-nulos da
 * coluna 'gln' (InnerIterator) e define seus valores para 0.
 * c. Zera a linha 'gln': Itera sobre todas as colunas 'k'. Se o elemento
 * (gln, k) existir (coeffRef != 0), define-o como 0.
 * 4. Após zerar todas as linhas/colunas dos vínculos, iteramos novamente
 * para definir o elemento da diagonal (gln, gln) como 1.0. Isso é
 * feito em um segundo passo (ou no final de cada passo) para garantir
 * que a diagonal não seja zerada por outra restrição.
 * 5. SSparse.prune(0.0f) é chamada para remover fisicamente os elementos
 * zero que foram explicitamente inseridos, mantendo a esparsidade.
 */
void Estrutura::aplicarCondicoesDeContornoEsparsa()
{
    // std::cout << "Aplicando condições de contorno na matriz ESPARSA..." << std::endl;

    Sesparsa.makeCompressed();

    for (int n = 0; n < (int)nos.size(); n++)
    {
        const auto& no = nos[n];
        
        if (no.fixoX)
        {
            int gln = n * 3;
            P(gln) = 0.0f; // Zera vetor de força

            for (Eigen::SparseMatrix<float>::InnerIterator it(Sesparsa, gln); it; ++it) 
            {
                it.valueRef() = 0.0f;
            }
    
            for (int k = 0; k < Sesparsa.outerSize(); ++k) 
            {
                if (Sesparsa.coeffRef(gln, k) != 0)
                    Sesparsa.coeffRef(gln, k) = 0.0f;
            }
        }

        // 3b. Vínculo em Y
        if (no.fixoY)
        {
            int gln = n * 3 + 1;
            P(gln) = 0.0f;
            
            for (Eigen::SparseMatrix<float>::InnerIterator it(Sesparsa, gln); it; ++it) {
                it.valueRef() = 0.0f;
            }
            for (int k = 0; k < Sesparsa.outerSize(); ++k) {
                if (Sesparsa.coeffRef(gln, k) != 0)
                    Sesparsa.coeffRef(gln, k) = 0.0f;
            }
        }

        // 3c. Vínculo em Rotação Z
        if (no.rotaZ)
        {
            int gln = n * 3 + 2;
            P(gln) = 0.0f;
            
            for (Eigen::SparseMatrix<float>::InnerIterator it(Sesparsa, gln); it; ++it) {
                it.valueRef() = 0.0f;
            }
            for (int k = 0; k < Sesparsa.outerSize(); ++k) {
                if (Sesparsa.coeffRef(gln, k) != 0)
                    Sesparsa.coeffRef(gln, k) = 0.0f;
            }
        }
    }

    // 4. Define as diagonais como 1 (após zerar linhas/colunas)
    for (int n = 0; n < (int)nos.size(); n++)
    {
        const auto& no = nos[n];
        if (no.fixoX) {
            Sesparsa.coeffRef(n * 3, n * 3) = 1.0f;
        }
        if (no.fixoY) {
            Sesparsa.coeffRef(n * 3 + 1, n * 3 + 1) = 1.0f;
        }
        if (no.rotaZ) {
            Sesparsa.coeffRef(n * 3 + 2, n * 3 + 2) = 1.0f;
        }
    }

    // 5. Remove os zeros explícitos da matriz
    Sesparsa.prune(0.0f);

    // std::cout << "Matriz Sesparsa (após CC) (NNZ: " << Sesparsa.nonZeros() << ") = \n"
    //           << Sesparsa << std::endl;
    // std::cout << "\nVetor P (após CC) = \n" << P << std::endl;
}

/**
 * @brief Resolve o sistema linear esparso S*d = P.
 *
 * Documentação:
 * 1. Chama os métodos para montar a matriz esparsa (SSparse) e o vetor
 * de forças (P), e aplica as condições de contorno esparsas.
 * 2. Escolhe um solver esparso. Como a matriz de rigidez é simétrica
 * e positiva definida (se a estrutura for estável), usamos o
 * `Eigen::SimplicialLDLT`. É o análogo esparso do `LLT` (Cholesky).
 * 3. `solver.compute(SSparse)`: Analisa a estrutura da matriz (padrão
 * de esparsidade) e calcula a decomposição.
 * 4. `solver.solve(P)`: Resolve o sistema para encontrar o vetor de
 * deslocamentos 'd'.
 * 5. O restante da função (cálculo de deslocamentos, forças e reações
 * nas barras) é idêntico à versão densa, pois apenas utiliza o
 * vetor de resultado 'd' e o 'BCN'.
 */
void Estrutura::resolverSistemaEsparsa()
{
    calcularMatrizRigidezEstruturaEsparsa();
    montarVetorForcas();
    aplicarCondicoesDeContornoEsparsa();

    // 2. Escolher e configurar o solver esparso
    // SimplicialLDLT é bom para matrizes simétricas positivas definidas esparsas
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<float>> solver;
    
    // std::cout << "\nResolvendo sistema ESPARSO SSparse * d = P..." << std::endl;

    // 3. Computar a decomposição
    solver.compute(Sesparsa);

    if (solver.info() != Eigen::Success)
    {
        std::cout << "Decomposição LDLT (Esparsa) falhou. A matriz pode ser singular." << std::endl;
        return;
    }

    // 4. Resolver o sistema
    d = solver.solve(P);

    if (solver.info() != Eigen::Success)
    {
        std::cout << "Resolução do sistema (Esparsa) falhou." << std::endl;
        return;
    }

    // std::cout << "\nDeslocamentos d = \n" << d << std::endl;

    // 5. O restante é idêntico ao 'resolverSistema' denso
    for (size_t n = 0; n < barras.size(); n++)
    {
        barras[n].calcularDeslocamentosGlobais(d, BCN[n]);
        // std::cout << "\nDeslocamentos globais (esparso) da barra " << n << " = \n"
        //           << barras[n].vGlobal << std::endl;

        barras[n].calcularForcasGlobais();
        // std::cout << "\nForças globais (esparso) da barra " << n << " = \n"
        //           << barras[n].Fglobal << std::endl;

        barras[n].calcularEsforcosLocais();
        // std::cout << "\nEsforços locais (esparso) da barra " << n << " = \n"
        //           << barras[n].fLocal << std::endl;

        for (int i = 0; i < 6; i++)
        {
            R(BCN[n][i]) += barras[n].Fglobal(i);
        }              
    }

    R = R - P; 
    
    // std::cout << "\nReações de apoio da estrutura (esparso)" << " = \n"
    //               << R << std::endl;     
}
