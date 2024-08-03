@ECHO OFF
set "InstallerPath=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\Common7\IDE"
if not exist "%InstallerPath%\devenv.exe" echo "VS2017 not found"&&exit /B 0
set "Path=%Path%;%InstallerPath%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4;%ProgramFiles(x86)%\7-Zip"

"D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.exe" AntiCheat.sln /build "Release|x86"
"D:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.exe" Tools/PackerTool/packer_tool.vcxproj /rebuild "Release|x86"


echo ***************������������ʼ***************
::devenv.exe AntiCheat.sln /rebuild "Release|x86"
devenv.exe AntiCheat.sln /rebuild "Release2|x86"
echo ***************�������������***************



echo ***************Shell�����ʼ***************
::call project\shellcode\shell_pack.bat Release
call project\shellcode\shell_pack.bat Release2
echo ***************Shell������***************



echo ***************ִ�����ؼӿǿ�ʼ***************
::VMProtect_con.exe agent_Release.vmp
VMProtect_con.exe agent_Release2.vmp
echo ***************ִ�����ؼӿ����***************

echo ***************ִ��DLL�ӿǿ�ʼ***************
::VMProtect_con.exe client_Release.vmp
VMProtect_con.exe client_Release2.vmp
echo ***************ִ��DLL�ӿ����***************



echo ***************ѹ���������ʼ***************
::del /q Release\Release.zip
::7z a Release\Release.zip	.\Release\bin\*
del /q Release2\Release2.zip
7z a Release2\Release2.zip	.\Release2\bin\*
echo ***************ѹ������������***************


pause