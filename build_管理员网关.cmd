@ECHO OFF
set "Path=%Path%;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4;"
set ReleaseDir=.\build\bin\Release\Win32
set OutputDir=.\build\bin\%2
mkdir %OutputDir%

set admin_server=%OutputDir%admin_server\%2

echo ***************��װ�����Ա��̨***************
VMProtect_Con.exe %ReleaseDir%\AdminGate.exe %admin_server%\��װ�����Ա��̨.exe -pf AdminGateRelease.vmp


pause