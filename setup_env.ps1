# ============================================================
# Environment setup for the AssetResolver plugin (PowerShell)
#
# Usage:
#   1. Edit $HFS below for your Houdini version.
#   2. . .\setup_env.ps1
#   3. & "$env:HFS\bin\hython.exe" tests\create_test_assets.py
#   4. & "$env:HFS\bin\hython.exe" tests\test_resolve.py
# ============================================================

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

# --- Houdini install path ---
if (-not $env:HFS) {
    $env:HFS = "C:\Program Files\Side Effects Software\Houdini 21.0.596"
}

# --- Houdini USD plugins (required for file formats etc.) ---
$env:PXR_PLUGINPATH_NAME = "$env:HFS\bin\usd_plugins;$ScriptDir\dist\resources;$env:PXR_PLUGINPATH_NAME"

# --- Plugin library path ---
$env:PATH = "$ScriptDir\dist\lib;$env:HFS\bin;$env:PATH"

# --- Asset root (where versioned assets live) ---
$env:USD_ASSET_ROOT = "$ScriptDir\tests\test_assets"

Write-Host "[setup_env] HFS=$env:HFS"
Write-Host "[setup_env] USD_ASSET_ROOT=$env:USD_ASSET_ROOT"
Write-Host "[setup_env] PXR_PLUGINPATH_NAME=$env:PXR_PLUGINPATH_NAME"
