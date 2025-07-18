set ReleaseDir=.\build\bin\Release\Win32
set PdbDir=.\build\pdb
set OutputDir=.\build\zip\%2
set PATH=C:\Program Files\7-Zip\;C:\Program Files\VMProtect Ultimate\;%PATH%
mkdir %OutputDir%

echo D|xcopy %ReleaseDir%\Service.exe %OutputDir%\gm_server\ /y
echo D|xcopy %ReleaseDir%\LogicServer.exe %OutputDir%\gm_server\ /y
echo D|xcopy %ReleaseDir%\Gate.exe %OutputDir%\gm_server\ /y

VMProtect_Con.exe %ReleaseDir%\Service.exe %OutputDir%\gm_server\Service.exe -pf ServiceRelease.vmp
VMProtect_Con.exe %ReleaseDir%\LogicServer.exe %OutputDir%\gm_server\LogicServer.exe -pf LogicRelease.vmp
VMProtect_Con.exe %ReleaseDir%\Gate.exe %OutputDir%\gm_server\Gate.exe -pf GateRelease.vmp

echo F|xcopy %ReleaseDir%\AdminGate.exe %OutputDir%\admin_server\AdminGate.exe /y
echo F|xcopy %ReleaseDir%\NewClient.dll %OutputDir%\admin_server\NewClient.dll /y

::echo F|xcopy %ReleaseDir%\*.exe %OutputDir%\all\ /y
::echo F|xcopy %ReleaseDir%\*.dll %OutputDir%\all\ /y
::echo F|xcopy %ReleaseDir%\*.bin %OutputDir%\all\ /y
::echo F|xcopy %PdbDir%\*.pdb %OutputDir%\all\ /y
::7z a -tzip %OutputDir%\pdbs.zip %OutputDir%\all\*
::7z a -tzip %OutputDir%\admin_server.zip %OutputDir%\admin_server\*
::7z a -tzip %OutputDir%\gm_server.zip %OutputDir%\gm_server\*
::7z a -tzip %OutputDir%\%1.zip %OutputDir%\admin_server.zip %OutputDir%\gm_server.zip

::del %OutputDir%\gm_server.zip
::del %OutputDir%\admin_server.zip
::rmdir /q /s %OutputDir%\all
::rmdir /q /s %OutputDir%\admin_server
::rmdir /q /s %OutputDir%\gm_server
