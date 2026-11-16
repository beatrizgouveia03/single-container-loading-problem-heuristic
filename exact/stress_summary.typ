#figure(
  table(
    columns: (auto, auto, auto, auto, auto, auto, auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Contêiner*], [*Tipos*], [*Resolvidas*], [*Limite de busca*], [*Timeout*], [*Falhas*], [*Tempo médio (s)*], [*Mediana (s)*], [*Máximo (s)*],
    [fraco], [10×10×10], [5], [1/1], [0], [0], [0], [0.169], [0.169], [0.169],
    [medio], [15×10×10], [10], [1/1], [0], [0], [0], [0.001], [0.001], [0.001],
    [forte], [20×20×20], [12], [0/1], [1], [0], [0], [-], [-], [-],
  ),
  caption: [Desempenho do algoritmo exato nos cenários de estresse (limite de 3600 s por instância). Os tempos consideram apenas instâncias resolvidas.],
) <tab-estresse-tempo>

#figure(
  table(
    columns: (auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Volume oferecido / contêiner (%)*], [*Ocupação média do ótimo (%)*], [*Lacuna média para o limite da mochila (%)*],
    [fraco], [146.9], [74.6], [25.40],
    [medio], [44.1], [44.1], [0.00],
    [forte], [99.0], [-], [-],
  ),
  caption: [Qualidade das soluções nos cenários de estresse. A lacuna é a diferença entre o volume do primeiro candidato da mochila (limite superior) e o volume da solução ótima.],
) <tab-estresse-qualidade>
