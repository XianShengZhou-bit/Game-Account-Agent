@echo off
cls

:: Switch to project root directory
cd /d "%~dp0.."

:: Run the Python script directly
python tmp/clear_database.py

pause