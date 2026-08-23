#!/usr/bin/env python3
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PLUGIN = ROOT / "Plugin" / "UnrealMCPTools"

REQUIRED = [
    ROOT / "README.md",
    ROOT / "AGENTS.md",
    PLUGIN / "UnrealMCPTools.uplugin",
    PLUGIN / "Source" / "UnrealMCPTools" / "UnrealMCPTools.Build.cs",
    PLUGIN / "Source" / "UnrealMCPTools" / "Public" / "UnrealMCPBlueprintToolset.h",
    PLUGIN / "Source" / "UnrealMCPTools" / "Public" / "UnrealMCPProjectToolset.h",
    PLUGIN / "Source" / "UnrealMCPTools" / "Public" / "UnrealMCPWorkflowToolset.h",
    ROOT / "docs" / "CODEX_WORKFLOW.md",
    ROOT / "docs" / "SAFETY.md",
]

FORBIDDEN_DIRS = {"Binaries", "Intermediate", "Saved", "DerivedDataCache", ".vs"}


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


for path in REQUIRED:
    if not path.exists():
        fail(f"missing required file: {path.relative_to(ROOT)}")

descriptor = json.loads((PLUGIN / "UnrealMCPTools.uplugin").read_text(encoding="utf-8"))
modules = descriptor.get("Modules", [])
if not any(m.get("Name") == "UnrealMCPTools" and m.get("Type") == "Editor" for m in modules):
    fail("UnrealMCPTools must remain an Editor module")

plugin_deps = {p.get("Name") for p in descriptor.get("Plugins", [])}
for dependency in {"ModelContextProtocol", "ToolsetRegistry"}:
    if dependency not in plugin_deps:
        fail(f"uplugin is missing dependency: {dependency}")

for path in ROOT.rglob("*"):
    if path.is_dir() and path.name in FORBIDDEN_DIRS:
        fail(f"generated Unreal directory is committed: {path.relative_to(ROOT)}")

headers = list((PLUGIN / "Source").rglob("*.h"))
aicallable_count = 0
for header in headers:
    text = header.read_text(encoding="utf-8")
    aicallable_count += text.count("AICallable")

    generated = [line for line in text.splitlines() if ".generated.h\"" in line]
    if generated:
        includes = [line for line in text.splitlines() if line.startswith("#include")]
        if includes and includes[-1] != generated[-1]:
            fail(f"generated header must be the final include: {header.relative_to(ROOT)}")

if aicallable_count < 20:
    fail(f"expected a substantial MCP tool surface; only found {aicallable_count} AICallable functions")

cpp_text = "\n".join(
    path.read_text(encoding="utf-8")
    for path in (PLUGIN / "Source").rglob("*.cpp")
)
required_safety_markers = [
    "FScopedTransaction",
    "AllowedAssetRoots",
    "CompileBlueprint",
    "UndoTransaction",
    "TryCreateConnection",
]
for marker in required_safety_markers:
    if marker not in cpp_text:
        fail(f"missing expected safety/graph marker: {marker}")

placeholder = re.compile(r"\b(TODO|FIXME|NOT_IMPLEMENTED)\b")
for path in list((PLUGIN / "Source").rglob("*.h")) + list((PLUGIN / "Source").rglob("*.cpp")):
    if placeholder.search(path.read_text(encoding="utf-8")):
        fail(f"placeholder marker found in implementation: {path.relative_to(ROOT)}")

print(f"OK: repository layout verified; {aicallable_count} AICallable tool functions discovered.")
print("NOTE: this is a static check. Build with UnrealBuildTool/UHT in the target UE 5.8+ installation.")
