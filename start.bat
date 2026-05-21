@echo off
chcp 65001 >nul

echo =========================================
echo Starting Game Account Exchange Services
echo =========================================
echo.

set BACKEND_PORT=8080
set API_PORT=8081
set FRONTEND_PORT=8082
set HOST=localhost

echo Configuration:
echo   LangGraph Backend:  http://%HOST%:%BACKEND_PORT%
echo   FastAPI API:        http://%HOST%:%API_PORT%
echo   Frontend:           http://%HOST%:%FRONTEND_PORT%
echo.
echo Hot reload is enabled for all services!
echo.

echo Killing existing processes on ports...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%BACKEND_PORT%.*LISTENING"') do taskkill /F /PID %%a >nul 2>&1
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%API_PORT%.*LISTENING"') do taskkill /F /PID %%a >nul 2>&1
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%FRONTEND_PORT%.*LISTENING"') do taskkill /F /PID %%a >nul 2>&1

echo Waiting for ports to be released...
set /a RETRY_COUNT=0
:CHECK_PORTS
set /a RETRY_COUNT+=1
if %RETRY_COUNT% GTR 10 goto PORTS_DONE
netstat -ano | findstr ":%BACKEND_PORT%.*LISTENING" >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    timeout /t 1 >nul
    goto CHECK_PORTS
)
netstat -ano | findstr ":%API_PORT%.*LISTENING" >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    timeout /t 1 >nul
    goto CHECK_PORTS
)
netstat -ano | findstr ":%FRONTEND_PORT%.*LISTENING" >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    timeout /t 1 >nul
    goto CHECK_PORTS
)
:PORTS_DONE
echo Checking port status...
netstat -ano | findstr ":%BACKEND_PORT%.*LISTENING" >nul 2>&1
if %ERRORLEVEL% EQU 0 echo WARNING: Port %BACKEND_PORT% is still in use!
netstat -ano | findstr ":%API_PORT%.*LISTENING" >nul 2>&1
if %ERRORLEVEL% EQU 0 echo WARNING: Port %API_PORT% is still in use!
netstat -ano | findstr ":%FRONTEND_PORT%.*LISTENING" >nul 2>&1
if %ERRORLEVEL% EQU 0 echo WARNING: Port %FRONTEND_PORT% is still in use!
echo.

echo Starting LangGraph backend server...
if not exist "langgraph.json" (
    echo Error: langgraph.json not found, cannot start LangGraph backend
    pause
    exit /b 1
)
start "LangGraph Backend" cmd /k "call .\venv\Scripts\activate.bat && langgraph dev --port %BACKEND_PORT% --no-browser"

timeout /t 3 >nul

echo Starting FastAPI server with hot reload...
if not exist "backend" (
    echo Error: backend directory not found
    pause
    exit /b 1
)
start "FastAPI Server" cmd /k "call .\venv\Scripts\activate.bat && cd backend && uvicorn api.app:app --host 0.0.0.0 --port %API_PORT% --reload"

timeout /t 3 >nul

echo Starting frontend server...
if not exist "frontend" (
    echo Error: frontend directory not found
    pause
    exit /b 1
)
start "Frontend" cmd /k "cd frontend && npm run dev -- --host 0.0.0.0 --port %FRONTEND_PORT%"

echo All services started with hot reload!
echo.
echo Press any key to stop all services...
pause >nul

echo Stopping services...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%BACKEND_PORT%.*LISTENING"') do taskkill /F /PID %%a >nul 2>&1
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%API_PORT%.*LISTENING"') do taskkill /F /PID %%a >nul 2>&1
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":%FRONTEND_PORT%.*LISTENING"') do taskkill /F /PID %%a >nul 2>&1
echo All services stopped.