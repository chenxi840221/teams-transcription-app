@echo off
echo Cleaning build artifacts...

if exist build rmdir /s /q build
if exist output rmdir /s /q output

echo Clean completed!
pause
