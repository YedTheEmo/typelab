@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
if %errorlevel% neq 0 (
    echo [ERROR] Failed to initialize MSVC environment.
    exit /b %errorlevel%
)
cl /std:c++17 /O2 /EHsc src\main.cpp /Fe:typelab.exe
