# Guia de integração do editor com análise estrutural e resultados

## Objetivo

Este documento apresenta a interface atual do editor, o fluxo principal do código e os pontos em que novas funcionalidades de **análise estrutural**, **imperfeição inicial** e **visualização de resultados** devem ser integradas.

O foco aqui é prático: ao final da leitura, a equipe deve saber:

- como o editor funciona hoje;
- onde ficam os dados do modelo estrutural;
- onde adicionar novas abas superiores como **Imperfeição Inicial**, **Análise** e **Resultados**;
- onde colocar o motor numérico;
- onde fazer a renderização dos resultados no viewport;
- o que precisa ser atualizado quando novos dados passam a fazer parte do projeto.

---

## 1. Visão geral do editor

O programa é um editor gráfico 2D para montagem do modelo estrutural. Ele já possui:

- criação e remoção de nós;
- criação e remoção de barras;
- seleção simples e por janela;
- movimentação, cópia e espelhamento de seleção;
- definição de apoios;
- aplicação e remoção de carga nodal;
- aplicação e remoção de carga distribuída em barras;
- cadastro e atribuição de materiais e seções;
- cotas;
- visualização da estrutura, cargas e elementos auxiliares.

Hoje, o editor ainda **não possui uma camada dedicada de análise estrutural pronta**. A integração da análise deverá ser adicionada sobre a arquitetura existente.

---

## 2. Interface atual

### 2.1 Barra superior

A barra superior é criada em `src/ui/TopToolbar.cpp`.

Ela possui atualmente três abas.

#### Aba **Página Inicial**
Contém os grupos:

- **Nós**: adicionar e remover;
- **Barras**: adicionar e remover;
- **Editar**: mover, copiar e espelhar;
- **Apoios**: nenhum, X, Y, XY e engaste;
- **Cotas**: adicionar e ajustar, além de opções de unidade e offset;
- **Visualização**: ligar/desligar nós, barras, cotas, materiais, seções, sombras e fade de texto.

#### Aba **Carregamentos**
Contém os grupos:

- **Concentrado em nó**: edição de `Fx`, `Fy` e `Mz`, além de ativação das ferramentas de adicionar/remover carga nodal;
- **Distribuída na barra**: edição de `qxStart`, `qyStart`, `qxEnd`, `qyEnd`, além de ativação das ferramentas de adicionar/remover carga distribuída;
- **Visualização**: opções de exibição das cargas, escala gráfica e unidades de exibição.

#### Aba **Materiais e Seções**
Contém os grupos:

- **Material**: seleção, criação, edição e remoção;
- **Seção**: seleção, criação, edição e remoção.

### 2.2 Painel esquerdo

O painel esquerdo é criado em `src/ui/LeftPanel.cpp`.

Hoje ele possui o grupo **Auxílios**, com:

- `Grade`;
- `Guias`;
- `Snap`.

Esse painel hoje é simples. Em princípio, novas interfaces de análise e resultados devem entrar primeiro como **novas abas na barra superior**, e não neste painel.

### 2.3 Viewport central

O viewport central é a área onde a estrutura é desenhada e editada.

É nele que o usuário:

- clica para criar nós e barras;
- seleciona entidades;
- arrasta janelas de seleção;
- aplica apoios e cargas;
- vê cotas, cargas, sombras, guias e demais elementos gráficos.

### 2.4 Overlay de status

Na parte inferior do viewport existe um quadro de status desenhado pelo editor. Ele informa:

- a ferramenta ativa;
- a etapa atual da operação;
- a ação esperada do usuário.

A lógica está em `src/editor/Editor.StatusOverlay.cpp`.

### 2.5 Diálogos modais

Os diálogos atuais ficam em `src/ui/AppDialogs.*`.

Hoje existem diálogos para:

- material;
- seção;
- distância de barra.

Se no futuro alguma configuração de análise exigir uma janela modal, esse é o subsistema que deve ser estendido.

