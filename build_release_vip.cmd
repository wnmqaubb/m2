@ECHO OFF
set InstallerPath="D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE"
if not exist "%InstallerPath%\devenv.exe" echo "VS2022 not found"&&exit /B 0
set "Path=%Path%;%InstallerPath%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4;%ProgramFiles(x86)%\7-Zip"
set ReleaseDir=.\build\bin\Release\Win32
set ReleaseDir_vip=.\build\bin\Release_vip\Win32
set OutputDir=.\build\bin\%2
set PATH=C:\Program Files\7-Zip\;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4\;%PATH%
mkdir %OutputDir%

::"D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.exe" AntiCheat.sln /build "Release|x86"
::"D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.exe" Tools/PackerTool/packer_tool.vcxproj /rebuild "Release|x86"


::echo ***************构建方案***************
::devenv.exe AntiCheat.sln /rebuild "Release2|x86"

set gm_server=%OutputDir%gm_server\%2
set admin_server=%OutputDir%admin_server\%2

::echo ***************构建方案***************
::echo D|xcopy %ReleaseDir%\Service.exe %gm_server% /y
::echo D|xcopy %ReleaseDir%\LogicServer.exe %gm_server% /y
::echo D|xcopy %ReleaseDir%\Gate.exe %gm_server% /y

::echo ***************gm_server***************
VMProtect_Con.exe %ReleaseDir%\Service.exe %gm_server%\g_Service.exe -pf ServiceRelease_vip.vmp
VMProtect_Con.exe %ReleaseDir%\LogicServer.exe %gm_server%\g_LogicServer.exe -pf LogicRelease_vip.vmp
VMProtect_Con.exe %ReleaseDir_vip%\GateF.exe %gm_server%\及时雨内部版.exe -pf GateRelease_vip.vmp
VMProtect_Con.exe %ReleaseDir%\AdminGate.exe %bin_Dir%\admin_server\AdminGate.exe -pf AdminGateRelease.vmp
echo D|xcopy %ReleaseDir%\plugin\* %gm_server%plugin\ /y

::echo ***************admin_server***************
echo F|xcopy %ReleaseDir%\AdminGate.exe %admin_server%AdminGate.exe /y
echo F|xcopy %ReleaseDir%\packer.exe %admin_server%packer.exe /y
echo F|xcopy %ReleaseDir%\packer_tool.exe %admin_server%packer_tool.exe /y
echo F|xcopy %ReleaseDir%\NewClient.dll %admin_server%NewClient.dll /y
echo F|xcopy %ReleaseDir%\stage_1_payload.bin %admin_server%stage_1_payload.bin /y
echo F|xcopy %ReleaseDir%\stage_2_payload.bin %admin_server%stage_2_payload.bin /y

pause