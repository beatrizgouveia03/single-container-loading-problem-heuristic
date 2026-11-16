# Timeout do teste de estresse: diagnóstico e plano

Data: 2026-11-15 · branch `dev/exact_algorithm` · binário `exact/build/test/StressTest`

## 1. Resumo

O limite de parede é respeitado: o filho é morto no `timeout_s` e a linha sai
como `timeout`. O problema é que **o tempo é todo gasto na checagem geométrica
de viabilidade**, e não há prazo dentro do solver. Duas formas de estouro
aparecem nas mesmas 12 instâncias:

| instância | volume oferecido | candidato da mochila | caixas | resultado (orçamento 10 s) |
| --- | --- | --- | --- | --- |
| 1 | 808 (81 %) | 808 (81 %) | 20 | **1 pop em 11,8 s** — a única checagem não terminou |
| 2 | 5006 (501 %) | 1000 (100 %) | 9 | **408 pops em 10,1 s** — 9,5 s só de checagens |
| 5 | 1542 (154 %) | 1000 (100 %) | 10 | resolvida em 2,7 s |
| 12 | 4123 (412 %) | 1000 (100 %) | 6 | resolvida em 2,9 s |
| 3, 6–11 | 188 %–823 % | 995–1000 (99–100 %) | 3–9 | resolvidas em 7–547 ms |
| 4 | 7296 (730 %) | 1000 (100 %) | 1 | resolvida em 1 ms |

Com orçamento de 10 s por instância, **10 das 12 resolvem em menos de 3 s**
(mediana ~90 ms) e **2 estouram**. Ou seja: não é "toda instância que não cabe
estoura", e o volume
oferecido **não é o preditor** — a instância 4 tem 730 % de volume oferecido e
resolve em 1 ms, a instância 1 tem 81 % e não termina uma checagem.

O preditor é o **candidato da mochila**: quando ele é um preenchimento quase
perfeito (80–100 % da capacidade) com **muitas caixas** (9 a 20), decidir se ele
empacota custa segundos ou minutos. Instâncias cujo candidato tem 1–6 caixas, ou
que empacotam de primeira, terminam em milissegundos.

## 2. Onde o tempo vai

Instância 2 (melhor caso para medir a divisão): 408 pops, **9 533 ms em
`fits_without_overlap`** contra **541 ms nas 621 chamadas de mochila**. A
checagem é 94 % do tempo.

Instância 1 (pior caso): o primeiro candidato tem 20 caixas e volume 808/1000.
Uma única checagem, medida isoladamente:

| `node_limit` | tempo | veredito |
| --- | --- | --- |
| 20 000 | 0,32 s | abortada |
| 100 000 | 1,63 s | abortada |
| 1 000 000 | 11,66 s | **concluída: não cabe** |
| 50 000 000 (padrão) | — | ~13–55 min para chegar ao limite |

A prova existe (não cabe), mas custa ~10⁵–10⁶ nós. O `node_limit` de 50 M é
~50× maior que o necessário e, na prática, **nunca é alcançado antes do
`timeout_s`**: o `SIGKILL` do arnês chega primeiro. É por isso que o sintoma é
"timeout" e não "limite".

## 3. Causas, em ordem de impacto

### C1. A checagem não propaga: ramificação de ~1331 posições por caixa
`PackingChecker::backtrack` tenta cada caixa em **todas** as coordenadas de soma
de subconjuntos dos três eixos (`|xs|×|ys|×|zs|`, até ~11³ = 1331 com dimensões
de 1 a 10) e só rejeita por colisão. Não existe a inferência:

> depois de colocar esta caixa, alguma caixa restante ainda cabe em algum
> espaço livre?

Exemplo com os dados da instância 2: o candidato tem 6 caixas 6×3×6, 2 caixas
3×4×8 e 1 caixa 8×10×2 (volume 1000). Cada 6×3×6 ocupa um pé 6×6 no plano x–z e
3 de espessura em y; **duas não cabem na mesma faixa de 3** (6+6 > 10 nos dois
eixos do plano), então 10 de altura só acomodam 3 dessas caixas — e o candidato
tem 6. É uma contradição de contagem, imediata, mas o código só a encontra
tentando posições: 428 nós e 10 s.

