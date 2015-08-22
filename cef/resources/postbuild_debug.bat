setlocal
set PROJECT_ROOT=%1%

set COMMON=resources\postbuild_common.bat
echo Calling %COMMON%...
call "%PROJECT_ROOT%%COMMON%" "%PROJECT_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

REM ###   CEF dependencies. Copied only if newer.  ###

set CEFWEBLIB_ROOT=%OMNIS_XCOMP_ROOT%\CefWebLib
xcopy /f /r /y /d "%CEF_ROOT%\Debug\d3dcompiler_43.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\d3dcompiler_47.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\ffmpegsumo.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\libcef.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\libEGL.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\libGLESv2.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\natives_blob.bin" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\snapshot_blob.bin" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Debug\wow_helper.exe" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

REM ###   Embed the manifest.   ###

mt.exe -nologo -manifest "%PROJECT_ROOT%\cef.exe.manifest" "%PROJECT_ROOT%\compatibility.manifest" -outputresource:"%PROJECT_ROOT%..\build\cef-debug\CefWebLib.exe";#1
if %errorlevel% neq 0 goto :cmEnd

REM ###   Install the built EXE in the CefWebLib directory.   ###

xcopy /f /r /y "%PROJECT_ROOT%..\build\cef-debug\CefWebLib.exe" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y "%PROJECT_ROOT%..\build\cef-debug\CefWebLib.pdb" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd