setlocal
set PROJECT_ROOT=%1%

REM ###   Install the built DLL in the xcomp directory.  ###

xcopy /y "%PROJECT_ROOT%\..\build\debug\CefWebLib.dll" "%OMNIS_XCOMP_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /y "%PROJECT_ROOT%\..\build\debug\CefWebLib.pdb" "%OMNIS_XCOMP_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd