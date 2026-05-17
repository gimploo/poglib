@echo off
SETLOCAL

REM ===========================================================================
REM                 -- WINDOWS BUILD SCRIPT FOR C PROJECTS --
REM ===========================================================================


REM Include compiler of choice (here its msvc)
set CC=cl
set CC_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set CC_DEFAULT_FLAGS=/std:c11 /W4 /wd4244 /wd4996 /wd4477 /wd4267 /w14996 /FC /TC /Zi /experimental:c11atomics
set CC_DEFAULT_LIBS=User32.lib Gdi32.lib Shell32.lib winmm.lib dbghelp.lib shlwapi.lib

REM Source and executalble path (default)
set EXE_FOLDER_DEFAULT_PATH=.\
set SRC_FOLDER_DEFAULT_PATH=.\
set LIBRARY_DEFAULT_PATH="C:\Users\Gokul\projects\GetBack\lib"

REM Source files and exe name
set SRC_FILE_NAME=main.c
set EXE_FILE_NAME=test.exe


:main

    if "%1" == "clean" (
        call :cleanup
        goto :end
    )

    if "%1" == "deepclean" (
        call :deepcleanup
        goto :end
    )

    cls
    echo [*] Running build script for windows...
    
    echo [*] Checking `%CC%` compiler is installed ...
    call :check_compiler_is_installed || goto :end


    REM EITHER A RELEASE BUILD OR A DEBUG BUILD
    if "%1" == "release" (
        echo [*] Building project [RELEASE BUILD]...
        call :build_project_with_msvc "release" || goto :end
        echo [*] Running executable ...
        call :run_executable
        echo [!] Exited! 
    ) else (
        echo [*] Building project [DEBUG BUILD]...
        call :build_project_with_msvc "debug" || goto :end
    )


    REM RUNS THE EXECUTABLE THROUGH A DEBUGGER (ONLY DEBUG BUILD)
    if "%1" == "debug" (
        if "%2" == "--vs" (
            call :run_executable_with_vsdebugger
            goto :end
        ) else if "%2" == "--rad" (
            call :run_executable_with_raddebug
            goto :end
        )
    )

    REM RUNS THE EXECUTABLE NORMALLY (ONLY DEBUG BUILD)
    if "%1" == "run" (
        echo [*] Running executable ...
        call :run_executable
        echo [!] Exited! 
    )

    goto :end


REM ===========================================================================
REM                         -- BUILD RECIPE --
REM ===========================================================================
REM (change whats in here to ) -
REM                            |
REM                            v
:build_project_with_msvc

    set INCLUDES=/I %LIBRARY_DEFAULT_PATH%

    if "%~1" == "debug" (
        set FLAGS=/DGLEW_STATIC /DDEBUG /DJPH_PROFILE_ENABLED /DJPH_USE_SSE4_2 /MTd
        echo [!] PREPROCESSOR FILE CREATED!!
        %CC% %CC_DEFAULT_FLAGS% %FLAGS%^
        /P %INCLUDES% %SRC_FOLDER_DEFAULT_PATH%\%SRC_FILE_NAME% || echo [!] Failed to preprocess! && exit /b 1
    ) else (
        set FLAGS=/DGLEW_STATIC /DJPH_USE_SSE4_2 /MT
    )

    set LIBS=%LIBRARY_DEFAULT_PATH%\GLEW\lib\Release\x64\glew32s.lib ^
                %LIBRARY_DEFAULT_PATH%\FREETYPE\win64\freetype.lib ^
                %LIBRARY_DEFAULT_PATH%\poglib\external\assimp\lib\Debug\assimp-vc143-mtd.lib ^
                %LIBRARY_DEFAULT_PATH%\GLFW\lib\glfw3dll.lib ^
                %LIBRARY_DEFAULT_PATH%\SDL2\lib\x64\SDL2.lib ^
                %LIBRARY_DEFAULT_PATH%\SDL2\lib\x64\SDL2main.lib ^
                %LIBRARY_DEFAULT_PATH%\poglib\external\joltc\lib\windows\debug\Joltd.lib ^
                %LIBRARY_DEFAULT_PATH%\poglib\external\joltc\lib\windows\debug\Joltcd.lib ^
                Opengl32.lib glu32.lib

    %CC% %CC_DEFAULT_FLAGS% %FLAGS%^
        %INCLUDES% ^
        /Fe%EXE_FOLDER_DEFAULT_PATH%\%EXE_FILE_NAME% ^
        %SRC_FOLDER_DEFAULT_PATH%\%SRC_FILE_NAME% ^
        /link %CC_DEFAULT_LIBS% -SUBSYSTEM:console || echo [!] Failed to compile! && exit /b 1

    move *.pdb %EXE_FOLDER_DEFAULT_PATH% >nul
    move *.obj %EXE_FOLDER_DEFAULT_PATH% >nul


    exit /b %errorlevel%




REM =======================================================================================
REM                             -- HELPER FUNCTIONS --
REM =======================================================================================
    
:run_executable
    echo.
    %EXE_FOLDER_DEFAULT_PATH%\%EXE_FILE_NAME%
    echo.
    exit /b %errorlevel% 

:run_executable_with_vsdebugger
    echo [!] Running executable through the VS debugger!
    devenv /DebugExe %EXE_FOLDER_DEFAULT_PATH%\%EXE_FILE_NAME%
    exit /b 0

:run_executable_with_raddebug
    echo [!] Running executable through the raddebugger!
    raddbg %EXE_FOLDER_DEFAULT_PATH%\%EXE_FILE_NAME% --project .\
    exit /b 0

:check_compiler_is_installed 
    %CC_PATH% || echo [!] Compiler %CC% not found! && goto :end
    echo [!] Compiler %CC% found!
    exit /b 0


:cleanup
    if exist bin (
        rd /s /q bin || echo [!] `bin` folder not found!
        echo [!] `bin` directory deleted!
    )
    exit /b 0

:deepcleanup
    echo [*] Cleanup in progress ...
    if exist "%LIBRARY_DEFAULT_PATH%" (
        rd /s /q "%LIBRARY_DEFAULT_PATH%"
        echo [!] `%LIBRARY_DEFAULT_PATH%` directory deleted!
    )

    call :cleanup
    echo [!] Cleanup done!
    exit /b 0


:end
    echo [!] Script exiting!
    ENDLOCAL
