@echo off
REM ============================================================================
REM WinDevices Code Coverage Script
REM ============================================================================
REM This script runs code coverage analysis for WinDevices
REM
REM Usage:
REM   coverage.cmd [debug|release] [options]
REM
REM Arguments:
REM   debug|release - Build configuration (default: debug)
REM
REM Options:
REM   --unit        - Run unit tests coverage only (default)
REM   --e2e         - Run E2E tests coverage only
REM   --all         - Run combined coverage (unit + E2E)
REM   --rebuild     - Rebuild before running coverage
REM   --open        - Open coverage report in browser
REM   --help        - Show this help message
REM
REM Examples:
REM   coverage.cmd
REM   coverage.cmd debug --open
REM   coverage.cmd release --all --rebuild
REM   coverage.cmd debug --e2e --open
REM ============================================================================

setlocal enabledelayedexpansion

REM Default values
set BUILD_TYPE=debug
set COVERAGE_TYPE=unit
set REBUILD=0
set OPEN_REPORT=0

REM Parse arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="debug" (
    set BUILD_TYPE=debug
    shift
    goto :parse_args
)
if /i "%~1"=="release" (
    set BUILD_TYPE=release
    shift
    goto :parse_args
)
if /i "%~1"=="--unit" (
    set COVERAGE_TYPE=unit
    shift
    goto :parse_args
)
if /i "%~1"=="--e2e" (
    set COVERAGE_TYPE=e2e
    shift
    goto :parse_args
)
if /i "%~1"=="--all" (
    set COVERAGE_TYPE=all
    shift
    goto :parse_args
)
if /i "%~1"=="--rebuild" (
    set REBUILD=1
    shift
    goto :parse_args
)
if /i "%~1"=="--open" (
    set OPEN_REPORT=1
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    echo.
    echo WinDevices Code Coverage Script
    echo.
    echo Usage: coverage.cmd [debug^|release] [options]
    echo.
    echo Arguments:
    echo   debug^|release - Build configuration (default: debug)
    echo.
    echo Options:
    echo   --unit        - Run unit tests coverage only (default)
    echo   --e2e         - Run E2E tests coverage only
    echo   --all         - Run combined coverage (unit + E2E)
    echo   --rebuild     - Rebuild before running coverage
    echo   --open        - Open coverage report in browser
    echo   --help        - Show this help message
    echo.
    echo Examples:
    echo   coverage.cmd
    echo   coverage.cmd debug --open
    echo   coverage.cmd release --all --rebuild
    echo   coverage.cmd debug --e2e --open
    echo.
    exit /b 0
)
echo Unknown argument: %~1
exit /b 1

:args_done

REM Set preset name and config
if /i "%BUILD_TYPE%"=="debug" (
    set PRESET=windows-debug
    set CONFIG=Debug
) else (
    set PRESET=windows-release
    set CONFIG=Release
)

REM Set coverage target and report path
if /i "%COVERAGE_TYPE%"=="unit" (
    set COVERAGE_TARGET=coverage
    set REPORT_PATH=build\%PRESET%\coverage\index.html
) else if /i "%COVERAGE_TYPE%"=="e2e" (
    set COVERAGE_TARGET=coverage-e2e
    set REPORT_PATH=build\%PRESET%\coverage\e2e\index.html
) else (
    set COVERAGE_TARGET=coverage-all
    set REPORT_PATH=build\%PRESET%\coverage\all\index.html
)

echo.
echo ============================================================================
echo WinDevices Code Coverage
echo ============================================================================
echo Configuration: %BUILD_TYPE% (%CONFIG%)
echo Coverage type: %COVERAGE_TYPE%
echo Rebuild:       %REBUILD%
echo ============================================================================
echo.

REM Check if OpenCppCoverage is installed
where OpenCppCoverage.exe >nul 2>&1
if errorlevel 1 (
    echo ERROR: OpenCppCoverage not found!
    echo.
    echo Please install OpenCppCoverage:
    echo   Option 1: choco install opencppcoverage
    echo   Option 2: Download from https://github.com/OpenCppCoverage/OpenCppCoverage/releases
    echo.
    exit /b 1
)

REM Check if build exists
if not exist "build\%PRESET%" (
    echo Build directory not found. Running initial configuration...
    cmake --preset %PRESET%
    if errorlevel 1 (
        echo ERROR: CMake configuration failed!
        exit /b 1
    )
)

REM Rebuild if requested
if %REBUILD%==1 (
    echo Rebuilding...
    cmake --build build\%PRESET% --config %CONFIG% --clean-first
    if errorlevel 1 (
        echo ERROR: Build failed!
        exit /b 1
    )
    echo.
)

REM Run coverage
echo Running %COVERAGE_TYPE% coverage analysis...
echo Target: %COVERAGE_TARGET%
echo.

cmake --build build\%PRESET% --target %COVERAGE_TARGET% --config %CONFIG%
set COVERAGE_EXIT=%errorlevel%

echo.

if %COVERAGE_EXIT% NEQ 0 (
    echo WARNING: Coverage analysis completed with errors
    echo Some tests may have failed, but coverage report was still generated
    echo.
)

REM Check if report was generated
if not exist "%REPORT_PATH%" (
    echo ERROR: Coverage report not found at: %REPORT_PATH%
    exit /b 1
)

echo ============================================================================
echo Coverage Report Generated
echo ============================================================================
echo.
echo Report location: %REPORT_PATH%
echo.

REM Display summary if possible
if /i "%COVERAGE_TYPE%"=="all" (
    echo Cobertura XML: build\%PRESET%\coverage\all\coverage.xml
    echo.
)

REM Open report if requested
if %OPEN_REPORT%==1 (
    echo Opening coverage report in browser...
    start %REPORT_PATH%
) else (
    echo To view the report, run:
    echo   start %REPORT_PATH%
    echo.
    echo Or run this script with --open flag:
    echo   coverage.cmd %BUILD_TYPE% --%COVERAGE_TYPE% --open
)

echo.
exit /b 0
