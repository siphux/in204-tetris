@echo off
REM Get WSL IP
for /f "tokens=*" %%a in ('wsl ip addr show eth0 ^| findstr "inet "') do set "line=%%a"
for /f "tokens=2" %%a in ("%line%") do set "WSL_IP=%%a"

REM Remove old rule if exists
netsh interface portproxy delete v4tov4 listenport=53000

REM Add new rule with current WSL IP
netsh interface portproxy add v4tov4 listenport=53000 listenaddress=0.0.0.0 connectport=53000 connectaddress=%WSL_IP%

echo Port forwarding set up: 53000 -^> %WSL_IP%:53000
pause