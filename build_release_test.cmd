@ECHO OFF
set InstallerPath="D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE"
if not exist "%InstallerPath%\devenv.exe" echo "VS2022 not found"&&exit /B 0
set "Path=%Path%;%InstallerPath%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4;%ProgramFiles(x86)%\7-Zip"
set ReleaseDir=.\build\bin\Release\Win32
set ReleaseDir_vip=.\build\bin\Release_vip\Win32
set OutputDir=.\build\bin\%2
set PATH=C:\Program Files\7-Zip\;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4\;%PATH%
mkdir %OutputDir%

set gm_server=%OutputDir%gm_server_vip\%2
set admin_server=%OutputDir%admin_server\%2
mkdir %gm_server%

::echo ***************gm_server***************
::VMProtect_Con.exe %ReleaseDir%\Service.exe %gm_server%\g_Service.exe -pf ServiceRelease_vip.vmp
::VMProtect_Con.exe %ReleaseDir%\LogicServer.exe %gm_server%\g_LogicServer.exe -pf LogicRelease_vip.vmp
VMProtect_Con.exe %ReleaseDir_vip%\GateF.exe %gm_server%\及时雨内部版.exe -pf GateRelease_vip.vmp
::VMProtect_Con.exe %ReleaseDir%\AdminGate.exe %admin_server%\AdminGate.exe -pf AdminGateRelease.vmp

pause