#!/usr/bin/env python3
"""Generate the static Game Dev Toolbox HTML site from the JSON catalog."""
from __future__ import annotations
import collections
import html
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / 'catalog' / 'game-dev-toolbox.json'
CATEGORY_ROOT = ROOT / 'catalog' / 'categories'
HTML_ROOT = ROOT / 'docs' / 'html'
TOOLS_ROOT = HTML_ROOT / 'tools'
ASSETS_ROOT = HTML_ROOT / 'assets'
SUMMARY = ROOT / 'docs' / 'GAME_DEV_TOOLBOX.md'

CSS = r''':root{--bg:#0b1020;--panel:#121a2b;--panel2:#182238;--text:#e9eef8;--muted:#9aa8bd;--accent:#6ee7ff;--good:#7ee787;--warn:#ffd166;--border:#2b3954}
*{box-sizing:border-box}body{margin:0;background:linear-gradient(180deg,#09101d,#0f1728);color:var(--text);font:16px/1.6 Inter,Segoe UI,Arial,sans-serif}
a{color:var(--accent)}.wrap{max-width:1050px;margin:auto;padding:36px 24px 70px}.top{display:flex;gap:16px;align-items:center;justify-content:space-between;flex-wrap:wrap}
.badge{display:inline-block;padding:5px 9px;border:1px solid var(--border);border-radius:999px;background:#10192a;color:var(--muted);font-size:13px}.priority{color:var(--good)}
h1{font-size:42px;line-height:1.1;margin:18px 0 10px}h2{margin-top:36px;font-size:24px}p.lead{font-size:19px;color:#c9d4e5;max-width:850px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:14px}.card{background:rgba(18,26,43,.92);border:1px solid var(--border);border-radius:16px;padding:18px}
ol,ul{padding-left:24px}code,pre{font-family:Consolas,monospace}.note{border-left:4px solid var(--warn);padding:12px 16px;background:#1b2030;border-radius:8px}
.nav{font-size:14px;color:var(--muted)}footer{margin-top:50px;border-top:1px solid var(--border);padding-top:20px;color:var(--muted);font-size:14px}
table{width:100%;border-collapse:collapse;background:var(--panel);border-radius:12px;overflow:hidden}th,td{padding:11px 12px;border-bottom:1px solid var(--border);text-align:left}th{background:var(--panel2)}
.filters{display:flex;gap:8px;flex-wrap:wrap;margin:20px 0}.filters button{background:#121a2b;color:var(--text);border:1px solid var(--border);padding:8px 12px;border-radius:999px;cursor:pointer}
.tool{margin:12px 0}.tool small{color:var(--muted)}.searchbox{width:100%;padding:12px 14px;border:1px solid var(--border);border-radius:12px;background:#0e1626;color:var(--text);font-size:16px;margin:18px 0}.meta{display:flex;gap:8px;flex-wrap:wrap}.category-count{color:var(--muted);font-size:14px}.tool[hidden]{display:none}.section-head{display:flex;justify-content:space-between;align-items:end;gap:10px}.source a{word-break:break-all}
'''

def esc(value: object) -> str:
    return html.escape(str(value), quote=True)

def render_page(t: dict) -> str:
    why = ''.join(f'<li>{esc(x)}</li>' for x in t['why'])
    flow = ''.join(f'<li>{esc(x)}</li>' for x in t['workflow'])
    return f'''<!doctype html>\n<html lang="en">\n<head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">\n<title>{esc(t['title'])} — Unreal Game Dev Toolbox</title>\n<link rel="stylesheet" href="../assets/style.css"></head>\n<body><main class="wrap">\n<div class="nav"><a href="../index.html">← Game Dev Toolbox</a> · {esc(t['category'])}</div>\n<div class="top"><div class="meta">\n<span class="badge">{esc(t['kind'])}</span>\n<span class="badge">{esc(t['ue'])}</span>\n<span class="badge">{esc(t['status'])}</span>\n</div><span class="badge priority">{esc(t['priority'])}</span></div>\n<h1>{esc(t['title'])}</h1>\n<p class="lead">{esc(t['summary'])}</p>\n<div class="grid">\n<section class="card"><h2>Why it helps</h2><ul>{why}</ul></section>\n<section class="card"><h2>Codex / MCP role</h2><p>{esc(t['codex'])}</p></section>\n</div>\n<section class="card"><h2>Recommended workflow</h2><ol>{flow}</ol></section>\n<section class="note"><strong>Watch out:</strong> {esc(t['caution'])}</section>\n<h2>Adoption decision</h2>\n<p><strong>{esc(t['priority'])}</strong>. Treat this page as a project decision guide; confirm engine version, license, team scale and project constraints before adoption.</p>\n<h2>Source</h2>\n<p class="source"><a href="{esc(t['source'])}" target="_blank" rel="noreferrer">{esc(t['source'])}</a></p>\n<footer>Unreal-MCP-Tools · Game Dev Toolbox · generated {esc(DATA['generated'])} · verify version-specific APIs before production adoption.</footer>\n</main></body></html>'''

