setlocal
set PROJECT_ROOT=%1%

REM ###   Install the built DLL in the xcomp directory.  ###

xcopy /f /r /y "%PROJECT_ROOT%..\build\release\CefWebLib.dll" "%OMNIS_XCOMP_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd