@ECHO OFF
set ReleaseDir=.\build\bin\Release\Win32
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
VMProtect_con.exe agent_Release2.vmp
VMProtect_Con.exe %ReleaseDir%\Service.exe %bin_Dir%\gm_server\Service.exe -pf ServiceRelease.vmp
VMProtect_Con.exe %ReleaseDir%\LogicServer.exe %bin_Dir%\gm_server\LogicServer.exe -pf LogicRelease.vmp
VMProtect_Con.exe %ReleaseDir%\Gate.exe %bin_Dir%\gm_server\Gate.exe -pf GateRelease.vmp
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