---

## 3. Atalhos atuais importantes

Os principais atalhos do editor hoje são:

- `N`: adicionar nó;
- `B`: adicionar barra;
- `D`: adicionar cota;
- `M`: mover seleção;
- `C`: copiar seleção;
- `E`: espelhar seleção;
- `S`: liga/desliga snap;
- `G`: liga/desliga grade ou guias;
- `Delete`: remove seleção;
- `Esc`: cancela ferramenta, operação, guia ou seleção, conforme o contexto;
- `Ctrl+Z`: desfazer;
- `Ctrl+Y` ou `Ctrl+Shift+Z`: refazer;
- `Ctrl+S`: salva em `saved-project.json`;
- `Ctrl+L`: carrega `saved-project.json`.

Na seleção:

- clique simples: substitui a seleção;
- `Ctrl`: adiciona à seleção;
- `Shift`: remove da seleção.

---

## 4. Estrutura central do código

Os arquivos mais importantes para quem vai integrar análise e resultados são estes.

### `src/app/Application.*`

A classe `Application` coordena o frame inteiro:

1. monta a interface;
2. coleta pedidos da interface;
3. aplica edições vindas da UI;
4. chama `editor.Update(...)`;
5. chama `editor.Render()`.

Ela é o ponto certo para tratar ações da interface que **não pertencem ao fluxo direto de clique no viewport**.

### `src/ui/TopToolbar.*`

É a origem das abas superiores. Ela **não deveria executar análise nem alterar diretamente o documento**. O papel dela é:

- desenhar a interface;
- preencher um pacote de pedidos (`FrameRequests`) com a intenção do usuário.

### `src/ui/LeftPanel.*`

Painel auxiliar simples. Hoje altera diretamente parte do estado visual do editor.

### `src/ui/AppDialogs.*`

Sistema de diálogos modais. Recebe um pedido de abertura, desenha o modal e devolve um resultado estruturado.

### `src/editor/FrameRequests.h`

É um arquivo central para extensões da UI.

Ele define:

- `EditorRequest`: pedidos ligados a ferramentas do editor;
- `DocumentRequest`: pedidos que alteram o documento;
- outras structs de pedido usadas pela toolbar.

Quando vocês adicionarem botões ou abas novas, muito provavelmente terão que criar novas structs aqui.

### `src/editor/Editor.*`

A classe `Editor` é o núcleo do viewport. Ela contém:

- `document`: o modelo persistente do projeto;
- `state`: o estado transitório da interação;
- `derivedData`: cache derivado do documento;
- `camera`: câmera do viewport;
- renderização;
- histórico de undo/redo.

### `src/model/*`

Contém os dados persistentes do projeto:

- `Node`;
- `Beam`;
- `BeamDistributedLoad` / `DistributedLoadValue`;
- `StructuralMaterial`;
- `Section`;
- `Dimension`;
- `ProjectDocument`.

### `src/editor/cache/ProjectDerivedData.*`

Guarda dados derivados do documento, usados principalmente para renderização e decisões gráficas.

### `src/editor/render/*` e `src/editor/EditorRenderer.*`

É o subsistema de desenho do viewport.

Hoje o renderer está organizado em passes:

- fundo;
- estrutura;
- overlays;
- cotas;
- previews.

Esse é o ponto natural para desenhar deslocamentos, deformadas, diagramas, cores de resultados e outros pós-processamentos.

### `src/editor/ops/*`

Contém operações sobre o documento e sobre a seleção. Se uma nova aba modificar o modelo, a lógica de modificação idealmente deve terminar em alguma operação estruturada daqui, ou em um novo módulo equivalente.

---

## 5. Fluxo real do programa por frame

O fluxo principal atual é este:

