@echo off
setlocal enabledelayedexpansion

set "outputDir=%~dp0"
set "outputFile=%outputDir%\generated_data.sql"
set "psScript=%outputDir%\generate_data.ps1"

if not exist "%outputDir%" (
    mkdir "%outputDir%"
)

powershell -ExecutionPolicy Bypass -File "%psScript%"

echo 数据生成完成！
echo 输出文件: %outputFile%