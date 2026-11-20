# Algoritmo exato para o PCumC (exact2)

Implementação do branch-and-bound para o Problema de Carregamento em um Único Contêiner. Este guia explica como compilar o projeto e reproduzir os resultados do relatório.

## Requisitos

- `g++` com suporte a C++17
- `make`

## Compilando

Dentro de `exact/`:

```bash
make
```

Os executáveis ficam em `build/`. Para apagar tudo e recompilar do zero:

```bash
make clean
make
```
## Reproduzindo os resultados do algoritmo exato

Os três experimentos do relatório são executados com:

```bash
./build/test_branch_and_bound f36_32_6  32 1 3 6 10 8 8 14
./build/test_branch_and_bound f36_28_11 28 1 3 6 10 8 8 14
./build/test_branch_and_bound f36_28_10 28 1 3 6 10 8 8 14
```

### Argumentos

Ordem dos argumentos, deduzida da saída do programa. **Confirme contra o `main` de `test/test_branch_and_bound.cpp`** antes de publicar este guia.

| Posição | Exemplo | Significado |
|---|---|---|
| 1 | `f36_32_6` | Nome do grupo de instâncias |
| 2 | `32` | Número de pacotes |
| 3 | `1` | Modo de execução (a saída mostra `solveSorted`) |
| 4 e 5 | `3 6` | Dimensão mínima e máxima dos pacotes |
| 6, 7 e 8 | `10 8 8` | Contêiner: largura, altura e profundidade |
| 9 | `14` | Limite de tempo, em minutos |

### Resultados esperados

| Instância | Pacotes | Volume total | Alocados | F | Ocupação | Tempo |
|---|---|---|---|---|---|---|
| `f36_32_6`  | 32 | 2677 | 9  | 613 | 95,78% | ~96 s |
| `f36_28_11` | 28 | 2351 | 10 | 605 | 94,53% | ~37 s |
| `f36_28_10` | 28 | 2245 | 11 | 629 | 98,28% | ~30 s |

Contêiner de volume 640 em todas. Nas três execuções o programa imprime `Otimo provado`.

Os valores de `F`, o número de pacotes alocados e a ocupação devem ser **exatamente** os da tabela. O tempo varia de uma execução para outra e entre máquinas.

### Salvando a saída

```bash
mkdir -p resultados
./build/test_branch_and_bound f36_32_6  32 1 3 6 10 8 8 14 | tee resultados/f36_32_6.txt
./build/test_branch_and_bound f36_28_11 28 1 3 6 10 8 8 14 | tee resultados/f36_28_11.txt
./build/test_branch_and_bound f36_28_10 28 1 3 6 10 8 8 14 | tee resultados/f36_28_10.txt
```

## Ambiente dos experimentos do relatório

| Item | Valor |
|---|---|
| Processador | Intel Core i5-10400F @ 2,90 GHz (6 núcleos) |
| Memória | 8 GB |
| Sistema | Linux Mint 22.3 |
| Compilador | `g++`, C++17 |

Para uma medição mais estável, feche outros programas pesados e repita cada execução algumas vezes, registrando o tempo médio.
