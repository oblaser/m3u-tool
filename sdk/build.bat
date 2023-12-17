@rem    author          Oliver Blaser
@rem    date            17.12.2023
@rem    copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cd .\omw\build\vs\
msbuild omw.sln /p:Configuration=Debug /p:Platform=x86
msbuild omw.sln /p:Configuration=Release /p:Platform=x86
@rem msbuild omw.sln /p:Configuration=Debug /p:Platform=x64
@rem msbuild omw.sln /p:Configuration=Release /p:Platform=x64
cd ..\..\..\

pause