1. `Application::Render()` chama `BuildUiFrameEdits(...)`;
2. `TopToolbar` e `LeftPanel` desenham a interface;
3. `TopToolbar` preenche `FrameRequests`;
4. `AppDialogs` desenha os modais e devolve `DialogFrameResult`;
5. `Application::ApplyUiFrameEdits(...)` aplica as mudanças vindas da interface;
6. `editor.Update(...)` trata interação do viewport, atalhos, ferramentas e cliques;
7. `editor.Render()` desenha o viewport.

A regra prática é:

- **UI superior e diálogos** produzem pedidos;
- **Application** decide como aplicar esses pedidos;
- **Editor** trata a interação do canvas;
- **Renderer** só desenha.

Essa separação deve ser mantida também para análise e resultados.

---

## 6. Onde estão os dados que a análise vai consumir

O motor de análise deve usar `ProjectDocument` como fonte principal de entrada.

### 6.1 Nós

Arquivo: `src/model/Node.h`

Cada `Node` contém:

- `id`;
- `position` (`x`, `y`);
- `support`;
- `load` com `fx`, `fy`, `mz`.

### 6.2 Barras

Arquivo: `src/model/Beam.h`

Cada `Beam` contém:

- `id`;
- `startNodeId`;
- `endNodeId`;
- `materialId`;
- `sectionId`.

A orientação `start -> end` é importante. Se o solver usar eixos locais, essa orientação define naturalmente o eixo local da barra.

### 6.3 Cargas distribuídas

Arquivo: `src/model/DistributedLoad.h`

Cada carga distribuída está associada a um `beamId` e possui:

- `qxStart`, `qyStart`;
- `qxEnd`, `qyEnd`.

Esses valores estão armazenados como componentes associadas à barra. Se o solver trabalhar em sistema local, vocês terão que converter essas componentes para o sistema local da barra no adaptador de entrada da análise.

### 6.4 Materiais

Arquivo: `src/model/StructuralMaterial.h`

Cada material possui:

- `youngModulus`;
- `thermalExpansion`.

### 6.5 Seções

Arquivo: `src/model/Section.h`

Cada seção possui:

- `area`;
- `inertia`.

### 6.6 Unidades

Arquivo: `src/model/DisplayUnits.h`

As unidades de exibição servem para a interface. O código já faz conversões de exibição usando `src/utils/UnitConversion.*`.

Para a análise, adotem a convenção interna do projeto:

- comprimento em base SI;
- força em N;
- momento em N·m;
- carga distribuída em N/m;
- tensão em Pa;
- módulo em Pa;
- área em m²;
- inércia em m⁴.

Em outras palavras: a análise deve trabalhar com os valores internos do documento, não com os textos formatados da interface.

---

## 7. Regra prática: onde colocar cada tipo de informação nova

Essa decisão é a mais importante para evitar código confuso.

### Coloque em `ProjectDocument` quando:

o dado faz parte do **modelo do projeto** e deve ser salvo/carregado.

Exemplos:

- parâmetros de imperfeição inicial que pertencem ao modelo;
- tipo de análise escolhido, se ele for parte do projeto salvo;
- conjuntos de carregamento, se forem persistentes;
- propriedades adicionais de elementos.

### Coloque em `EditorState` ou em um novo estado do editor quando:

o dado é apenas de **interação** ou **visualização temporária**.

Exemplos:

- qual resultado está sendo exibido;
- escala gráfica da deformada;
- mostrar ou não mostrar valores numéricos;
- tipo de mapa de cores selecionado.

### Não coloquem resultados numéricos do solver dentro de `ProjectDocument` por padrão

Deslocamentos, esforços internos, reações, tensões e outros resultados:

- não precisam entrar no undo/redo do modelo;
- não deveriam ser confundidos com a geometria original;
- em geral não precisam ser serializados no arquivo do projeto na primeira versão.

A recomendação para este projeto é:

- manter o **solver** em uma pasta própria, por exemplo `src/analysis/`;
- manter o **estado e os resultados correntes da análise** associados ao `Editor`.

Isso reduz acoplamento com a UI e simplifica a invalidação dos resultados quando o modelo muda.

