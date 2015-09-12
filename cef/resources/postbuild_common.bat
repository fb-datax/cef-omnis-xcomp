setlocal
set PROJECT_ROOT=%1%

REM ###   Create the CefWebLib directory in the xcomp directory if necessary.   ###

set CEFWEBLIB_ROOT=%OMNIS_XCOMP_ROOT%\CefWebLib
if not exist "%CEFWEBLIB_ROOT%" echo "Creating %CEFWEBLIB_ROOT%" & mkdir "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

REM ###   CEF dependencies. Copied only if newer.  ###

xcopy /f /r /y /d "%CEF_ROOT%\Resources\cef.pak" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Resources\cef_100_percent.pak" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Resources\cef_200_percent.pak" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Resources\cef_extensions.pak" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Resources\devtools_resources.pak" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Resources\icudtl.dat" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d /i /s "%CEF_ROOT%\Resources\locales" "%CEFWEBLIB_ROOT%\locales\"
if %errorlevel% neq 0 goto :cmEnd

:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd