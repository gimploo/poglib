@echo off

REM =============================================================================================
REM                            -- WINDOWS BUILD SCRIPT FOR C PROJECTS --
REM =============================================================================================

REM Include required dependencies
set DEPENDENCY_LIST=SDL2 GLEW
set SDL2_URL=https://www.libsdl.org/release/SDL2-devel-2.0.16-VC.zip
set GLEW_URL=https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip

REM Include compiler of choice (here its msvc)
set CC=cl
set CC_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set CC_DEFAULT_FLAGS=/std:c11 /W4 /wd4244 /wd4996 /wd5105 /FC /TC /Zi 
set CC_DEFAULT_LIBS=User32.lib Gdi32.lib Shell32.lib

REM Source and executalble path (default)
set EXE_DEFAULT_PATH=.\bin
set SRC_DEFAULT_PATH=.
set DEPENDENCY_DEFAULT_PATH=.\external

REM Source files and exe name
set SRC_FILES=test01.c
set EXE=test.exe



:main
    if "%1" == "deepclean" (
        call :deepcleanup
        exit /b 0
    )

    if "%1" == "clean" (
        call :cleanup
        exit /b 0
    )

    cls
    echo [*] Running build script for windows...
    
    echo [*] Checking %CC% compiler is installed ...
    call :check_compiler_is_installed || goto :EOF


    echo [*] Checking if all dependenices are installed ...
    if exist "%DEPENDENCY_DEFAULT_PATH%" (
        echo [!] External directory found!
    ) else (
        echo [!] External directory not found!
        mkdir "%DEPENDENCY_DEFAULT_PATH%"
    )

    echo [*] Checking dependenices ...
    call :check_dependencies_are_installed || goto :EOF
    echo [!] Dependencies all found!

    if exist bin (
        echo [!] Bin directory found!
    ) else (
        echo [!] Bin directory not found!
        mkdir bin
        echo [!] Bin directory made!
    )

    echo [*] Building project ...
    call :build_project_with_msvc || goto :EOF

    if %errorlevel% == 0 (
        echo [*] Running executable ...
        call :run_executable || goto :EOF

        if %errorlevel% neq 0 (
            echo [*] Running executable through the debugger ...
            call :run_executable_with_debugger || goto :EOF
        )
    )
    

    echo [!] Exited! 
    exit /b 0


REM ==================================================================================
REM                         -- BUILD RECIPE --
REM ==================================================================================
REM (change whats in here to ) -
REM                            |
REM                            v
:build_project_with_msvc
    SETLOCAL

    set INCLUDES=/I %DEPENDENCY_DEFAULT_PATH%\SDL2\include ^
                    /I %DEPENDENCY_DEFAULT_PATH%\GLEW\include

    set FLAGS=/DGLEW_STATIC 

    set LIBS=%DEPENDENCY_DEFAULT_PATH%\SDL2\lib\x64\SDL2.lib ^
                %DEPENDENCY_DEFAULT_PATH%\SDL2\lib\x64\SDL2main.lib ^
                %DEPENDENCY_DEFAULT_PATH%\GLEW\lib\Release\x64\glew32s.lib ^
                Opengl32.lib glu32.lib

    cl %CC_DEFAULT_FLAGS% %FLAGS%^
        %INCLUDES% ^
        /Fe%EXE_DEFAULT_PATH%\%EXE% ^
        %SRC_DEFAULT_PATH%\%SRC_FILES% ^
        /link %CC_DEFAULT_LIBS% %LIBS% -SUBSYSTEM:windows


    ENDLOCAL
    exit /b %errorlevel%




REM =======================================================================================
REM                             -- HELPER FUNCTIONS --
REM =======================================================================================
    
:run_executable
    %EXE_DEFAULT_PATH%\%EXE%
    exit /b %errorlevel% 

:run_executable_with_debugger
    devenv /DebugExe %EXE_DEFAULT_PATH%\%EXE%
    exit /b 0

:deepcleanup
    echo [*] Cleanup in progress ...
    if exist "%DEPENDENCY_DEFAULT_PATH%" (
        rd /s /q "%DEPENDENCY_DEFAULT_PATH%"
        echo [!] %DEPENDENCY_DEFAULT_PATH% directory deleted!
    )
    call :cleanup
    echo [!] Cleanup done!
    exit /b 0

:cleanup
    if exist bin (
        rd /s /q bin
        echo [!] bin directory deleted!
    )
    del /s *.pdb *.obj 2>nul
    exit /b 0

:check_dependencies_are_installed
    pushd %DEPENDENCY_DEFAULT_PATH%
        for %%x in (%DEPENDENCY_LIST%) do (
            if not exist %%x (
                echo [!] %%x directory not found!
                call :download_dependency %%x
            ) else (
                echo [!] %%x folder found!
            )
        )
    popd
    exit /b 0

:check_compiler_is_installed 
    %CC_PATH% || echo [!] Compiler %CC% not found! && goto :EOF
    echo [!] Compiler %CC% found!
    exit /b 0

:download_dependency
    echo [*] Installing %~1 ...
    call echo [!] link: %%%~1_URL%%%

    call curl -L --output %~1.zip %%%~1_URL%%% 
    mkdir %~1 
    tar -xf %~1.zip -C %~1 --strip-components 1 && del %~1.zip

    REM (to future me) U had sdl2 setup this way in ubuntu 
    if "%~1" == "SDL2" (
        pushd SDL2\include
            mkdir SDL2
            move *.h SDL2\ >nul
        popd
    )

    echo [!] Successfully installed %~1!
    exit /b 0