### C2. Orçamento em nós, não em tempo
`set_node_limit(50 000 000)` é a única barreira interna. Além de ser grande
demais, é **imprevisível**: o custo por nó varia de ~12 µs a ~360 µs conforme a
profundidade e a instância (medido entre os candidatos de 9 a 24 caixas).
Orçamento em nós não dá prazo; só um deadline de parede dá.

### C3. O abort é engolido (bug)
`PCUCExact::solve_pcuc` nunca consulta `packingChecker.aborted()`. O `false` de
uma checagem que desistiu é tratado como "não cabe", e o laço continua com os
~12 irmãos, cada um podendo gastar outro `node_limit`. Com isso o orçamento se
multiplica em vez de parar a busca. Além disso, `solve_pcuc` só devolve vetor
vazio quando a fila esvazia — o que nunca acontece, porque a seleção vazia
sempre cabe. O status `limite` do `StressTest` é, na prática, inalcançável.

### C4. Não há limite inferior (incumbente)
O primeiro preenchimento viável só aparece quando a busca o encontra; não há
empacotamento guloso para servir de incumbente e cortar nós com
`bound ≤ incumbente`. Toda instância começa do zero, e as que não terminam não
devolvem nada aproveitável.

### C5. O arnês não tem orçamento global nem resposta parcial
- `timeout_s` é por instância: o cenário pode durar `instâncias × timeout`.
- Quando o filho é morto, perde-se até o limite superior que já estava no pipe:
  a linha do CSV fica sem volume, sem bound e sem indicação do que foi tentado.
- O build do Makefile **não usa `-O`**, então tudo acima (medido com os objetos
  sem otimização) é o que o usuário vê — e um `-O2` já reduziria boa parte.

## 4. Plano de correção

### Fase 1 — prazo, status e abort (1 dia) — mata o sintoma
1. `PackingChecker::set_deadline(steady_clock::time_point)`, verificado a cada
   1024 nós (custo desprezível) além do `node_limit`.
2. `PCUCExact::solve` devolve `{status, selection, volume, proven, nodes,
   checks, aborted_checks}`, com `status ∈ {Optimal, OutOfTime}`.
3. **Abort encerra a busca**: se a checagem desistiu, o laço termina com
   `OutOfTime`; nunca tratar "desisti" como "não cabe".
4. `StressTest`: `limite` passa a ser emitido quando o solver devolve
   `OutOfTime`, com `proven=0` e o volume do incumbente na linha.

*Aceite:* nenhuma linha com `wall_s > timeout_s + 0,05`; toda linha tem bound e
volume; `PCUCExactTest` continua 1238/1238.

### Fase 2 — checagem com espaços livres e propagação (2–3 dias) — mata a causa
5. Trocar a grade por **espaços livres maximais**: manter os espaços vazios do
   estado, colocar a caixa no canto mínimo de um espaço e dividir o espaço em
   até 6 placas. A ramificação cai de ~1331 posições por nível para o número de
   espaços (O(nº de caixas)).
6. **Propagação em todo nó**: descartar espaços que não comportam nenhuma caixa
   restante e **falhar o ramo se alguma caixa restante não couber em espaço
   algum**. É isso que resolve a instância 2 em poucos nós (contradição de
   contagem) em vez de 428.
7. Poda por volume livre × volume restante e simetria entre caixas de dimensões
   idênticas (não só do mesmo tipo).
8. Usar a mesma estrutura para um **incumbente guloso** (ordem decrescente de
   volume): limite inferior viável desde o primeiro milissegundo.

*Aceite:* instância 2 resolvida em ≤ 1 s; instância 1 decidida (ou marcada
`limite` com incumbente) em ≤ 2 s; ≥ 10× menos nós para as mesmas decisões;
1238/1238 no `PCUCExactTest`.

### Fase 3 — branch-and-bound e mochila (1–2 dias)
9. Mochila iterativa com buffers reaproveitados (sem copiar `Path`): ~1 ms por
   nó em vez de 0,5–1,5 ms; hoje são 100–600 chamadas por instância.
10. Não gerar filho com o mesmo bound do pai; cortar com
    `node.volume ≤ incumbente`.
11. Teto no `visited` (hoje até `∏(q_k+1)` vetores, ~1,7 GB no `forte`).

### Fase 4 — arnês (½ dia)
12. `--budget=S` global e por cenário; cada filho recebe
    `min(timeout_s, restante)`; instâncias não executadas saem como `orcamento`.
13. `setpgid` no filho + `kill(-pid, SIGKILL)`; `poll()` no pipe em vez de
    `read()` bloqueante; `waitpid` com `EINTR` = continuar.
