#figure(
  table(
    columns: (auto, auto, auto, auto, auto, auto, auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Contêiner*], [*Tipos*], [*Resolvidas*], [*Limite de busca*], [*Timeout*], [*Falhas*], [*Tempo médio (s)*], [*Mediana (s)*], [*Máximo (s)*],
    [fraco], [10×10×10], [5], [1/1], [0], [0], [0], [0.000], [0.000], [0.000],
    [medio], [15×10×10], [10], [1/1], [0], [0], [0], [0.001], [0.001], [0.001],
    [forte], [20×20×20], [12], [1/1], [0], [0], [0], [0.007], [0.007], [0.007],
  ),
  caption: [Desempenho do algoritmo exato nos cenários de estresse (limite de 3600 s por instância). Os tempos consideram apenas instâncias resolvidas.],
) <tab-estresse-tempo>

#figure(
  table(
    columns: (auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Volume oferecido / contêiner (%)*], [*Ocupação média do ótimo (%)*], [*Lacuna média para o limite da mochila (%)*],
    [fraco], [32.4], [32.4], [0.00],
    [medio], [71.2], [71.2], [0.00],
    [forte], [55.0], [55.0], [0.00],
  ),
  caption: [Qualidade das soluções nos cenários de estresse. A lacuna é a diferença entre o volume do primeiro candidato da mochila (limite superior) e o volume da solução ótima.],
) <tab-estresse-qualidade>