def main() -> None:
    global DATA
    category_docs = [json.loads(p.read_text(encoding='utf-8')) for p in sorted(CATEGORY_ROOT.glob('*.json'))]
    categories = [d['category'] for d in category_docs]
    tools = [t for d in category_docs for t in d['tools']]
    DATA = {'generated': '2026-08-23', 'baseline': 'Unreal Engine 5.8+ for MCP-specific features', 'count': len(tools), 'categories': categories, 'tools': tools}
    CATALOG.parent.mkdir(parents=True, exist_ok=True)
    CATALOG.write_text(json.dumps(DATA, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    TOOLS_ROOT.mkdir(parents=True, exist_ok=True)
    ASSETS_ROOT.mkdir(parents=True, exist_ok=True)
    for stale in TOOLS_ROOT.glob('*.html'):
        stale.unlink()
    (ASSETS_ROOT / 'style.css').write_text(CSS, encoding='utf-8')
    for t in tools:
        (TOOLS_ROOT / f"{t['id']}.html").write_text(render_page(t), encoding='utf-8')

    counts = collections.Counter(t['category'] for t in tools)
    parts = [
        '<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">',
        '<title>Unreal Game Dev Toolbox</title><link rel="stylesheet" href="assets/style.css"></head><body><main class="wrap">',
        f'<span class="badge">{len(tools)} focused guides</span><h1>Unreal Game Dev Toolbox</h1>',
        '<p class="lead">Curated tools, repositories, engine systems and engineering methods that make a Blueprint-heavy Unreal Engine project easier to build, inspect, automate, test, profile, collaborate on and maintain with Codex.</p>',
        '<div class="card"><strong>Rule:</strong> prefer Unreal-native deterministic APIs first, official MCP toolsets second, our custom MCP layer only for missing capabilities, UI automation after native tools, and direct binary asset mutation only for specialized offline tooling.</div>',
        '<input id="q" class="searchbox" type="search" placeholder="Filter guides: blueprint, multiplayer, CI, profiling, assets…" aria-label="Filter guides">',
    ]
    for category in categories:
        parts.append(f'<div class="section-head"><h2>{esc(category)}</h2><span class="category-count">{counts[category]} guides</span></div>')
        for t in (x for x in tools if x['category'] == category):
            searchable = ' '.join([t['title'], t['category'], t['kind'], t['priority'], t['summary'], *t['why'], *t['workflow'], t['codex']]).lower()
            parts.append(f'<div class="card tool" data-search="{esc(searchable)}"><strong><a href="tools/{esc(t["id"])}.html">{esc(t["title"])}</a></strong><br><small>{esc(t["priority"])} · {esc(t["kind"])} · {esc(t["ue"])}</small><p>{esc(t["summary"])}</p></div>')
    parts.append("<script>const q=document.getElementById('q');q.addEventListener('input',()=>{const s=q.value.trim().toLowerCase();document.querySelectorAll('.tool').forEach(x=>x.hidden=s&&!x.dataset.search.includes(s));});</script>")
    parts.append('<footer>Unreal-MCP-Tools · use the individual HTML guide for setup, workflow, Codex role and adoption cautions.</footer></main></body></html>')
    (HTML_ROOT / 'index.html').write_text(''.join(parts), encoding='utf-8')

    lines = ['# Game Dev Toolbox', '', f'This catalog contains **{len(tools)} separate guides** covering tools, repositories, Unreal systems and development methods that materially improve a Blueprint-heavy UE workflow.', '', 'Open `docs/html/index.html` in a browser for the visual catalog.', '', '## Query from terminal', '', '```bash', 'python scripts/game_dev_toolbox.py list', 'python scripts/game_dev_toolbox.py show official-unreal-mcp', 'python scripts/game_dev_toolbox.py recommend "multiplayer blueprint debugging"', '```', '', '## Adoption hierarchy', '', '1. Unreal-native deterministic API/tool.', '2. Official UE 5.8 MCP toolset.', '3. Our `UnrealMCPTools` extension when Epic has no equivalent.', '4. Mature external plugin/repository after version/license verification.', '5. Slate UI automation for workflows with no native API.', '6. Direct `.uasset` mutation only for specialized offline inspection/tooling.', '', '## Categories', '']
    for category in categories:
        lines.append(f'- **{category}** — {counts[category]} guides')
    SUMMARY.parent.mkdir(parents=True, exist_ok=True)
    SUMMARY.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(f'Generated {len(tools)} guides across {len(categories)} categories.')

if __name__ == '__main__':
    DATA = {}
    main()