14. `-O2` no build e alvo `make stress` separado de `make test`.

### Fase 5 — o que reportar no artigo
15. Para instâncias não provadas: **melhor volume viável + bound** (anytime), em
    vez de célula vazia.
16. Separar "provadas" de "não provadas" e mostrar a lacuna incumbent/bound. A
    Tabela 2 atual (`0/1 resolvidas`) esconde o resultado útil.

## 5. Ordem e custo

| Fase | Esforço | Efeito |
| --- | --- | --- |
| 1 | 1 dia | o teste para de estourar e a linha vira "não provado" com volume |
| 2 | 2–3 dias | os casos decidíveis passam de segundos para milissegundos |
| 3 | 1–2 dias | ~10–30× mais nós por segundo e árvore menor |
| 4 | ½ dia | orçamento previsível, sem processos órfãos, build otimizado |
| 5 | — | tabelas do artigo com o resultado real |

A fase 1 é a que impede o sintoma (nada é reportado) e a fase 2 é a que faz o
algoritmo realmente andar; nenhuma das duas garante decidir *todas* as
instâncias — preenchimentos quase perfeitos continuam NP-difíceis, e é por isso
que o deadline e a resposta parcial são parte da solução, não um paliativo.

## 6. O que foi feito e o que a medição mostrou

A correção executada corresponde às fases 1, 4 (parcial) e 5 do plano; na fase 2
a medição derrubou a hipótese, e o código foi revertido (detalhes em 6.2).

### 6.1 Mudanças que ficaram

| # | Mudança | Onde |
| --- | --- | --- |
| 1 | `-O2` no `CXXFLAGS` (era o único build, sem `-O`) | `Makefile` |
| 2 | `PackingChecker::set_deadline(t)`: prazo de parede, checado a cada 64 nós, além do `node_limit` | `PackingChecker` |
| 3 | `PackingChecker::greedy_packing()`: empacotamento construtivo viável em µs (incumbente) | `PackingChecker` |
| 4 | `PCUCExact::solve()` devolve `SolveReport { status, selection, volume, checks, pops, pruned, aborted_checks }`; `solve_pcuc` continua existindo e devolve `selection` | `PCUCExact` |
| 5 | **Abort deixa de ser engolido**: checagem que desiste termina a busca com `OutOfTime` e devolve o incumbente | `PCUCExact` |
| 6 | Poda por incumbente (`node.volume ≤ melhor`) e não gera filho cujo candidato é igual ao do pai | `PCUCExact` |
| 7 | Arnês: o solver recebe prazo de `timeout_s − 0,1 s`, verifica que a seleção devolvida é mesmo um empacotamento, e `limite` passa a trazer volume/lacuna | `StressTest` |
| 8 | Teste diferencial: 554 checagens novas comparando o checker com uma referência exaustiva independente e validando as posições devolvidas | `PCUCExactTest` |

Resultado: **nenhum timeout em nenhuma execução** (o filho nunca mais é morto pelo
pai), `limite` é alcançável e traz resposta, e o teste unitário passou de
1238 para **1792/1792** checagens.

### 6.2 O que a hipótese errou

A fase 2 previa que a checagem geométrica fosse o gargalo e que trocá-la por
espaços livres maximais com propagação resolveria os casos difíceis. Foi
implementada e medida, e o resultado foi outro:

| Versão da checagem | nós totais (inst. 12) | tempo (inst. 12) | nós (inst. 1, candidato-raiz) |
| --- | --- | --- | --- |
| original (somas de subconjuntos) | 624 456 | 160 ms | 0,34 s |
| bitmask + espaços livres + propagação | 624 452 | 558 ms | 0,51 s |
| bitmask, sem propagação | 624 452 | 470 ms | 0,51 s |

A árvore de busca é a mesma nos dois casos (a poda por propagação não cortou
quase nada) e a constante do bitmask é pior: o teste `overlaps` contra 2–3 caixas
já colocadas é mais barato que recompor a janela de bits de cada posição. Duas
lições ficam registradas: **o gargalo não era a ramificação, era o número de
nós** (que só cai com incumbente e poda de limite), e **o `-O2` valia 3–4×**:
com ele o caso que estourava (inst. 2) já terminava em 433 ms no código antigo.

### 6.3 Antes e depois nas 12 instâncias que estouravam

