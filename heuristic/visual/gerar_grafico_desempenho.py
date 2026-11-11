#!/usr/bin/env python3
"""Gera a imagem do desempenho de um grupo de instâncias.

Lê o arquivo ``desempenho.csv`` escrito pelo comando ``rodar-grupo`` (padrão:
``instances/<grupo>/desempenho.csv``) e escreve uma imagem SVG autônoma — sem
HTML e sem dependências externas — com uma única série: a ocupação (%) por id
de instância.

Uso:
    python3 visual/gerar_grafico_desempenho.py instances/escala
    python3 visual/gerar_grafico_desempenho.py instances/escala/desempenho.csv
    python3 visual/gerar_grafico_desempenho.py     # grupo mais recente

Saída:
    visual/desempenho_<grupo>.svg
"""

import csv
import html
import math
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent

COLUNAS = ["id", "ocupacao_pct"]

W, H = 960, 520
ML, MR, MT, MB = 80, 40, 70, 70
PLOT_W = W - ML - MR
PLOT_H = H - MT - MB
TICKS = 3


def format_tick(value):
    if abs(value - round(value)) < 1e-9:
        return f"{value:.0f}%"
    return f"{value:.1f}%"


def nice_max(value, ticks=TICKS):
    """Arredonda o topo do eixo para 1, 2, 2.5 ou 5 vezes uma potência de 10."""
    if value <= 0:
        return float(ticks)
    bruto = value / ticks
    base = 10 ** math.floor(math.log10(bruto))
    for multiplo in (1, 2, 2.5, 5, 10):
        if bruto <= multiplo * base:
            return multiplo * base * ticks
    return value


def resolver_csv(argumento):
    """Aceita uma pasta de grupo, um CSV ou nada (grupo mais recente)."""
    if argumento is None:
        candidatos = sorted(
            ROOT.glob("instances/*/desempenho.csv"),
            key=lambda p: p.stat().st_mtime,
        )
        if not candidatos:
            raise SystemExit(
                "nenhum desempenho.csv encontrado em instances/*/. "
                "Rode antes: ./exec_trabalho rodar-grupo <nome>"
            )
        return candidatos[-1]

    caminho = Path(argumento)
    if not caminho.is_absolute():
        caminho = (Path.cwd() / caminho).resolve()
    if caminho.is_dir():
        caminho = caminho / "desempenho.csv"
    if not caminho.is_file():
        raise SystemExit(f"arquivo de desempenho não encontrado: {caminho}")
    return caminho


def ler_desempenho(caminho):
    with caminho.open(newline="", encoding="utf-8") as arq:
        leitor = csv.DictReader(arq)
        faltando = [c for c in COLUNAS if c not in (leitor.fieldnames or [])]
        if faltando:
            raise SystemExit(
                f"{caminho} não tem as colunas esperadas: {', '.join(faltando)}"
            )
        linhas = [
            {
                "id": int(linha["id"]),
                "ocupacao_pct": float(linha["ocupacao_pct"]),
            }
            for linha in leitor
        ]
    if not linhas:
        raise SystemExit(f"{caminho} está vazio")
    linhas.sort(key=lambda item: item["id"])
    return linhas


