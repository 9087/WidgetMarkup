@echo off
setlocal
cd /d "%~dp0..\..\..\..\.."
set "APP=Game\Binaries\Win64\WidgetMarkupApp.exe"
set "EXTRA_ARGS=--extra-arguments "test -nullrhi -WidgetMarkupTestTimeout=120""
set "UPROJECT=%~1"
if "%UPROJECT%"=="" set "UPROJECT=%CD%\Game\Game.uproject"

echo ========================================
echo WidgetMarkup Test Suite
echo ========================================

:: Count the tests automatically from the "call :run_test" lines below, so the
:: [n/total] numbering never needs manual updating when tests are added/removed.
:: /b restricts the match to lines that START with "call :run_test" so comments
:: and this very findstr command (which also contain the text) are not counted.
set /a TOTAL=0
for /f "delims=" %%A in ('findstr /b /c:"call :run_test" /c:"call :run_negative_test" "%~f0"') do set /a TOTAL+=1
set /a COUNT=0

:: Every test call is followed by "|| exit /b 1" so a failing subroutine
:: aborts the whole suite instead of continuing to "All tests passed."
call :run_test "TestEmpty" "Empty widgetmarkup" || exit /b 1
call :run_test "TestReactive" "Reactive properties" || exit /b 1
call :run_test "TestComputed" "Computed properties" || exit /b 1
call :run_test "TestTextBlock" "TextBlock widget" || exit /b 1
call :run_test "TestButton" "Button widget" || exit /b 1
call :run_test "TestImage" "Image widget" || exit /b 1
call :run_test "TestBorder" "Border widget (single-child container)" || exit /b 1
call :run_test "TestCanvasPanel" "CanvasPanel widget" || exit /b 1
call :run_test "TestHorizontalBox" "HorizontalBox widget" || exit /b 1
call :run_test "TestVerticalBox" "VerticalBox widget" || exit /b 1
call :run_test "TestGridPanel" "GridPanel widget (ColumnFill/RowFill containers)" || exit /b 1
call :run_test "TestOverlay" "Overlay widget" || exit /b 1
call :run_test "TestStyleSheetInline" "StyleSheet (inline)" || exit /b 1
call :run_test "TestStyleSheetOverride" "StyleSheet (inherit standalone file + override)" || exit /b 1
call :run_test "TestListView" "ListView + ObservableCollection" || exit /b 1
call :run_test "TestDynamicChild" "Dynamic add_child / remove_child / get_child" || exit /b 1
call :run_test "TestStaticChild" "Static child widget blueprint + get_child / remove_child" || exit /b 1
call :run_test "TestVariable" "Variable element (defaults, types, brace literals)" || exit /b 1
call :run_negative_test "TestConflict" "Property value+children conflict (expected compile failure)" || exit /b 1

echo.
echo ========================================
echo All tests passed.
echo ========================================
exit /b 0

:run_test
set /a COUNT+=1
echo [%COUNT%/%TOTAL%] %~2...
"%APP%" /WidgetMarkup/Tests/%~1 --project "%UPROJECT%" %EXTRA_ARGS%
if errorlevel 1 (
    echo.
    echo ========================================
    echo TESTS FAILED
    echo ========================================
    exit /b 1
)
exit /b 0

:run_negative_test
set /a COUNT+=1
echo [%COUNT%/%TOTAL%] %~2...
:: A negative test must fail compilation. In test mode the commandlet exits
:: with code 7 when the initial compile fails, so assert the exact exit code
:: instead of scraping logs or killing the process after a timeout.
"%APP%" /WidgetMarkup/Tests/%~1 --project "%UPROJECT%" %EXTRA_ARGS%
if errorlevel 8 goto :negative_fail
if not errorlevel 7 goto :negative_fail
echo [PASS] %~2 (expected compile failure with exit code 7)
exit /b 0

:negative_fail
echo.
echo ========================================
echo TESTS FAILED (negative test %~1 did not fail compilation as expected)
echo ========================================
exit /b 1
