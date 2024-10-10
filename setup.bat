@echo off
REM TO BE USED FOR WINDOWS

REM Create the build directory if it doesn't exist
if not exist build mkdir build

REM Navigate into the build directory
cd build

REM Run CMake to generate Unix Makefiles
cmake .. -G "Unix Makefiles"

REM Compile the project with make
make

REM Check if make succeeded (error level 0)
if errorlevel 1 (
    echo running 'make' failed. Exiting.
    exit /b 1
) else (
    echo Build succeeded. Running raycast.exe...
    raycast.exe
)
