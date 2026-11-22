@echo off
title CPR - Crowd Pressure Radar (Exhibition Launcher)
echo =================================================================
echo   CPR (Crowd Pressure Radar) — Exhibition Demo Launcher
echo =================================================================
echo.
echo [1/3] Starting Telemetry Hub & Web Dashboard on http://localhost:5000...
start "CPR Telemetry Server" node CPR-AI/dashboard/server.js
timeout /t 2 >nul

echo [2/3] Opening Exhibition Dashboards in your default browser...
echo   - Live Crowd Monitor:        http://localhost:5000
echo   - Interactive Hardware Demo: http://localhost:5000/demo
start http://localhost:5000
start http://localhost:5000/demo

echo [3/3] Launching Master Computer Vision AI Engine on RTX 3050...
python CPR-AI/main.py
pause