Cenário `fraco` com dimensões 1..10 e quantidades 1..6, contêiner 10×10×10,
orçamento de 10 s por instância. “Antes” = `git HEAD`; “depois” = esta versão.

| Instância | Caixas no candidato | Bound | Antes (build padrão) | Depois (build padrão) |
| --- | --- | --- | --- | --- |
| 1 | 20 | 808 | timeout aos 10 s | limite, incumbente **718** |
| 2 | 9 | 1000 | timeout aos 10 s | ótimo 806 — 0,43 s |
| 3 | 6 | 1000 | resolvida | ótimo 784 — 0,08 s |
| 4 | 1 | 1000 | resolvida | ótimo 1000 — 0,0002 s |
| 5 | 10 | 1000 | resolvida | ótimo 928 — 0,14 s |
| 6–12 | 3–9 | 995–1000 | resolvidas | ótimas — 0,001–0,16 s |

Total: **10/12 → 11/12 ótimas, 1 marcada como não provada (com incumbente), 0
timeouts**.

### 6.4 Estresse completo (3 cenários × 10 instâncias, orçamento 10 s)

```console
[fraco]  -> solved 10/10, limit 0, timeouts 0, failures 0
[medio]  -> solved  9/10, limit 1, timeouts 0, failures 0, mean 0.310 s, max 2.782 s
[forte]  -> solved  7/10, limit 3, timeouts 0, failures 0, mean 0.119 s, max 0.783 s
```

As instâncias `limite` trazem incumbente e lacuna: 20,3 % (`medio`), 8,7 %,
15,1 % e 17,0 % (`forte`). A dificuldade varia muito entre execuções (cada uma
gera instâncias novas): outra rodada de `forte` deu 3/10 ótimas e 7 `limite`.
Variância essa que é uma propriedade do problema, não do gerador: quem decide é a
distância entre o bound da mochila e um preenchimento quase perfeito.

### 6.5 Trecho pronto para o Typst

```typst
#figure(
  table(
    columns: (auto, auto, auto, auto, auto),
    align: center,
    [*Instância*], [*Caixas no candidato*], [*Bound*], [*Antes*], [*Depois*],
    [1], [20], [808], [timeout (10 s)], [limite, incumbente 718],
    [2], [9], [1000], [timeout (10 s)], [ótimo 806 (0,43 s)],
    [3], [6], [1000], [resolvida], [ótimo 784 (0,08 s)],
    [4], [1], [1000], [resolvida], [ótimo 1000 (0,00 s)],
    [5], [10], [1000], [resolvida], [ótimo 928 (0,14 s)],
  ),
  caption: [Efeito da correção nas 12 instâncias do cenário fraco que estouravam
  o orçamento de 10 s. Antes: 10/12 resolvidas e 2 timeouts. Depois: 11/12 ótimas,
  1 não provada com incumbente e nenhum timeout.],
) <tab-estresse-antes-depois>

#figure(
  table(
    columns: (auto, auto, auto, auto, auto, auto),
    align: center,
    [*Cenário*], [*Resolvidas*], [*Não provadas*], [*Timeout*], [*Mediana (s)*], [*Lacuna máx. (%)*],
    [fraco], [10/10], [0], [0], [0,000], [--],
    [medio], [9/10], [1], [0], [0,001], [20,3],
    [forte], [7/10], [3], [0], [0,009], [17,0],
  ),
  caption: [Estresse com orçamento de 10 s por instância depois da correção.
  Nenhuma instância é morta pelo arnês; as não provadas devolvem o melhor
  empacotamento encontrado e a lacuna para o limite da mochila.],
) <tab-estresse-depois>
```

## 7. Como as medições foram feitas

Instâncias geradas com `InstanceManagerExact::generateGroup` (5 tipos,
dimensões 1..10, quantidades 1..6, contêiner 10×10×10 — os parâmetros que o
cenário `fraco` usou na execução que estourou), cada uma resolvida em um
processo filho com limite de parede, contando pops, checagens e tempo por
checagem. O custo de uma checagem isolada foi medido variando `node_limit` no
candidato raiz. Os números das seções 1 e 2 são as saídas desses dois medidores.
A mochila e a checagem usadas nas medições são exatamente os objetos do build do
Makefile. As medições das seções 1–5 foram feitas com o build sem `-O` (o padrão
da época) e as da seção 6 com `-O2` (o padrão atual); onde a comparação exige o
mesmo build, o texto diz qual foi usado.
