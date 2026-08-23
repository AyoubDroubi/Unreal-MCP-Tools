#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 /path/to/UnrealProject [--force]" >&2
  exit 2
fi

PROJECT_PATH="$(cd "$1" && pwd)"
FORCE="${2:-}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SOURCE="$REPO_ROOT/Plugin/UnrealMCPTools"
DEST="$PROJECT_PATH/Plugins/UnrealMCPTools"

shopt -s nullglob
UPROJECTS=("$PROJECT_PATH"/*.uproject)
if [[ ${#UPROJECTS[@]} -eq 0 ]]; then
  echo "No .uproject found directly under '$PROJECT_PATH'." >&2
  exit 1
fi

if [[ -e "$DEST" ]]; then
  if [[ "$FORCE" != "--force" ]]; then
    echo "'$DEST' already exists. Re-run with --force to replace it." >&2
    exit 1
  fi
  rm -rf "$DEST"
fi

mkdir -p "$PROJECT_PATH/Plugins"
cp -R "$SOURCE" "$DEST"

echo "Installed UnrealMCPTools to: $DEST"
echo "Next: enable ModelContextProtocol, ToolsetRegistry, and Unreal MCP Tools; then rebuild the Editor target."
