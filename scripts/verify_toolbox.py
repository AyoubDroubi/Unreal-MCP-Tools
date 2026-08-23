#!/usr/bin/env python3
from __future__ import annotations
import json
import html
from pathlib import Path
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / 'catalog' / 'game-dev-toolbox.json'
INDEX = ROOT / 'docs' / 'html' / 'index.html'
REQUIRED = {'id','title','category','kind','priority','ue','status','summary','why','workflow','codex','caution','source','slug','html'}

def fail(msg: str) -> None:
    raise SystemExit(f'ERROR: {msg}')

data = json.loads(CATALOG.read_text(encoding='utf-8'))
tools = data.get('tools', [])
if data.get('count') != len(tools):
    fail(f'catalog count={data.get("count")} but tools={len(tools)}')
ids = [t.get('id') for t in tools]
if len(ids) != len(set(ids)):
    fail('duplicate tool ids')
if set(data.get('categories', [])) != {t.get('category') for t in tools}:
    fail('category list does not match tool categories')
index = INDEX.read_text(encoding='utf-8')
for t in tools:
    missing = REQUIRED - set(t)
    if missing: fail(f'{t.get("id")}: missing fields {sorted(missing)}')
    if not t['why'] or not t['workflow']: fail(f'{t["id"]}: why/workflow cannot be empty')
    u = urlparse(t['source'])
    if u.scheme != 'https' or not u.netloc: fail(f'{t["id"]}: source must be https URL')
    page = ROOT / t['html']
    if not page.is_file(): fail(f'{t["id"]}: missing HTML {t["html"]}')
    text = page.read_text(encoding='utf-8')
    if t['title'] not in html.unescape(text): fail(f'{t["id"]}: title not present in HTML')
    rel = f'tools/{t["id"]}.html'
    if rel not in index: fail(f'{t["id"]}: missing from HTML index')
print(f'OK: {len(tools)} toolbox guides verified across {len(data["categories"])} categories')
