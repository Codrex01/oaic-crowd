# CPR - Crowd Pressure Radar (PowerShell Launcher)
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host "  CPR (Crowd Pressure Radar) — Exhibition Demo Launcher" -ForegroundColor Yellow
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "[1/3] Starting Telemetry Hub & Dashboard on http://localhost:5000..." -ForegroundColor Green
Start-Process powershell -ArgumentList "-NoExit", "-Command", "node CPR-AI/dashboard/server.js"

Start-Sleep -Seconds 2

Write-Host "[2/3] Opening Exhibition Dashboards in browser..." -ForegroundColor Green
Write-Host "  - Live Operational Monitor:  http://localhost:5000" -ForegroundColor Cyan
Write-Host "  - Interactive Hardware Demo: http://localhost:5000/demo" -ForegroundColor Cyan
Start-Process "http://localhost:5000"
Start-Process "http://localhost:5000/demo"

Write-Host "[3/3] Launching Master Computer Vision Engine on NVIDIA RTX 3050..." -ForegroundColor Green
python CPR-AI/main.py