---

## 8. Organização recomendada para a análise

Sugestão de novos arquivos:

```text
src/analysis/
  AnalysisInput.h
  AnalysisInputBuilder.h
  AnalysisInputBuilder.cpp
  AnalysisOptions.h
  AnalysisResult.h
  StructuralAnalysisSolver.h
  StructuralAnalysisSolver.cpp
```

### Papel de cada arquivo sugerido

#### `AnalysisInput.h`
Define estruturas simples de entrada para o solver.

Exemplo de conteúdo esperado:

- lista de nós;
- lista de elementos;
- vínculos;
- cargas nodais;
- cargas distribuídas convertidas para o formato do solver;
- materiais e seções já resolvidos.

#### `AnalysisInputBuilder.*`
Converte `ProjectDocument` em `AnalysisInput`.

Esse adaptador é importante para não misturar a lógica do editor com a lógica numérica.

#### `AnalysisOptions.h`
Define parâmetros de execução da análise.

Exemplos:

- tipo de análise;
- número de incrementos;
- tolerâncias;
- incluir ou não imperfeição inicial;
- parâmetros da imperfeição.

#### `AnalysisResult.h`
Define a saída do solver.

Exemplo de conteúdo esperado:

- deslocamentos nodais;
- reações de apoio;
- esforços por barra;
- rotações;
- tensões;
- deformada;
- campos escalares usados pela renderização.

#### `StructuralAnalysisSolver.*`
Implementa o motor numérico propriamente dito.

Esse módulo deve ficar **sem dependência de ImGui e sem dependência de Raylib**.

---

## 9. Organização recomendada para os resultados dentro do editor

Sugestão prática para este projeto:

1. criar uma estrutura nova dentro do editor, por exemplo:

```cpp
struct AnalysisState
{
    bool hasResults = false;
    bool resultsOutdated = false;
    AnalysisOptions options;
    AnalysisResult result;

    enum class DisplayMode
    {
        None,
        DeformedShape,
        AxialForce,
        ShearForce,
        BendingMoment,
        Reaction,
        Stress
    } displayMode = DisplayMode::None;

    float deformationScale = 1.0f;
    bool showValues = false;
};
```

2. adicionar esse estado como membro de `Editor`;
3. fazer o `Editor` invalidar os resultados quando o documento for modificado;
4. fazer o `EditorRenderer` ler esse estado para desenhar o pós-processamento.

### Por que isso é útil aqui

Porque o `Editor` já é o ponto que:

- conhece o documento;
- sabe quando o documento mudou;
- chama o renderer;
- mantém o viewport.

Isso evita espalhar o estado da análise por muitos lugares.

---

## 10. Como adicionar novas abas superiores

### 10.1 Onde modificar

Arquivos principais:

- `src/ui/TopToolbar.h`
- `src/ui/TopToolbar.cpp`
- `src/editor/FrameRequests.h`
- `src/app/Application.h`
- `src/app/Application.DocumentEdits.cpp`
- eventualmente `src/ui/AppDialogs.*` se houver modal

### 10.2 Passo mínimo para criar uma nova aba

#### No header da toolbar

Adicionar novos métodos, por exemplo:

```cpp
void DrawImperfectionTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState);

void DrawAnalysisTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState);

void DrawResultsTab(
    FrameRequests& requests,
    ProjectDocument& projectDocument,
    EditorState& editorState);
```

#### No `TopToolbar::FillRequests(...)`

Adicionar novas abas ao `TabBar`:

```cpp
if (ImGui::BeginTabItem("Imperfeição Inicial"))
{
    DrawImperfectionTab(requests, projectDocument, editorState);
    ImGui::EndTabItem();
}

if (ImGui::BeginTabItem("Análise"))
{
    DrawAnalysisTab(requests, projectDocument, editorState);
    ImGui::EndTabItem();
}

if (ImGui::BeginTabItem("Resultados"))
{
    DrawResultsTab(requests, projectDocument, editorState);
    ImGui::EndTabItem();
}
```