def build_svg(linhas, grupo):
    n = len(linhas)
    ocupacoes = [item["ocupacao_pct"] for item in linhas]
    ymax = nice_max(max(ocupacoes) * 1.02, TICKS)
    slot = PLOT_W / n if n > 1 else PLOT_W
    base_y = MT + PLOT_H

    def px(i):
        # Um único ponto fica centralizado.
        return ML + slot * (i + 0.5) if n > 1 else ML + PLOT_W / 2

    def py(valor):
        return MT + PLOT_H * (1.0 - min(valor, ymax) / ymax)

    partes = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" '
        f'width="{W}" height="{H}" role="img" '
        f'aria-label="Ocupação por instância do grupo {html.escape(grupo)}">',
        f'<rect width="{W}" height="{H}" fill="#ffffff"/>',
        f'<text x="{ML}" y="40" font-family="system-ui, sans-serif" '
        f'font-size="20" font-weight="600" fill="#1f2430">'
        f"Ocupação por instância — grupo “{html.escape(grupo)}”</text>",
    ]

    # Grade e eixo y (ocupação em %).
    for i in range(TICKS + 1):
        fracao = i / TICKS
        y = MT + PLOT_H * fracao
        valor = ymax * (1.0 - fracao)
        partes.append(
            f'<line x1="{ML}" y1="{y:.1f}" x2="{ML + PLOT_W}" y2="{y:.1f}" '
            f'stroke="#e6e9ef" stroke-width="1"/>'
        )
        partes.append(
            f'<text x="{ML - 12}" y="{y + 4:.1f}" text-anchor="end" '
            f'font-family="system-ui, sans-serif" font-size="12" '
            f'fill="#6b7484">{format_tick(valor)}</text>'
        )

    # Eixos.
    partes.append(
        f'<line x1="{ML}" y1="{base_y}" x2="{ML + PLOT_W}" y2="{base_y}" '
        f'stroke="#b9c0cc" stroke-width="1.2"/>'
    )

    # Série única: ocupação (%) por id.
    pontos = " ".join(f"{px(i):.1f},{py(item['ocupacao_pct']):.1f}"
                      for i, item in enumerate(linhas))
    partes.append(
        f'<polyline points="{pontos}" fill="none" stroke="#2f66b8" '
        f'stroke-width="2.2" stroke-linejoin="round" stroke-linecap="round"/>'
    )
    for i, item in enumerate(linhas):
        partes.append(
            f'<circle cx="{px(i):.1f}" cy="{py(item["ocupacao_pct"]):.1f}" '
            f'r="3.2" fill="#2f66b8">'
            f"<title>Instância {item['id']}: {item['ocupacao_pct']:.2f}%"
            f"</title></circle>"
        )

    # Rótulos do eixo x (id da instância), amostrados para não sobrepor.
    passo = max(1, n // 14)
    ultimo = -1
    for i in range(0, n, passo):
        if n - 1 - i < passo:
            continue
        partes.append(
            f'<text x="{px(i):.1f}" y="{base_y + 22}" text-anchor="middle" '
            f'font-family="system-ui, sans-serif" font-size="12" '
            f'fill="#6b7484">{linhas[i]["id"]}</text>'
        )
        ultimo = i
    if ultimo != n - 1:
        partes.append(
            f'<text x="{px(n - 1):.1f}" y="{base_y + 22}" '
            f'text-anchor="middle" font-family="system-ui, sans-serif" '
            f'font-size="12" fill="#6b7484">{linhas[-1]["id"]}</text>'
        )

    # Títulos dos eixos.
    meio_y = MT + PLOT_H / 2
    partes.append(
        f'<text x="18" y="{meio_y:.0f}" text-anchor="middle" '
        f'font-family="system-ui, sans-serif" font-size="12" fill="#4d5666" '
        f'font-weight="600" transform="rotate(-90 18 {meio_y:.0f})">'
        f"ocupação (%)</text>"
    )
    partes.append(
        f'<text x="{ML + PLOT_W / 2:.0f}" y="{H - 16}" text-anchor="middle" '
        f'font-family="system-ui, sans-serif" font-size="12" fill="#4d5666" '
        f'font-weight="600">id da instância</text>'
    )

    partes.append("</svg>")
    return "".join(partes)


def main():
    argumento = sys.argv[1] if len(sys.argv) > 1 else None
    caminho = resolver_csv(argumento)
    linhas = ler_desempenho(caminho)
    grupo = caminho.parent.name if caminho.name == "desempenho.csv" else caminho.stem

    saida = HERE / f"desempenho_{re.sub(r'[^0-9A-Za-z_-]', '_', grupo)}.svg"
    saida.write_text(build_svg(linhas, grupo), encoding="utf-8")

    media = sum(item["ocupacao_pct"] for item in linhas) / len(linhas)
    print(f"Gráfico do grupo '{grupo}' gerado em {saida.relative_to(ROOT)}")
    print(f"Instâncias: {len(linhas)} | ocupação média: {media:.2f}%")


if __name__ == "__main__":
    main()
