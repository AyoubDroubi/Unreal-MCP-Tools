param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath,

    [switch]$Force
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$Source = Join-Path $RepoRoot "Plugin\UnrealMCPTools"
$ProjectFull = (Resolve-Path $ProjectPath).Path

$UProject = Get-ChildItem -Path $ProjectFull -Filter *.uproject -File | Select-Object -First 1
if (-not $UProject) {
    throw "No .uproject found directly under '$ProjectFull'."
}

$PluginsDir = Join-Path $ProjectFull "Plugins"
$Destination = Join-Path $PluginsDir "UnrealMCPTools"

if (Test-Path $Destination) {
    if (-not $Force) {
        throw "'$Destination' already exists. Re-run with -Force to replace it."
    }
    Remove-Item -Recurse -Force $Destination
}

New-Item -ItemType Directory -Force -Path $PluginsDir | Out-Null
Copy-Item -Recurse -Force $Source $Destination

Write-Host "Installed UnrealMCPTools to: $Destination"
Write-Host "Next: enable ModelContextProtocol, ToolsetRegistry, and Unreal MCP Tools; then rebuild the Editor target."
