setlocal
set PROJECT_ROOT=%1%

set CEFWEBLIB_ROOT=%OMNIS_XCOMP_ROOT%\CefWebLib
if not exist "%CEFWEBLIB_ROOT%" echo "Creating %CEFWEBLIB_ROOT%" & mkdir "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd