@ECHO OFF
set InstallerPath="D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE"
if not exist "%InstallerPath%\devenv.exe" echo "VS2022 not found"&&exit /B 0
set "Path=%Path%;%InstallerPath%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4_;%ProgramFiles(x86)%\7-Zip"
set ReleaseDir=.\build\bin\Release\Win32
set OutputDir=.\build\bin\%2
set PATH=C:\Program Files\7-Zip\;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4\;%PATH%
mkdir %OutputDir%

set gm_server=%OutputDir%及时雨封装定制版\%2
set admin_server=%OutputDir%admin_server\%2
mkdir %gm_server%

::echo ***************gm_server***************
VMProtect_Con.exe %ReleaseDir%\g_Service.exe %gm_server%\g_Service.exe -pf ServiceRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir%\g_LogicServer.exe %gm_server%\g_LogicServer.exe -pf LogicRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir%\GateF.exe %gm_server%\及时雨定制版.exe -pf GateRelease_blue.vmp
::VMProtect_Con.exe %ReleaseDir%\AdminGate.exe %admin_server%\AdminGate.exe -pf AdminGateRelease.vmp
echo D|xcopy %ReleaseDir%\plugin\* %gm_server%plugin\ /y

::echo ***************admin_server***************
echo F|xcopy %ReleaseDir%\packer.exe %admin_server%packer.exe /y
::echo F|xcopy %ReleaseDir%\packer_tool.exe %admin_server%packer_tool.exe /y
echo F|xcopy %ReleaseDir%\NewClient.dll %admin_server%NewClient.dll /y
echo F|xcopy %ReleaseDir%\stage_1_payload.bin %admin_server%stage_1_payload.bin /y
echo F|xcopy %ReleaseDir%\stage_2_payload.bin %admin_server%stage_2_payload.bin /y

pause