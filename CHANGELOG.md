# Changelog

## 0.2.0 — 2026-08-23

Game-development productivity expansion:

- 91 separate Game Dev Toolbox guides across 9 categories
- one generated HTML page per tool, repository, Unreal subsystem or engineering method
- searchable `docs/html/index.html` catalog
- category-scoped JSON source of truth under `catalog/categories/`
- deterministic HTML/catalog generator
- catalog CLI for list/show/recommend workflows
- toolbox verifier for IDs, categories, sources, HTML output and index linkage
- GitHub Actions workflow that regenerates and commits toolbox documentation
- coverage for official UE 5.8 MCP toolsets, Blueprint productivity, testing, profiling, assets, Azure/Git source control, CI, gameplay architecture, AI, world building, animation, VFX, audio, rendering, multiplayer and crash/logging workflows
- external repositories are classified as adoption candidates or references instead of being blindly vendored into the plugin

## 0.1.0 — 2026-08-23

Initial production-oriented foundation:

- UE 5.8 official MCP / Toolset Registry extension plugin
- project and Asset Registry inspection
- Blueprint summary, graph, pin, variable, and component inspection
- Blueprint analysis for common complexity/performance smells
- Blueprint creation, function graphs, variables, and components
- common Blueprint graph node creation and wiring
- compile diagnostics, explicit save, undo
- writable-root policy, transactions, auto-compile, rollback policy
- Codex workflow, installation docs, safety model, static repository verification
