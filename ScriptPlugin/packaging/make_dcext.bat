set PLUGIN_NAME=ScriptPlugin

cd ..\projects\make

set PATH=H:\MinGW\bin
mingw32-make

set PATH=H:\mingw64\bin
mingw32-make

cd ..\..\packaging

copy ..\projects\make\build-mingw-x64\%PLUGIN_NAME%.dll %PLUGIN_NAME%-x64.dll
copy ..\projects\make\build-mingw-x64\%PLUGIN_NAME%.pdb %PLUGIN_NAME%-x64.pdb
copy ..\projects\make\build-mingw-x86\%PLUGIN_NAME%.dll %PLUGIN_NAME%-x86.dll
copy ..\projects\make\build-mingw-x86\%PLUGIN_NAME%.pdb %PLUGIN_NAME%-x86.pdb

set PATH=C:\Cygwin\bin
zip -9 -r %PLUGIN_NAME%.dcext info.xml %PLUGIN_NAME%-x64.dll %PLUGIN_NAME%-x86.dll %PLUGIN_NAME%-x64.pdb %PLUGIN_NAME%-x86.pdb %PLUGIN_NAME%.ico scripts

del %PLUGIN_NAME%-x64.dll %PLUGIN_NAME%-x86.dll %PLUGIN_NAME%-x64.pdb %PLUGIN_NAME%-x86.pdb

pause
