# Single Container Loading Problem — Heuristic

A C++ implementation of a **greedy heuristic** for the Single Container Loading
Problem: packing as much volume as possible of rectangular boxes into a
container, respecting the container limits, without overlaps, and considering
the 6 orientations of each box.

The project comes with a **command console** to create instances and run the
heuristic, per-instance **solution reports**, and an **interactive 3D
visualization** of the solution in the browser.

---

## Table of Contents

- [Requirements](#requirements)
- [Building](#building)
- [Usage — command console](#usage--command-console)
- [Instance format](#instance-format)
- [Instance groups](#instance-groups)
- [Output reports](#output-reports)
- [3D visualization](#3d-visualization)
- [Makefile targets](#makefile-targets)

---

## Requirements

| Tool | Tested version | Needed for |
| --- | --- | --- |
| `g++` (any C++17 compiler) | 11.4.0 | building the project |
| `make` | 4.3 | build and helper targets |
| `python3` | 3.10.12 | generating the 3D visualization |
| Modern browser | any | opening the visualization |

There are no external dependencies to build: the JSON parser
([nlohmann/json](https://github.com/nlohmann/json)) is already in `lib/json.hpp`
and the Three.js library used in the visualization is in `visual/vendor/`, so
the scene works **offline**.

## Building

```bash
make        # produces the ./exec_trabalho executable
```

The build uses `-std=c++17 -Wall -Wextra` and places object files in `build/`.
To remove everything (objects, executable and reports):

```bash
make clean
```

## Usage — command console

```bash
make run            # or: ./exec_trabalho
```

Available commands (type `ajuda` in the console):

| Command | Description |
| --- | --- |
| `criar <packages> <min> <max> <C> <L> <A>` | Creates an instance with `<packages>` packages of random dimensions between `<min>` and `<max>`, for a `C x L x A` container. E.g.: `criar 8 2 5 15 10 10` |
| `listar` | Lists the instances saved in `instances/` with their container and number of packages |
| `rodar <id>` | Loads instance `<id>`, runs the heuristic and writes the report to `saidas/saida_<id>.txt` |
| `criar-grupo <name> <n> <packages> <min> <max> <C> <L> <A>` | Creates `<n>` instances in `instances/<name>/`. Instance `i` uses packages between `<min>*i` and `<max>*i` and a container `C*i x L*i x A*i`. E.g.: `criar-grupo escala 50 20 1 5 15 10 10` |
| `listar-grupos` | Lists the groups under `instances/` with their instance count |
| `listar-grupo <name>` | Lists the instances of a group |
| `rodar-grupo <name>` | Runs every instance of the group and writes its performance CSV |
| `demo` | Runs the built-in example (15×10×10 container with 6 boxes) |
| `ajuda` | Shows the help |
| `sair` | Exits |


Example session (the heuristic's debug output has been omitted):

```console
$ ./exec_trabalho
=== Single Container Loading — console de testes ===
Instâncias em: instances/
Digite 'ajuda' para ver os comandos.

> criar 8 2 5 15 10 10
Instância 1 salva em instances/instance_1.json
  contêiner: 15 x 10 x 10 (volume 1500)
  pacotes:   8 com dimensões entre 2 e 5 (volume total 454)
Para testar:  rodar 1
> listar
Instâncias em instances:
  [1] contêiner 15x10x10 | 8 pacotes
> rodar 1
=== Instância 1 ===
Contêiner: 15 x 10 x 10
Pacotes:   8
...
=== Resultado ===
Alocados: 8 de 8 pacotes
Ocupação: 454/1500 u³ (30.3 %)
Relatório: saidas/saida_1.txt (visualize com `make visual ID=1`)
> sair
```

Commands can also be run outside the console (useful for scripts):

```bash
./exec_trabalho demo               # runs the built-in example and exits
./exec_trabalho rodar <id>         # runs the heuristic on instance <id> and exits
./exec_trabalho criar-grupo escala 50 20 1 5 15 10 10
./exec_trabalho rodar-grupo escala # runs all 50 instances at once
```

## Instance format

Each instance is stored in `instances/instance_<id>.json` and holds **the
container dimensions and the dimensions of each package**:

```json
{
    "container": [15, 10, 10],
    "packages": [
        [4, 4, 3],
        [5, 2, 5],
        [2, 2, 2]
    ]
}
```

- `container`: `[length, width, height]` of the container (integer units).
- `packages`: list of `[l, w, h]` for each package. Ids are assigned by list
  order (1, 2, 3, ...).

Instances in the old format (only the list of packages, with no `container`) are
still recognized by the reader, but **cannot be run** — `rodar` warns about it
and asks you to recreate the instance with the `criar` command.

## Instance groups

`criar-grupo` builds a whole family of instances at once. Instance `i`
(1..`n`) scales every dimension by `i`, which increases the absolute
heterogeneity of the packages while preserving all proportions:

| iteration | packages (min..max) | container |
| --- | --- | --- |
| 1 | 1..5 | 15 × 10 × 10 |
| 2 | 2..10 | 30 × 20 × 20 |
| … | … | … |
| 50 | 50..250 | 750 × 500 × 500 |

```bash
./exec_trabalho criar-grupo escala 50 20 1 5 15 10 10
```

Each group lives in its own folder inside `instances/`:

```
instances/escala/
├── grupo.json          # parameters used to generate the group
├── instance_1.json     # iteration 1
├── ...
├── instance_50.json    # iteration 50
└── desempenho.csv      # written by rodar-grupo
saidas/escala/
├── saida_1.txt
├── ...
└── saida_50.txt
```

An existing group is never overwritten: pick another name or remove the folder
to generate it again.

`rodar-grupo <name>` runs the heuristic on every instance in one go (the
detailed per-package log is silenced):

```console
$ ./exec_trabalho rodar-grupo escala
[  1/50] instância   1: 20/20 pacotes, 29.53 % (443/1500 u³)
...
=== Desempenho do grupo 'escala' ===
Instâncias executadas: 50
Ocupação média: 35.58 %
Desempenho por id: instances/escala/desempenho.csv
```

### Performance file

`instances/<group>/desempenho.csv` has one row per instance id, with the filled
volume, total volume and occupancy:

```csv
id,pacotes,alocados,volume_preenchido,volume_total,ocupacao_pct
1,20,20,443,1500,29.53
2,20,20,4358,12000,36.32
```

### Performance chart

The script `visual/gerar_grafico_desempenho.py` reads that CSV and writes a
standalone **SVG image** (no HTML page and no external dependencies) showing
the relationship between occupancy (%) and instance id. Only that series is
plotted — the volume columns of the CSV are not used in the chart.

```bash
make grafico GRUPO=escala
# or, for an explicit file
python3 visual/gerar_grafico_desempenho.py instances/escala/desempenho.csv
```

Output: `visual/desempenho_escala.svg`. Without arguments, the script uses the
most recent `instances/*/desempenho.csv`.

## 3D visualization

The 3D scene (`visual/index.html`) shows the container filled with the solution
from the report: colored boxes with their ID, edges, shadows, an interactive
legend, a tooltip with position/dimensions, occupancy statistics, and controls
for auto-rotation, IDs, walls and the initial view.

```bash
make visual            # runs the built-in example and generates the scene from saidas/saida_demo.txt
make visual ID=1       # runs the heuristic on instance 1 and generates the scene from saidas/saida_1.txt
```

You can also generate the scene from any existing report, without running the
heuristic:

```bash
python3 visual/gerar_visualizacao.py saidas/saida_1.txt   # explicit path
python3 visual/gerar_visualizacao.py                      # uses the most recent report in saidas/
```

The generator validates the solution (container bounds and overlaps) before
writing the HTML. To open the scene, serve the project root with any static
server and point the browser to `visual/index.html`:

```bash
python3 -m http.server 8000
# http://127.0.0.1:8000/visual/index.html
```

> The scene uses ES modules, so a static server (any one) is required instead of
> opening the file with `file://`. The Three.js files are in `visual/vendor/`,
> so no internet access is needed.

## Makefile targets

| Target | What it does |
| --- | --- |
| `make` / `make all` | Builds `./exec_trabalho` |
| `make run` | Builds and opens the command console |
| `make test` | Builds and runs the tests |
| `make visual` | Runs `demo` and generates `visual/index.html` from its report |
| `make visual ID=<n>` | Runs the heuristic on instance `<n>` and generates `visual/index.html` |
| `make grupo GRUPO=<name>` | Creates and runs a 50-instance growing group (20 packages, 1..5 dimensions, 15×10×10 base container) |
| `make grafico GRUPO=<name>` | Generates the `visual/desempenho_<name>.svg` image (occupancy per instance id) from the group's performance CSV |
| `make clean` | Removes `build/`, `exec_trabalho`, `saidas/` and `relatorio_saida.txt` |