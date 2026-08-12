@echo off
setlocal
cd /d "%~dp0..\..\..\..\.."
set "APP=Game\Binaries\Win64\WidgetMarkupApp.exe"
set "EXTRA_ARGS=--extra-arguments "test -nullrhi""
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

call :run_test "TestEmpty" "Empty widgetmarkup"
call :run_test "TestReactive" "Reactive properties"
call :run_test "TestComputed" "Computed properties"
call :run_test "TestTextBlock" "TextBlock widget"
call :run_test "TestButton" "Button widget"
call :run_test "TestImage" "Image widget"
call :run_test "TestBorder" "Border widget (single-child container)"
call :run_test "TestCanvasPanel" "CanvasPanel widget"
call :run_test "TestHorizontalBox" "HorizontalBox widget"
call :run_test "TestVerticalBox" "VerticalBox widget"
call :run_test "TestGridPanel" "GridPanel widget (ColumnFill/RowFill containers)"
call :run_test "TestOverlay" "Overlay widget"
call :run_test "TestStyleSheetInline" "StyleSheet (inline)"
call :run_test "TestStyleSheetOverride" "StyleSheet (inherit standalone file + override)"
call :run_test "TestListView" "ListView + ObservableCollection"
call :run_test "TestDynamicChild" "Dynamic add_child / remove_child / get_child"
call :run_test "TestStaticChild" "Static child widget blueprint + get_child / remove_child"
call :run_test "TestVariable" "Variable element (defaults, types, brace literals)"
call :run_negative_test "TestConflict" "Property value+children conflict (expected compile failure)"

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
:: A negative test is EXPECTED to fail compilation. A compile failure means the
:: app never shuts down on its own, so run it in the background, wait, kill it,
:: then assert the log shows a compile failure and no passing Python checks.
set "TESTLOG=%CD%\Game\Saved\Logs\WidgetMarkupApp.log"
if exist "%TESTLOG%" del /f /q "%TESTLOG%"
start "" /b "%APP%" /WidgetMarkup/Tests/%~1 --project "%UPROJECT%" --extra-arguments "test -nullrhi -log" >nul 2>&1
timeout /t 30 /nobreak >nul
taskkill /F /IM WidgetMarkupApp.exe >nul 2>&1

findstr /c:"CompileFromSourceCode failed" "%TESTLOG%" >nul 2>&1
if errorlevel 1 goto :negative_fail
findstr /c:"ALL CHECKS PASSED" "%TESTLOG%" >nul 2>&1
if not errorlevel 1 goto :negative_fail
echo [PASS] %~2 (expected compile failure)
exit /b 0

:negative_fail
echo.
echo ========================================
echo TESTS FAILED (negative test %~1 did not fail as expected)
echo ========================================
exit /b 1
