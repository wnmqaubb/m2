@ECHO OFF
set ReleaseDir=.\build\bin\Release\Win32
set ReleaseDir_red=.\build\bin\Release_red\Win32
set ReleaseDir_blue=.\build\bin\Release_blue\Win32
set bin_Dir=.\build\bin
set "InstallerPath=D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE"
if not exist "%InstallerPath%\devenv.exe" echo "VS2022 not found"&&exit /B 0
set "Path=%Path%;%InstallerPath%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4;%ProgramFiles(x86)%\7-Zip"

echo ***************������������ʼ***************
::devenv.exe AntiCheat.sln /rebuild "Release|x86"
echo ***************�������������***************



echo ***************Shell�����ʼ***************
::call project\shellcode\shell_pack.bat Release
::call project\shellcode\shell_pack.bat Release2
echo ***************Shell������***************



echo ***************ִ�����ؼӿǿ�ʼ***************
VMProtect_Con.exe %ReleaseDir%\g_Service.exe %bin_Dir%\gm_server\g_Service.exe -pf ServiceRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir%\g_LogicServer.exe %bin_Dir%\gm_server\g_LogicServer.exe -pf LogicRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir_blue%\Gate.exe %bin_Dir%\gm_server\��ʱ�궨��.exe -pf GateRelease_blue.vmp
VMProtect_Con.exe %ReleaseDir%\AdminGate.exe %bin_Dir%\admin_server\AdminGate.exe -pf AdminGateRelease_blue.vmp
echo ***************ִ�����ؼӿ����***************

echo ***************ִ��DLL�ӿǿ�ʼ***************
::VMProtect_con.exe client_Release.vmp
::VMProtect_con.exe client_Release2.vmp
echo ***************ִ��DLL�ӿ����***************



echo ***************ѹ���������ʼ***************
::del /q Release\Release.zip
::7z a Release\Release.zip	.\Release\bin\*
::del /q Release2\Release2.zip
::7z a Release2\Release2.zip	.\Release2\bin\*
echo ***************ѹ������������***************


pause