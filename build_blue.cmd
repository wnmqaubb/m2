@ECHO OFF
set ReleaseDir=.\build\bin\Release\Win32
set ReleaseDir_red=.\build\bin\Release_red\Win32
set ReleaseDir_blue=.\build\bin\Release_blue\Win32
set bin_Dir=.\build\bin
set "InstallerPath=D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE"
if not exist "%InstallerPath%\devenv.exe" echo "VS2022 not found"&&exit /B 0
set "Path=%Path%;%InstallerPath%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4;%ProgramFiles(x86)%\7-Zip"

echo ***************编译解决方案开始***************
::devenv.exe AntiCheat.sln /rebuild "Release|x86"
echo ***************编译解决方案完成***************



echo ***************Shell打包开始***************
::call project\shellcode\shell_pack.bat Release
::call project\shellcode\shell_pack.bat Release2
echo ***************Shell打包完成***************



echo ***************执行网关加壳开始***************
VMProtect_Con.exe %ReleaseDir%\g_Service.exe %bin_Dir%\gm_server\g_Service.exe -pf ServiceRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir%\g_LogicServer.exe %bin_Dir%\gm_server\g_LogicServer.exe -pf LogicRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir_blue%\Gate.exe %bin_Dir%\gm_server\及时雨定制.exe -pf GateRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir%\AdminGate.exe %bin_Dir%\admin_server\AdminGate.exe -pf AdminGateRelease_blue.vmp
echo ***************执行网关加壳完成***************

echo ***************执行DLL加壳开始***************
::VMProtect_con.exe client_Release.vmp
::VMProtect_con.exe client_Release2.vmp
echo ***************执行DLL加壳完成***************



echo ***************压缩打包程序开始***************
::del /q Release\Release.zip
::7z a Release\Release.zip	.\Release\bin\*
::del /q Release2\Release2.zip
::7z a Release2\Release2.zip	.\Release2\bin\*
echo ***************压缩打包程序完成***************


pause