### 10.3 Não executar a análise diretamente dentro da toolbar

A toolbar é desenhada a cada frame. Portanto:

- não coloquem algoritmo numérico em `TopToolbar.cpp`;
- não alterem diretamente o documento a partir de widgets complexos;
- usem `FrameRequests` para representar a intenção do usuário.

### 10.4 Criar requests novos

Em `src/editor/FrameRequests.h`, adicionem structs específicas para o que a nova aba precisa.

Exemplo:

```cpp
struct AnalysisRequest
{
    bool active = false;
    bool run = false;
    bool clear = false;
};

struct ResultViewRequest
{
    bool active = false;
    bool setDisplayMode = false;
    bool setScale = false;
    int displayMode = 0;
    float scale = 1.0f;
};
```

Depois, adicionem esses campos dentro de `FrameRequests`.

### 10.5 Aplicar os requests na aplicação

Criem métodos em `Application` para tratar esses pedidos.

Exemplo:

```cpp
void Application::ApplyAnalysisRequest(const FrameRequests::AnalysisRequest& request)
{
    if (!request.active)
    {
        return;
    }

    if (request.run)
    {
        editor.RunAnalysis();
    }

    if (request.clear)
    {
        editor.ClearAnalysisResults();
    }
}
```

A ideia é manter o padrão já usado hoje:

- a UI monta o pedido;
- `Application` aplica;
- o `Editor` executa o que for do seu domínio.

---

## 11. Como integrar **Imperfeição Inicial**

Essa aba pode representar duas coisas diferentes, e a decisão muda onde os dados devem ficar.

### Caso A — a imperfeição é um parâmetro persistente do projeto

Exemplo:

- amplitude de imperfeição;
- forma adotada;
- direção;
- fator por barra ou global.

Nesse caso, a recomendação é:

1. adicionar uma estrutura nova em `ProjectDocument`, por exemplo `InitialImperfectionSettings`;
2. criar requests para edição desses dados;
3. aplicar a edição em `Application` e/ou em um módulo de operações;
4. atualizar persistência e histórico.

Arquivos que também terão que ser atualizados:

- `src/editor/Editor.Persistence.cpp`
- `src/editor/Editor.DocumentHistory.cpp`

### Caso B — a imperfeição é apenas uma variação temporária para a análise

Se a imperfeição for somente um parâmetro temporário de execução, ela pode ficar em um estado de análise, fora do documento.

### Recomendação prática

Se o valor precisa ser salvo junto com o projeto, coloquem em `ProjectDocument`.
Se for apenas um teste temporário da sessão atual, coloquem em um estado de análise.

### Importante

Não deformem `document.nodes` para representar a imperfeição inicial.

A geometria original do modelo deve continuar intacta. A imperfeição deve ser tratada como:

- dado adicional de entrada para o solver; ou
- uma geometria auxiliar gerada no momento da análise/renderização.

---

## 12. Como integrar a execução da análise

### 12.1 Método sugerido no editor

Criem métodos explícitos no `Editor`, por exemplo:

```cpp
bool RunAnalysis();
void ClearAnalysisResults();
void InvalidateAnalysisResults();
```

### 12.2 O que `RunAnalysis()` deve fazer

Fluxo sugerido:

1. ler `document`;
2. montar `AnalysisInput` usando `AnalysisInputBuilder`;
3. chamar o solver;
4. armazenar o `AnalysisResult` no estado de análise do editor;
5. marcar `hasResults = true` e `resultsOutdated = false`.

### 12.3 Invalidação dos resultados

Sempre que o modelo estrutural mudar, os resultados antigos devem ser invalidados.

Os pontos principais para isso são:

- `Editor::RecordDocumentChange(...)`
- `Editor::RecordExternalDocumentChange(...)`
- `Editor::RestoreDocumentSnapshot(...)`

Nesses pontos, chamem algo como:

