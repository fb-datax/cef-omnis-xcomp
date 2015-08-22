setlocal
set PROJECT_ROOT=%1%

set COMMON=resources\postbuild_common.bat
echo Calling %COMMON%...
call "%PROJECT_ROOT%%COMMON%" "%PROJECT_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

REM ###   CEF dependencies. Copied only if newer.  ###

set CEFWEBLIB_ROOT=%OMNIS_XCOMP_ROOT%\CefWebLib
xcopy /f /r /y /d "%CEF_ROOT%\Release\d3dcompiler_43.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\d3dcompiler_47.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\ffmpegsumo.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\libcef.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\libEGL.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\libGLESv2.dll" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\natives_blob.bin" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\snapshot_blob.bin" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd
xcopy /f /r /y /d "%CEF_ROOT%\Release\wow_helper.exe" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

REM ###   Embed the manifest.   ###

mt.exe -nologo -manifest "%PROJECT_ROOT%\cef.exe.manifest" "%PROJECT_ROOT%\compatibility.manifest" -outputresource:"%PROJECT_ROOT%..\build\cef-release\CefWebLib.exe";#1
if %errorlevel% neq 0 goto :cmEnd

REM ###   Install the built EXE in the CefWebLib directory.   ###

xcopy /f /r /y "%PROJECT_ROOT%..\build\cef-release\CefWebLib.exe" "%CEFWEBLIB_ROOT%"
if %errorlevel% neq 0 goto :cmEnd

:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd