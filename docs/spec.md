# Especificação Funcional: Análise Estrutural 2D

## 1. Escopo
O objetivo é implementar um motor de análise estrutural linear e não-linear para pórticos planos (2D), integrado ao editor gráfico existente.

## 2. Requisitos Funcionais
### 2.1 Configuração de Análise
- Permitir a escolha entre análise linear (1ª ordem) e não-linear (2ª ordem).
- Configurar parâmetros de convergência (tolerância, número máximo de iterações).
- Definir imperfeições iniciais globais e locais (L/500, etc.).

### 2.2 Execução
- Transformar o modelo geométrico (nós, barras) em um modelo matemático.
- Resolver o sistema de equações globais $[K]\{u\} = \{F\}$.
- No caso não-linear, realizar o processo iterativo (Newton-Raphson).

### 2.3 Resultados (Saídas)
- **Deslocamentos**: Translações (X, Y) e rotações (Z) em cada nó.
- **Reações de Apoio**: Forças (X, Y) e momentos (Z) nos apoios.
- **Esforços Internos**: Força Axial (N), Força Cortante (V) e Momento Fletor (M) ao longo das barras.

## 3. Requisitos Não-Funcionais
- **Performance**: O solver deve utilizar a biblioteca **Eigen** para operações de álgebra linear.
- **Desacoplamento**: O motor numérico não deve depender de bibliotecas de interface gráfica (Raylib/ImGui).
- **Unidades**: Todo o cálculo interno deve ser realizado em unidades do SI (N, m, Pa).
