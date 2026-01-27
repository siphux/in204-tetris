@echo off
REM Remove port forwarding rule
netsh interface portproxy delete v4tov4 listenport=53000

echo Port forwarding rule removed
pause