```cpp
InvalidateAnalysisResults();
```

Assim, o programa deixa claro que os resultados já não correspondem ao modelo atual.

### 12.4 Não rodar a análise a cada frame

A análise deve ser executada apenas quando o usuário solicitar explicitamente, por exemplo com um botão **Executar** na aba **Análise**.

---

## 13. Como renderizar os resultados

### 13.1 Onde desenhar

O ponto principal é `src/editor/EditorRenderer.*`.

Hoje o renderer já é separado em passes. A recomendação é inserir um novo pass de resultados.

Exemplo de ideia:

```cpp
void EditorRenderer::Render(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const EditorState& state,
    const AnalysisState* analysis,
    const Camera2D& camera,
    float zoomTarget)
{
    RenderBackgroundPass(document, derivedData, state, camera);
    RenderStructurePass(document, state, camera);
    RenderResultsPass(document, state, analysis, camera);
    RenderOverlayPass(document, derivedData, state, camera, zoomTarget);
    RenderDimensionPass(document, state, camera);
    RenderPreviewPass(document, state, camera);
}
```

### 13.2 O que normalmente entra em `RenderResultsPass`

Exemplos:

- deformada da estrutura;
- mapa de cor por barra;
- diagramas de esforço;
- reações de apoio;
- valores numéricos próximos aos nós ou barras.

### 13.3 Regra para escolher o tipo de pass

#### Resultado gráfico ligado à geometria da estrutura
Use um pass próprio entre a estrutura e os overlays.

Exemplos:

- deformada;
- diagrama de momento;
- cor por elemento.

#### Resultado textual ou anotação leve
Pode ser desenhado como overlay.

Exemplos:

- valor de reação ao lado do apoio;
- valor máximo destacado;
- legenda simples.

### 13.4 Recomendação para visualização de resultados

Criem também um pequeno estado de visualização de resultados, por exemplo:

- modo de exibição atual;
- escala da deformada;
- mostrar ou não valores;
- opacidade;
- valor mínimo/máximo para mapa de cores.

Esse estado pode ficar:

- dentro de `AnalysisState`; ou
- em uma struct separada de visualização associada ao editor.

---

## 14. Recomendação de arquitetura para a primeira integração

Para este projeto, a abordagem mais simples e consistente é esta:

### O que fica em `src/analysis/`

- toda a matemática;
- montagem de matrizes;
- solução do sistema;
- cálculo de esforços, deslocamentos e tensões;
- conversão do documento em entrada do solver.

### O que fica em `Editor`

- estado atual da análise;
- resultado mais recente;
- invalidação dos resultados quando o modelo muda;
- ponte entre documento e renderer.

### O que fica em `TopToolbar`

- botões e combos de **Imperfeição Inicial**, **Análise** e **Resultados**;
- geração de `FrameRequests`.

### O que fica em `Application`

- aplicação dos novos requests vindos da interface;
- chamada de métodos de análise do editor;
- coordenação entre UI e núcleo do editor.

### O que fica em `EditorRenderer`

- desenho dos resultados no viewport.

---

## 15. Checklist mínimo para adicionar um novo dado persistente ao projeto

Sempre que vocês adicionarem um dado novo em `ProjectDocument`, verifiquem estes pontos:

1. adicionar o campo no modelo;
2. atualizar a serialização em `src/editor/Editor.Persistence.cpp`;
3. atualizar a leitura em `src/editor/Editor.Persistence.cpp`;
4. atualizar a assinatura do documento em `src/editor/Editor.DocumentHistory.cpp`;
5. atualizar a UI que edita esse dado;
6. invalidar resultados de análise, se esse dado influenciar o solver.

Se esse checklist não for seguido, é comum acontecer um destes problemas:

- o dado aparece na interface mas não é salvo;
- o undo/redo ignora a alteração;
- a análise usa valores defasados;
- o render mostra resultado incompatível com o modelo.

---

