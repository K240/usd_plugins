@echo off
REM ============================================================
REM Environment setup for the AssetResolver plugin.
REM
REM Usage:
REM   1. Edit HFS below for your Houdini version.
REM   2. call setup_env.bat
REM   3. "%HFS%\bin\hython.exe" tests\create_test_assets.py
REM   4. "%HFS%\bin\hython.exe" tests\test_resolve.py
REM ============================================================

REM --- Houdini install path ---
if not defined HFS (
    set "HFS=C:\Program Files\Side Effects Software\Houdini 21.0.596"
)

REM --- Houdini USD plugins (required for file formats etc.) ---
set "PXR_PLUGINPATH_NAME=%HFS%\bin\usd_plugins;%~dp0dist\resources;%PXR_PLUGINPATH_NAME%"

REM --- Plugin library path ---
set "PATH=%~dp0dist\lib;%HFS%\bin;%PATH%"

REM --- Asset root (where versioned assets live) ---
set "USD_ASSET_ROOT=%~dp0tests\test_assets"

echo [setup_env] HFS=%HFS%
echo [setup_env] USD_ASSET_ROOT=%USD_ASSET_ROOT%
echo [setup_env] PXR_PLUGINPATH_NAME=%PXR_PLUGINPATH_NAME%
