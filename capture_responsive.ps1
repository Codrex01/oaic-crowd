$edge = 'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
$sizes = @(
    @{ w = 360;  h = 800;  file = 'C:\Users\subha\Downloads\crowd-pressure-radar\screenshot_360.png' },
    @{ w = 768;  h = 1024; file = 'C:\Users\subha\Downloads\crowd-pressure-radar\screenshot_768.png' },
    @{ w = 1280; h = 800;  file = 'C:\Users\subha\Downloads\crowd-pressure-radar\screenshot_1280.png' },
    @{ w = 2560; h = 1440; file = 'C:\Users\subha\Downloads\crowd-pressure-radar\screenshot_2560.png' }
)

foreach ($s in $sizes) {
    Write-Host "Capturing $($s.w)x$($s.h)..."
    Start-Process -FilePath $edge -ArgumentList '--headless', '--disable-gpu', "--window-size=$($s.w),$($s.h)", '--virtual-time-budget=2000', "--screenshot=$($s.file)", 'http://localhost:5173' -Wait
}
Write-Host 'All screenshots captured.'