## 16. Checklist mínimo para adicionar uma nova aba de análise

### Aba `Imperfeição Inicial`

- criar `DrawImperfectionTab(...)` em `TopToolbar`;
- criar request próprio em `FrameRequests.h`;
- decidir se o dado vai para `ProjectDocument` ou para um estado de análise;
- aplicar o request em `Application`.

### Aba `Análise`

- criar `DrawAnalysisTab(...)`;
- criar `AnalysisRequest`;
- adicionar botão `Executar`;
- implementar `editor.RunAnalysis()`;
- invalidar resultados quando o modelo mudar.

### Aba `Resultados`

- criar `DrawResultsTab(...)`;
- criar `ResultViewRequest`;
- permitir troca do modo de visualização;
- ligar esse modo ao `RenderResultsPass`.

---

## 17. Regras práticas para não degradar a arquitetura

1. **Não coloquem solver dentro da toolbar.**
2. **Não coloquem Raylib/ImGui dentro do solver.**
3. **Não usem `ProjectDocument` para guardar resultados temporários.**
4. **Não alterem a geometria original do documento para mostrar deformada ou imperfeição.**
5. **Não esqueçam de invalidar resultados quando o modelo mudar.**
6. **Se um dado novo pertence ao projeto, atualizem persistência e histórico.**
7. **Se um resultado depende de seleção ou modo visual, tratem isso como estado do editor, não como dado do documento.**

---

## 18. Arquivos que vocês provavelmente vão editar

### Para novas abas e controles

- `src/ui/TopToolbar.h`
- `src/ui/TopToolbar.cpp`
- `src/editor/FrameRequests.h`
- `src/app/Application.h`
- `src/app/Application.DocumentEdits.cpp`

### Para novos dados persistentes

- `src/model/ProjectDocument.h`
- eventualmente novos arquivos em `src/model/`
- `src/editor/Editor.Persistence.cpp`
- `src/editor/Editor.DocumentHistory.cpp`

### Para análise estrutural

- novos arquivos em `src/analysis/`
- `src/editor/Editor.h`
- `src/editor/Editor.cpp`

### Para renderização dos resultados

- `src/editor/EditorRenderer.h`
- `src/editor/EditorRenderer.cpp`
- novos arquivos em `src/editor/render/`

---

## 19. Ponto de partida recomendado para a turma

Uma sequência segura de trabalho seria:

1. criar as abas **Imperfeição Inicial**, **Análise** e **Resultados** apenas com controles básicos;
2. criar as structs de request correspondentes;
3. criar o módulo `src/analysis/` com entrada, opções e resultado;
4. implementar um `RunAnalysis()` mínimo que leia o documento e produza um resultado simples;
5. desenhar primeiro uma visualização simples, por exemplo:
   - deslocamentos nodais; ou
   - deformada sem escala automática; ou
   - valores de reação nos apoios;
6. só depois evoluir para diagramas e mapas de cores.

Essa ordem reduz o risco de misturar UI, solver e renderização cedo demais.

---

## 20. Resumo final

Para trabalhar corretamente neste projeto, usem estas referências mentais:

- **TopToolbar**: monta a interface superior e gera pedidos;
- **Application**: recebe esses pedidos e coordena a execução;
- **ProjectDocument**: guarda o modelo estrutural persistente;
- **Editor**: centraliza viewport, estado, histórico e integração com análise;
- **src/analysis**: contém o motor numérico;
- **EditorRenderer**: desenha os resultados.

Em termos práticos:

- novas abas superiores entram em `TopToolbar`;
- novos pedidos de UI entram em `FrameRequests`;
- dados persistentes entram em `ProjectDocument`;
- solver entra em `src/analysis`;
- resultados correntes ficam associados ao editor;
- desenho dos resultados entra no renderer.

Esse é o caminho mais compatível com a estrutura atual do projeto e o mais direto para integrar análise estrutural e pós-processamento sem desorganizar o editor.
