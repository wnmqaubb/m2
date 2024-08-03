set ReleaseDir=.\build\bin\Release\Win32
set PdbDir=.\build\pdb
set OutputDir=.\build\zip\%2
set PATH=C:\Program Files\7-Zip\;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4\;%PATH%
mkdir %OutputDir%

echo D|xcopy %ReleaseDir%\Service.exe %OutputDir%\gm_server\ /y
echo D|xcopy %ReleaseDir%\LogicServer.exe %OutputDir%\gm_server\ /y
echo D|xcopy %ReleaseDir%\Gate.exe %OutputDir%\gm_server\ /y

VMProtect_Con.exe %ReleaseDir%\Service.exe %OutputDir%\gm_server\Service.exe -pf ServiceRelease.vmp
VMProtect_Con.exe %ReleaseDir%\LogicServer.exe %OutputDir%\gm_server\LogicServer.exe -pf LogicRelease.vmp
VMProtect_Con.exe %ReleaseDir%\Gate.exe %OutputDir%\gm_server\Gate.exe -pf GateRelease.vmp

echo D|xcopy %OutputDir%\gm_server\Service.exe .\build\bin\gm_server\Service.exe /y
echo D|xcopy %OutputDir%\gm_server\LogicServer.exe .\build\bin\gm_server\LogicServer.exe /y
echo D|xcopy %OutputDir%\gm_server\Gate.exe .\build\bin\gm_server\Gate.exe /y
echo D|xcopy %ReleaseDir%\plugin\* .\build\bin\gm_server\plugin\ /y
echo F|xcopy %ReleaseDir%\AdminGate.exe .\build\bin\admin_server\AdminGate.exe /y
echo F|xcopy %ReleaseDir%\packer.exe .\build\bin\admin_server\packer.exe /y
echo F|xcopy %ReleaseDir%\packer_tool.exe .\build\bin\admin_server\packer_tool.exe /y
echo F|xcopy %ReleaseDir%\NewClient.dll .\build\bin\admin_server\NewClient.dll /y
echo F|xcopy %ReleaseDir%\stage_1_payload.bin .\build\bin\admin_server\stage_1_payload.bin /y
echo F|xcopy %ReleaseDir%\stage_2_payload.bin .\build\bin\admin_server\stage_2_payload.bin /y


echo D|xcopy %ReleaseDir%\plugin\* %OutputDir%\gm_server\plugin\ /y
echo F|xcopy %ReleaseDir%\AdminGate.exe %OutputDir%\admin_server\AdminGate.exe /y
echo F|xcopy %ReleaseDir%\packer.exe %OutputDir%\admin_server\packer.exe /y
echo F|xcopy %ReleaseDir%\packer_tool.exe %OutputDir%\admin_server\packer_tool.exe /y
echo F|xcopy %ReleaseDir%\NewClient.dll %OutputDir%\admin_server\NewClient.dll /y
echo F|xcopy %ReleaseDir%\stage_1_payload.bin %OutputDir%\admin_server\stage_1_payload.bin /y
echo F|xcopy %ReleaseDir%\stage_2_payload.bin %OutputDir%\admin_server\stage_2_payload.bin /y

echo F|xcopy %ReleaseDir%\*.exe %OutputDir%\all\ /y
echo F|xcopy %ReleaseDir%\*.dll %OutputDir%\all\ /y
echo F|xcopy %ReleaseDir%\*.bin %OutputDir%\all\ /y
echo F|xcopy %PdbDir%\*.pdb %OutputDir%\all\ /y
7z a -tzip %OutputDir%\pdbs.zip %OutputDir%\all\*
7z a -tzip %OutputDir%\admin_server.zip %OutputDir%\admin_server\*
7z a -tzip %OutputDir%\gm_server.zip %OutputDir%\gm_server\*
7z a -tzip %OutputDir%\%1.zip %OutputDir%\admin_server.zip %OutputDir%\gm_server.zip

del %OutputDir%\gm_server.zip
del %OutputDir%\admin_server.zip
rmdir /q /s %OutputDir%\all
rmdir /q /s %OutputDir%\admin_server
rmdir /q /s %OutputDir%\gm_server
