#figure(
  table(
    columns: (auto, auto, auto, auto, auto, auto, auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Contêiner*], [*Tipos*], [*Resolvidas*], [*Limite de busca*], [*Timeout*], [*Falhas*], [*Tempo médio (s)*], [*Mediana (s)*], [*Máximo (s)*],
    [fraco], [10×10×10], [5], [1/1], [0], [0], [0], [0.001], [0.001], [0.001],
    [medio], [15×10×10], [10], [1/1], [0], [0], [0], [0.004], [0.004], [0.004],
    [forte], [20×20×20], [12], [1/1], [0], [0], [0], [0.041], [0.041], [0.041],
  ),
  caption: [Desempenho do algoritmo exato nos cenários de estresse (limite de 3600 s por instância). Os tempos consideram apenas instâncias resolvidas.],
) <tab-estresse-tempo>

#figure(
  table(
    columns: (auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Volume oferecido / contêiner (%)*], [*Ocupação média do ótimo (%)*], [*Lacuna média para o limite da mochila (%)*],
    [fraco], [12.0], [12.0], [0.00],
    [medio], [67.5], [67.5], [0.00],
    [forte], [65.0], [65.0], [0.00],
  ),
  caption: [Qualidade das soluções nos cenários de estresse. A lacuna é a diferença entre o volume do primeiro candidato da mochila (limite superior) e o volume da solução ótima.],
) <tab-estresse-qualidade>
