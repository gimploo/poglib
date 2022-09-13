@echo off
SETLOCAL

REM ===========================================================================
REM                 -- WINDOWS BUILD SCRIPT FOR C PROJECTS --
REM ===========================================================================

REM Include required dependencies
set LIBRARY_LIST=GLFW SDL2 GLEW FREETYPE POGLIB
set GLFW_URL=https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.bin.WIN64.zip
set SDL2_URL=https://www.libsdl.org/release/SDL2-devel-2.0.20-VC.zip
set GLEW_URL=https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip
set FREETYPE_URL=https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/heads/master.zip
set POGLIB_URL=https://github.com/gimploo/poglib/archive/refs/heads/main.zip


REM Include compiler of choice (here its msvc)
set CC=cl
set CC_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set CC_DEFAULT_FLAGS=/std:c11 /W4 /wd4244 /wd4996 /wd4477 /wd4267 /FC /TC /Zi 
set CC_DEFAULT_LIBS=User32.lib Gdi32.lib Shell32.lib winmm.lib dbghelp.lib shlwapi.lib

REM Source and executalble path (default)
set EXE_FOLDER_DEFAULT_PATH=.\bin
set SRC_FOLDER_DEFAULT_PATH=.\src
set LIBRARY_DEFAULT_PATH=.\lib

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


    echo [*] Checking if all dependenices are installed ...
    if exist "%LIBRARY_DEFAULT_PATH%" (
        echo [!] `lib` directory found!
    ) else (
        echo [!] `lib` directory not found!
        echo [*] Checking dependenices ...
        if "%USERNAME%" == "gokul" (
            mklink /j "%LIBRARY_DEFAULT_PATH%" C:\Users\User\OneDrive\Documents\projects\dev-libs
        ) else (
            call :check_dependencies_are_installed
        )
        echo [!] Dependencies all found!
    )


    echo [*] Checking for `bin` folder ...
    if exist bin (
        echo [!] `bin` directory found!
    ) else (
        echo [!] `bin` directory not found!
        mkdir bin
        call :copy_all_dlls_to_bin
        echo [!] `bin` directory made!
    )

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
        call :run_executable_with_debugger
        goto :end
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

    set INCLUDES=/I %LIBRARY_DEFAULT_PATH%\GLEW\include ^
                    /I %LIBRARY_DEFAULT_PATH%\FREETYPE\include ^
                    /I %LIBRARY_DEFAULT_PATH%\SDL2\include ^
                    /I %LIBRARY_DEFAULT_PATH%\GLFW\include ^
                    /I %LIBRARY_DEFAULT_PATH%\

    if "%~1" == "debug" (
        set FLAGS=/DGLEW_STATIC /DDEBUG
    ) else (
        set FLAGS=/DGLEW_STATIC 
    )

    set LIBS=%LIBRARY_DEFAULT_PATH%\GLEW\lib\Release\x64\glew32s.lib ^
                %LIBRARY_DEFAULT_PATH%\FREETYPE\lib\win64\freetype.lib ^
                %LIBRARY_DEFAULT_PATH%\GLFW\lib\glfw3dll.lib ^
                %LIBRARY_DEFAULT_PATH%\SDL2\lib\x64\SDL2.lib ^
                %LIBRARY_DEFAULT_PATH%\SDL2\lib\x64\SDL2main.lib ^
                Opengl32.lib glu32.lib

    %CC% %CC_DEFAULT_FLAGS% %FLAGS%^
        %INCLUDES% ^
        /Fe%EXE_FOLDER_DEFAULT_PATH%\%EXE_FILE_NAME% ^
        %SRC_FOLDER_DEFAULT_PATH%\%SRC_FILE_NAME% ^
        /link %CC_DEFAULT_LIBS% %LIBS% -SUBSYSTEM:console || echo [!] Failed to compile! && exit /b 1

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

:run_executable_with_debugger
    echo [!] Running executable through the debugger!
    devenv /DebugExe %EXE_FOLDER_DEFAULT_PATH%\%EXE_FILE_NAME%
    exit /b 0

:check_dependencies_are_installed

    mkdir "%LIBRARY_DEFAULT_PATH%"

    pushd %LIBRARY_DEFAULT_PATH% 
        for %%x in (%LIBRARY_LIST%) do (
            if not exist %%x (
                echo [!] `%%x` directory not found!
                call :download_dependency %%x
            ) else (
                echo [!] `%%x` folder found!
            )
        )
    popd
    exit /b 0

:check_compiler_is_installed 
    %CC_PATH% || echo [!] Compiler %CC% not found! && goto :end
    echo [!] Compiler %CC% found!
    exit /b 0

:copy_all_dlls_to_bin
    echo [*] Copying all DLLs to bin ...

    REM ADD New dlls here! 

    pushd %LIBRARY_DEFAULT_PATH%
        if exist SDL2 (
            copy SDL2\lib\x64\SDL2.dll ..\%EXE_FOLDER_DEFAULT_PATH% >nul
        )
        if exist GLFW (
            copy GLFW\lib\glfw3.dll ..\%EXE_FOLDER_DEFAULT_PATH% >nul
        )
    popd

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

    if "%~1" == "FREETYPE" (
        pushd FreeType\
            rename "release static" lib
            move "lib\vs2015-2022\win64" lib\ >nul
            move "lib\vs2015-2022\win32" lib\ >nul
            rd /s /q "lib\vs2015-2022"
        popd
    )

    if "%~1" == "POGLIB" (
        rename "POGLIB" poglib
    )

    if "%~1" == "GLFW" (
        pushd GLFW\
            rename "lib-static-ucrt" lib
        popd
    )

    echo [!] Successfully installed %~1!
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
        if "%USERNAME%" == "gokul" (
            rmdir "%LIBRARY_DEFAULT_PATH%"
        ) else (
            rd /s /q "%LIBRARY_DEFAULT_PATH%"
        )
        echo [!] `%LIBRARY_DEFAULT_PATH%` directory deleted!
    )

    call :cleanup
    echo [!] Cleanup done!
    exit /b 0


:end
    echo [!] Script exiting!
    ENDLOCAL
