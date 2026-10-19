#!/usr/bin/env python3
"""Gera a visualização 3D da solução do algoritmo.

Lê o relatório gerado pelo algoritmo (padrão: o mais recente em
saidas/saida_*.txt) e escreve visual/index.html, uma cena Three.js com o
contêiner e as caixas alocadas nas posições exatas reportadas pela heurística.

Uso:
    python3 visual/gerar_visualizacao.py [caminho/para/relatorio.txt]

Exemplos:
    make visual            # relatório do demo (saidas/saida_demo.txt)
    make visual ID=6       # relatório da instância 6 (saidas/saida_6.txt)

Saída:
    visual/index.html
"""

import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent

RE_CONTAINER = re.compile(
    r"Dimensões do Contêiner:\s*(\d+)\s*x\s*(\d+)\s*x\s*(\d+)"
)
RE_BOX = re.compile(
    r"Caixa ID:\s*(\d+)\s*\|\s*Pos \(x,y,z\):\s*\(\s*(\d+),\s*(\d+),\s*(\d+)\s*\)"
    r"\s*\|\s*Dimensões usadas:\s*\(\s*(\d+),\s*(\d+),\s*(\d+)\s*\)"
)


def parse_report(text):
    m = RE_CONTAINER.search(text)
    if not m:
        raise ValueError("não encontrei as dimensões do contêiner no relatório")
    container = {"c": int(m.group(1)), "l": int(m.group(2)), "a": int(m.group(3))}

    boxes = []
    for m in RE_BOX.finditer(text):
        boxes.append({
            "id": int(m.group(1)),
            "pos": [int(m.group(2)), int(m.group(3)), int(m.group(4))],
            "dim": [int(m.group(5)), int(m.group(6)), int(m.group(7))],
        })
    if not boxes:
        raise ValueError("nenhuma caixa alocada encontrada no relatório")
    return container, boxes


def validate(container, boxes):
    """Confere limites do contêiner e sobreposições (grade inteira)."""
    c, l, a = container["c"], container["l"], container["a"]
    grid = {}
    for b in boxes:
        x, y, z = b["pos"]
        dx, dy, dz = b["dim"]
        if x + dx > c or y + dy > l or z + dz > a:
            raise ValueError(f"caixa {b['id']} extrapola o contêiner")
        for ix in range(x, x + dx):
            for iy in range(y, y + dy):
                for iz in range(z, z + dz):
                    cell = (ix, iy, iz)
                    if cell in grid:
                        raise ValueError(
                            f"sobreposição entre as caixas {grid[cell]} e {b['id']}"
                        )
                    grid[cell] = b["id"]


