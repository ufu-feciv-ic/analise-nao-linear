# Design Técnico: Integração de Análise Estrutural

## 1. Módulo `src/analysis/`
Responsável pela lógica numérica pura.

- **`AnalysisInput.h`**: DTO (Data Transfer Object) com os dados prontos para o solver (vetores e matrizes).
- **`AnalysisInputBuilder.cpp`**: Traduz o `ProjectDocument` (objetos do editor) para o `AnalysisInput`.
- **`StructuralAnalysisSolver.cpp`**: Núcleo matemático usando **Eigen**. Realiza montagem da matriz de rigidez global e solução do sistema.
- **`AnalysisResult.h`**: Armazena os resultados finais (deslocamentos, esforços, reações).

## 2. Fluxo de Dados e Integração no `Editor`
O `Editor` gerencia o ciclo de vida da análise:

1. **Pedido (Request)**: A `TopToolbar` gera um `AnalysisRequest` via `FrameRequests`.
2. **Execução**: A `Application` captura o pedido e chama `Editor::RunAnalysis()`.
3. **Invalidação**: O `Editor` marca os resultados como obsoletos sempre que o `ProjectDocument` é alterado.
4. **Renderização**: O `EditorRenderer` recebe o `AnalysisResult` para desenhar os diagramas e a deformada no `RenderResultsPass`.

## 3. Visualização de Resultados
O `Editor` manterá um `AnalysisState` com:
- `displayMode`: (Nenhum, Deformada, Axial, Cortante, Momento, Reação).
- `deformationScale`: Fator de escala para visualização.
- `showValues`: Toggle para exibir valores numéricos nas barras/nós.
