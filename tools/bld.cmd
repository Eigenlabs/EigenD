@call "%DXSDK_DIR%Utilities\bin\dx_setenv.cmd" x86

@if "%TOOLS%" == "" set TOOLS=tools

if "%ProgramFiles(x86)%" == "" goto x86
    @"C:\Program Files (x86)\Eigenlabs\runtime-1.0.0\Python26\python" %TOOLS%\scons.py -f %TOOLS%\SConstruct %1 %2 %3 %4 %5
    goto done

:x86
    @"C:\Program Files\Eigenlabs\runtime-1.0.0\Python26\python" %TOOLS%\scons.py -f %TOOLS%\SConstruct %1 %2 %3 %4 %5
    goto done

:done