HTML_TEMPLATE = r"""<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>Visualização 3D — solução do algoritmo</title>
<style>
  :root {
    --bg: #0d1117;
    --panel: rgba(22, 27, 34, 0.92);
    --border: #30363d;
    --text: #e6edf3;
    --muted: #8b949e;
    --accent: #58a6ff;
  }
  * { box-sizing: border-box; }
  html, body { height: 100%; margin: 0; }
  body {
    background: radial-gradient(1200px 800px at 70% 20%, #16202e 0%, var(--bg) 60%);
    color: var(--text);
    font: 14px/1.45 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
    overflow: hidden;
  }
  #c { position: absolute; inset: 0; width: 100%; height: 100%; display: block; }
  .panel {
    position: absolute;
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 10px;
    backdrop-filter: blur(6px);
    padding: 12px 14px;
    box-shadow: 0 8px 28px rgba(0, 0, 0, 0.45);
  }
  #hud { top: 14px; left: 14px; width: 290px; max-height: calc(100% - 28px); overflow: auto; }
  #hud h1 { font-size: 15px; margin: 0 0 8px; }
  #hud h1 span { color: var(--accent); }
  .stats { display: grid; grid-template-columns: 1fr auto; gap: 3px 10px; font-size: 13px; }
  .stats b { font-variant-numeric: tabular-nums; text-align: right; }
  .muted { color: var(--muted); }
  hr { border: 0; border-top: 1px solid var(--border); margin: 10px 0; }
  #legend { list-style: none; margin: 0; padding: 0; }
  #legend li {
    display: flex; align-items: center; gap: 8px;
    padding: 5px 6px; border-radius: 6px; cursor: pointer;
    font-size: 13px;
  }
  #legend li:hover, #legend li.active { background: rgba(88, 166, 255, 0.14); }
  .swatch { width: 12px; height: 12px; border-radius: 3px; flex: 0 0 auto; box-shadow: 0 0 0 1px rgba(0,0,0,.5); }
  #legend .dim { margin-left: auto; color: var(--muted); font-variant-numeric: tabular-nums; }
  #toolbar { bottom: 14px; left: 14px; display: flex; gap: 8px; flex-wrap: wrap; padding: 10px; }
  button {
    background: #21262d; color: var(--text); border: 1px solid var(--border);
    border-radius: 7px; padding: 6px 11px; font: inherit; font-size: 13px; cursor: pointer;
  }
  button:hover { background: #30363d; border-color: #8b949e; }
  button.on { background: #1f3a5a; border-color: var(--accent); color: #dbeafe; }
  #hint { bottom: 14px; right: 14px; font-size: 12px; color: var(--muted); padding: 8px 12px; }
  #tooltip {
    position: absolute; pointer-events: none; display: none; z-index: 10;
    background: rgba(13, 17, 23, 0.95); border: 1px solid var(--border);
    border-radius: 8px; padding: 7px 10px; font-size: 12.5px; white-space: nowrap;
    box-shadow: 0 6px 20px rgba(0,0,0,.5);
  }
  #tooltip b { color: var(--accent); }
  #status {
    position: absolute; inset: 0; display: flex; align-items: center; justify-content: center;
    font-size: 15px; color: var(--muted); text-align: center; padding: 24px; white-space: pre-line;
  }
  @media (max-width: 720px) {
    #hud { width: 230px; }
    #hint { display: none; }
  }
</style>
</head>
<body>
<canvas id="c"></canvas>
<div id="status">Carregando cena 3D…</div>

<div id="hud" class="panel" style="display:none">
  <h1>Container <span>— solução da heurística</span></h1>
  <div class="stats">
    <span class="muted">Dimensões (c × l × a)</span><b id="s-dims">—</b>
    <span class="muted">Caixas alocadas</span><b id="s-count">—</b>
    <span class="muted">Volume ocupado</span><b id="s-vol">—</b>
    <span class="muted">Taxa de ocupação</span><b id="s-fill">—</b>
  </div>
  <hr />
  <ul id="legend"></ul>
</div>

<div id="toolbar" class="panel" style="display:none">
  <button id="btn-rotate" class="on" title="Girar a câmera automaticamente">⟳ Auto-rotação</button>
  <button id="btn-labels" class="on" title="Mostrar o ID de cada caixa">IDs</button>
  <button id="btn-walls" class="on" title="Mostrar paredes do contêiner">Paredes</button>
  <button id="btn-view" title="Repor a câmera na vista inicial">Vista inicial</button>
</div>

<div id="hint" class="panel" style="display:none">arraste = orbitar · scroll = zoom · botão direito = deslocar</div>
<div id="tooltip"></div>

<script type="importmap">
{ "imports": { "three": "./vendor/three.module.js", "three/addons/": "./vendor/" } }
</script>
<script type="module">
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/OrbitControls.js';

const SOLUTION = __SOLUTION_DATA__;
const C = SOLUTION.container;          // {c, l, a}  eixo z = altura
const BOXES = SOLUTION.boxes;

const status = document.getElementById('status');
const canvas = document.getElementById('c');

function fail(err) {
  status.style.display = 'flex';
  status.textContent = 'Não foi possível iniciar a cena 3D:\n' + (err && err.message ? err.message : err);
  console.error(err);
}

try {
  init();
} catch (err) {
  fail(err);
}

function init() {
  // ---------- render / cena / câmera ----------
  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = THREE.PCFSoftShadowMap;
  renderer.outputColorSpace = THREE.SRGBColorSpace;

  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x0d1117);

  const center = new THREE.Vector3(C.c / 2, C.a / 2, C.l / 2);
  const span = Math.max(C.c, C.l, C.a, 1);

  const camera = new THREE.PerspectiveCamera(45, 1, 0.1, span * 50);
  const initialView = new THREE.Vector3(
    C.c * 0.55 + span * 0.95,
    C.a * 0.75 + span * 0.75,
    C.l * 0.55 + span * 1.15
  );
  function resetView() {
    camera.position.copy(initialView);
    controls.target.copy(center);
    controls.update();
  }

  const controls = new OrbitControls(camera, canvas);
  controls.enableDamping = true;
  controls.dampingFactor = 0.08;
  controls.autoRotate = true;
  controls.autoRotateSpeed = 0.7;
  controls.maxPolarAngle = Math.PI * 0.98;
  controls.minDistance = span * 0.4;
  controls.maxDistance = span * 8;
  resetView();

  // ---------- luzes e chão ----------
  scene.add(new THREE.HemisphereLight(0xbcd4ff, 0x1a222d, 0.65));

  const sun = new THREE.DirectionalLight(0xffffff, 1.7);
  sun.position.set(C.c * 1.1 + span, C.a * 2 + span * 1.6, C.l * 0.6 + span);
  sun.castShadow = true;
  sun.shadow.mapSize.set(2048, 2048);
  sun.shadow.camera.near = 0.1;
  sun.shadow.camera.far = span * 10;
  const s = span * 1.4;
  sun.shadow.camera.left = -s;
  sun.shadow.camera.right = s;
  sun.shadow.camera.top = s;
  sun.shadow.camera.bottom = -s;
  sun.shadow.bias = -0.0004;
  sun.target.position.copy(center);
  scene.add(sun, sun.target);

  const fill = new THREE.DirectionalLight(0x8fb6ff, 0.35);
  fill.position.set(-C.c, C.a, -C.l);
  scene.add(fill);

  const ground = new THREE.Mesh(
    new THREE.PlaneGeometry(span * 8, span * 8),
    new THREE.ShadowMaterial({ opacity: 0.35 })
  );
  ground.rotation.x = -Math.PI / 2;
  ground.position.y = -0.02;
  ground.receiveShadow = true;
  scene.add(ground);

  const grid = new THREE.GridHelper(Math.max(C.c, C.l) * 1.6, Math.max(C.c, C.l) * 1.6, 0x30363d, 0x21262d);
  grid.position.y = -0.015;
  if (Array.isArray(grid.material)) grid.material.forEach((m) => (m.transparent = true, m.opacity = 0.5));
  else { grid.material.transparent = true; grid.material.opacity = 0.5; }
  scene.add(grid);

  // ---------- contêiner (paredes translúcidas + arestas) ----------
  const containerGroup = new THREE.Group();
  const shellGeo = new THREE.BoxGeometry(C.c, C.a, C.l);
  const walls = new THREE.Mesh(
    shellGeo,
    new THREE.MeshBasicMaterial({
      color: 0x79c0ff, transparent: true, opacity: 0.07,
      side: THREE.BackSide, depthWrite: false
    })
  );
  walls.position.copy(center);
  const shellEdges = new THREE.LineSegments(
    new THREE.EdgesGeometry(shellGeo),
    new THREE.LineBasicMaterial({ color: 0x79c0ff, transparent: true, opacity: 0.85 })
  );
  shellEdges.position.copy(center);
  containerGroup.add(walls, shellEdges);
  scene.add(containerGroup);

  // ---------- caixas ----------
  const labelGroup = new THREE.Group();
  scene.add(labelGroup);

  const raycastMeshes = [];
  const byId = new Map();
  const n = Math.max(BOXES.length, 1);

  BOXES.forEach((b, i) => {
    const [x, y, z] = b.pos;
    const [dx, dy, dz] = b.dim;
    const color = new THREE.Color().setHSL((i / n + 0.07) % 1, 0.62, 0.56);

    const mesh = new THREE.Mesh(
      new THREE.BoxGeometry(dx, dz, dy),
      new THREE.MeshStandardMaterial({ color, roughness: 0.5, metalness: 0.04 })
    );
    mesh.position.set(x + dx / 2, z + dz / 2, y + dy / 2);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    mesh.userData.info = b;
    mesh.userData.baseEmissive = 0x000000;

    const outline = new THREE.LineSegments(
      new THREE.EdgesGeometry(mesh.geometry),
      new THREE.LineBasicMaterial({ color: 0x0b0f14, transparent: true, opacity: 0.6 })
    );
    mesh.add(outline);

    const label = makeLabel('#' + b.id);
    label.position.set(x + dx / 2, z + dz + Math.max(0.45, span * 0.045), y + dy / 2);
    labelGroup.add(label);

    scene.add(mesh);
    raycastMeshes.push(mesh);
    byId.set(b.id, { mesh, label, color, info: b });
  });

  // ---------- HUD ----------
  const volTotal = C.c * C.l * C.a;
  const volUsed = BOXES.reduce((acc, b) => acc + b.dim[0] * b.dim[1] * b.dim[2], 0);
  document.getElementById('s-dims').textContent = `${C.c} × ${C.l} × ${C.a}`;
  document.getElementById('s-count').textContent = String(BOXES.length);
  document.getElementById('s-vol').textContent = `${volUsed} / ${volTotal} u³`;
  document.getElementById('s-fill').textContent = ((100 * volUsed) / volTotal).toFixed(1) + ' %';

  const legend = document.getElementById('legend');
  byId.forEach(({ color, info, mesh }, id) => {
    const li = document.createElement('li');
    li.dataset.id = id;
    li.innerHTML =
      `<span class="swatch" style="background:${color.getStyle()}"></span>` +
      `<span>Caixa ${id}</span>` +
      `<span class="dim">${info.dim.join('×')}</span>`;
    li.addEventListener('mouseenter', () => setHighlight(id));
    li.addEventListener('mouseleave', () => setHighlight(null));
    legend.appendChild(li);
  });
  if (BOXES.length > 120) toggleLabels(false);

  // ---------- interação ----------
  const tooltip = document.getElementById('tooltip');
  const raycaster = new THREE.Raycaster();
  const ndc = new THREE.Vector2();
  let hovered = null;

  function setHighlight(id) {
    byId.forEach((entry, key) => {
      const active = key === id;
      entry.mesh.material.emissive.setHex(active ? 0x444444 : 0x000000);
      entry.label.material.opacity = active || id === null ? 1 : 0.45;
    });
    legend.querySelectorAll('li').forEach((li) => {
      li.classList.toggle('active', Number(li.dataset.id) === id);
    });
  }

  canvas.addEventListener('pointermove', (ev) => {
    const r = canvas.getBoundingClientRect();
    ndc.x = ((ev.clientX - r.left) / r.width) * 2 - 1;
    ndc.y = -((ev.clientY - r.top) / r.height) * 2 + 1;
    raycaster.setFromCamera(ndc, camera);
    const hit = raycaster.intersectObjects(raycastMeshes, false)[0];
    const id = hit ? hit.object.userData.info.id : null;
    if (id !== hovered) {
      hovered = id;
      setHighlight(id);
      if (id !== null) {
        const b = byId.get(id);
        tooltip.innerHTML =
          `<b>Caixa ${id}</b><br>` +
          `pos: (${b.info.pos.join(', ')})<br>` +
          `dim: (${b.info.dim.join(', ')})`;
        tooltip.style.display = 'block';
        canvas.style.cursor = 'pointer';
      } else {
        tooltip.style.display = 'none';
        canvas.style.cursor = 'grab';
      }
    }
    if (id !== null) {
      tooltip.style.left = Math.min(ev.clientX + 14, window.innerWidth - 160) + 'px';
      tooltip.style.top = Math.min(ev.clientY + 14, window.innerHeight - 80) + 'px';
    }
  });
  canvas.addEventListener('pointerleave', () => {
    hovered = null;
    setHighlight(null);
    tooltip.style.display = 'none';
  });

  // ---------- botões ----------
  const btnRotate = document.getElementById('btn-rotate');
  const btnLabels = document.getElementById('btn-labels');
  const btnWalls = document.getElementById('btn-walls');

  function toggleLabels(force) {
    labelGroup.visible = force === undefined ? !labelGroup.visible : force;
    btnLabels.classList.toggle('on', labelGroup.visible);
  }
  btnRotate.addEventListener('click', () => {
    controls.autoRotate = !controls.autoRotate;
    btnRotate.classList.toggle('on', controls.autoRotate);
  });
  btnLabels.addEventListener('click', () => toggleLabels());
  btnWalls.addEventListener('click', () => {
    containerGroup.visible = !containerGroup.visible;
    btnWalls.classList.toggle('on', containerGroup.visible);
  });
  document.getElementById('btn-view').addEventListener('click', resetView);

  // ---------- redimensionamento / loop ----------
  function resize() {
    const w = canvas.clientWidth || window.innerWidth;
    const h = canvas.clientHeight || window.innerHeight;
    renderer.setSize(w, h, false);
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
  }
  window.addEventListener('resize', resize);
  resize();

  document.getElementById('hud').style.display = 'block';
  document.getElementById('toolbar').style.display = 'flex';
  document.getElementById('hint').style.display = 'block';
  status.style.display = 'none';

  renderer.setAnimationLoop(() => {
    controls.update();
    renderer.render(scene, camera);
  });
}

function makeLabel(text) {
  const font = 'bold 44px system-ui, -apple-system, "Segoe UI", Roboto, sans-serif';
  const measure = document.createElement('canvas').getContext('2d');
  measure.font = font;
  const w = Math.ceil(measure.measureText(text).width) + 28;
  const h = 56;

  const cv = document.createElement('canvas');
  cv.width = w;
  cv.height = h;
  const ctx = cv.getContext('2d');
  ctx.font = font;
  ctx.fillStyle = 'rgba(13, 17, 23, 0.88)';
  ctx.strokeStyle = 'rgba(88, 166, 255, 0.9)';
  ctx.lineWidth = 2;
  const r = 10;
  ctx.beginPath();
  ctx.moveTo(r, 1);
  ctx.lineTo(w - r, 1);
  ctx.quadraticCurveTo(w - 1, 1, w - 1, r);
  ctx.lineTo(w - 1, h - r);
  ctx.quadraticCurveTo(w - 1, h - 1, w - r, h - 1);
  ctx.lineTo(r, h - 1);
  ctx.quadraticCurveTo(1, h - 1, 1, h - r);
  ctx.lineTo(1, r);
  ctx.quadraticCurveTo(1, 1, r, 1);
  ctx.closePath();
  ctx.fill();
  ctx.stroke();
  ctx.fillStyle = '#e6edf3';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText(text, w / 2, h / 2 + 1);

  const tex = new THREE.CanvasTexture(cv);
  tex.colorSpace = THREE.SRGBColorSpace;
  const sprite = new THREE.Sprite(
    new THREE.SpriteMaterial({ map: tex, transparent: true, depthTest: false })
  );
  sprite.scale.set((w / h) * 0.75, 0.75, 1);
  sprite.renderOrder = 999;
  return sprite;
}
</script>
</body>
</html>
"""


