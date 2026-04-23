# Backlog de Implementação: Análise Estrutural

## Fase 1: Interface e Comunicação 🏗️
- [ ] Adicionar `AnalysisRequest` e `ResultsViewRequest` em `FrameRequests.h`.
- [ ] Implementar abas "Imperfeição", "Análise" e "Resultados" em `TopToolbar.cpp`.
- [ ] Implementar handlers em `Application.cpp` para tratar os requests da UI.

## Fase 2: Infraestrutura de Análise (`src/analysis/`) 🧬
- [ ] Definir `AnalysisOptions.h` e `AnalysisResult.h`.
- [ ] Implementar `AnalysisInputBuilder` para converter o documento em dados SI.
- [ ] Criar estrutura base do `StructuralAnalysisSolver` com Eigen.

## Fase 3: Integração no Editor 🔄
- [ ] Adicionar `AnalysisState` ao `Editor.h`.
- [ ] Implementar `Editor::RunAnalysis()`.
- [ ] Implementar lógica de invalidação de resultados em `Editor::RecordDocumentChange`.

## Fase 4: Renderização de Resultados 🎨
- [ ] Criar `RenderResultsPass` em `EditorRenderer.cpp`.
- [ ] Implementar visualização da estrutura deformada.
- [ ] Implementar visualização de diagramas (N, V, M).

## Fase 5: Análise Não-Linear e Refinamentos 🚀
- [ ] Implementar imperfeições iniciais no solver.
- [ ] Implementar loop Newton-Raphson para não-linearidade geométrica.
- [ ] Adicionar persistência das configurações de análise no JSON do projeto.
