@echo off
setlocal

REM Директория сборки
set BUILD_DIR=build
set BUILD_TYPE=Debug

REM Создаём директорию
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Генератор: Ninja или Visual Studio
where ninja >nul 2>&1
if %errorlevel%==0 (
    REM Используем Ninja
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
    cmake --build . --config %BUILD_TYPE%
) else (
    REM Используем Visual Studio (пример: 2022, x64)
    cmake -G "Visual Studio 17 2022" -A x64 ..
    cmake --build . --config %BUILD_TYPE%
)

endlocal

pause
