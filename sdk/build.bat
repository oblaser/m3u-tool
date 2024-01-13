@rem    author          Oliver Blaser
@rem    date            12.01.2024
@rem    copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser

setlocal

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cd .\omw\build\vs\
msbuild omw.sln /p:Configuration=Debug /p:Platform=x86
msbuild omw.sln /p:Configuration=Release /p:Platform=x86
@rem msbuild omw.sln /p:Configuration=Debug /p:Platform=x64
@rem msbuild omw.sln /p:Configuration=Release /p:Platform=x64
cd ..\..\..\

cd .\omw\tests\unit\vs\
msbuild omw-tests-unit.sln /p:Configuration=Release /p:Platform=x86
cd Release
echo.
echo ###############################################################################
omw-tests-unit.exe
cd ..\..\..\..\..\

pause

endlocal
