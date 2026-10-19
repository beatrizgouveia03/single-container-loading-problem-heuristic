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
| `demo` | Runs the built-in example (15×10×10 container with 6 boxes) |
| `ajuda` | Shows the help |
| `sair` | Exits |

> The command names and console messages are kept in Portuguese, as they are
> in the program itself (`criar` = create, `listar` = list, `rodar` = run,
> `ajuda` = help, `sair` = exit).

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
./exec_trabalho demo           # runs the built-in example and exits
./exec_trabalho rodar <id>     # runs the heuristic on instance <id> and exits
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

## Output reports

Each run writes a report to `saidas/`, numbered after the instance that produced
the solution (`demo` uses `saidas/saida_demo.txt`):

```
saidas/
├── saida_1.txt        # solution for instance 1
├── saida_2.txt        # solution for instance 2
└── saida_demo.txt     # solution for the built-in example
```

Report format — position in the `(x, y, z)` coordinate system with the origin at
the container corner and dimensions already oriented (the report text itself is
written in Portuguese by the program):

```
=== RELATÓRIO DO EXPERIMENTO DE CARREGAMENTO ===
Dimensões do Contêiner: 15 x 10 x 10
Total de itens alocados: 6
--------------------------------------------------
Caixa ID: 106 | Pos (x,y,z): (0, 0, 0) | Dimensões usadas: (5, 5, 5)
Caixa ID: 103 | Pos (x,y,z): (0, 5, 0) | Dimensões usadas: (2, 5, 4)
Caixa ID: 101 | Pos (x,y,z): (0, 0, 5) | Dimensões usadas: (3, 6, 4)
```

The `saidas/` folder is created automatically.

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
| `make clean` | Removes `build/`, `exec_trabalho`, `saidas/` and `relatorio_saida.txt` |