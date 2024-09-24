set ReleaseDir=.\build\bin\Release\Win32
set PdbDir=.\build\pdb
set OutputDir=.\build\zip\%2
set PATH=C:\Program Files\7-Zip\;D:\tool\52pojie\Tools\Packers\VMProtect Ultimate3.4\;%PATH%
mkdir %OutputDir%
set gm_server=.\build\bin\gm_server
set admin_server=.\build\bin\admin_server
set gm_server_zip=%OutputDir%gm_server
set admin_server_zip=%OutputDir%admin_server

mkdir %gm_server_zip%
mkdir %admin_server_zip%

echo F|xcopy %gm_server%\Service.exe %gm_server_zip%\Service.exe /y
echo F|xcopy %gm_server%\LogicServer.exe %gm_server_zip%\LogicServer.exe /y
echo F|xcopy %gm_server%\Gate.exe %gm_server_zip%\Gate.exe /y
echo F|xcopy %gm_server%\serial_demo.txt %gm_server_zip%\serial.txt /y

echo D|xcopy %gm_server%\plugin\* %gm_server_zip%\plugin\ /y

echo F|xcopy %ReleaseDir%\*.exe %OutputDir%all\ /y
echo F|xcopy %ReleaseDir%\*.dll %OutputDir%all\ /y
echo F|xcopy %ReleaseDir%\*.bin %OutputDir%all\ /y
echo F|xcopy %PdbDir%\*.pdb %OutputDir%\all\ /y

7z a -tzip %OutputDir%pdbs.zip %OutputDir%all\*
::7z a -tzip %admin_server_zip%.zip %admin_server_zip%\*
7z a -tzip %gm_server_zip%.zip %gm_server_zip%\*
7z a -tzip %OutputDir%%1.zip %admin_server_zip%.zip %gm_server_zip%.zip

::del %OutputDir%\gm_server.zip
::del %admin_server_zip%.zip
rmdir /q /s %OutputDir%all
::rmdir /q /s %admin_server_zip%
rmdir /q /s %gm_server_zip%

pause