def latest_report():
    """Relatório mais recente em saidas/saida_*.txt (ou None)."""
    candidates = [
        p for p in (ROOT / "saidas").glob("saida_*.txt") if p.is_file()
    ]
    if not candidates:
        return None
    return max(candidates, key=lambda p: p.stat().st_mtime)


def main():
    if len(sys.argv) > 1:
        report = Path(sys.argv[1])
    else:
        report = latest_report()
        if report is None:
            sys.exit(
                "erro: nenhum relatório em saidas/\n"
                "Gere um com `make visual`, `make visual ID=<id>` ou "
                "`./exec_trabalho rodar <id>`."
            )
    if not report.is_file():
        sys.exit(
            f"erro: relatório não encontrado: {report}\n"
            "Gere um com `make visual`, `make visual ID=<id>` ou "
            "`./exec_trabalho rodar <id>`."
        )

    try:
        container, boxes = parse_report(report.read_text(encoding="utf-8"))
        validate(container, boxes)
    except ValueError as exc:
        sys.exit(f"erro: {exc}\nrelatório: {report}")

    data = json.dumps(
        {"container": container, "boxes": boxes}, ensure_ascii=False, indent=2
    )
    html = HTML_TEMPLATE.replace("__SOLUTION_DATA__", data)

    out = HERE / "index.html"
    out.write_text(html, encoding="utf-8")

    vol_total = container["c"] * container["l"] * container["a"]
    vol_used = sum(b["dim"][0] * b["dim"][1] * b["dim"][2] for b in boxes)
    print(f"Relatório: {report}")
    print(f"Contêiner: {container['c']} x {container['l']} x {container['a']}")
    print(f"Caixas alocadas: {len(boxes)}")
    print(f"Ocupação: {vol_used}/{vol_total} u³ ({100 * vol_used / vol_total:.1f} %)")
    print(f"Visualização gerada em: {out}")


if __name__ == "__main__":
    main()
