#!/usr/bin/env python3
"""Query the Unreal-MCP-Tools game-development catalog.

No network access and no automatic third-party installation are performed.
The catalog is intentionally advisory: version/license compatibility must be
verified before a project adopts an external plugin.
"""
from __future__ import annotations
import argparse, json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "catalog" / "game-dev-toolbox.json"

def load():
    return json.loads(CATALOG.read_text(encoding="utf-8"))["tools"]

def main():
    p = argparse.ArgumentParser()
    p.add_argument("command", choices=["list", "show", "recommend"])
    p.add_argument("value", nargs="?")
    args = p.parse_args()
    tools = load()

    if args.command == "list":
        for t in tools:
            print(f'{t["id"]:34} | {t["category"]:24} | {t["priority"]}')
        return

    if args.command == "show":
        if not args.value:
            p.error("show requires a tool id")
        t = next((x for x in tools if x["id"] == args.value), None)
        if not t:
            raise SystemExit(f"Unknown tool: {args.value}")
        print(json.dumps(t, indent=2, ensure_ascii=False))
        return

    q = (args.value or "").lower()
    ranked = []
    for t in tools:
        hay = " ".join([t["title"], t["category"], t["summary"], " ".join(t["why"])]).lower()
        score = sum(1 for token in q.split() if token in hay)
        if score:
            ranked.append((score, t))
    for score, t in sorted(ranked, key=lambda x: (-x[0], x[1]["title"]))[:12]:
        print(f'{t["id"]:34} | {t["priority"]:16} | {t["summary"]}')

if __name__ == "__main__":
    main()
