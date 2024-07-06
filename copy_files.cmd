
echo F|xcopy .\build\bin\Release\Win32\Gate.exe .\build\bin\gm_server\Gate.exe /y
echo F|xcopy .\build\bin\Release\Win32\Service.exe .\build\bin\gm_server\Service.exe /y
echo F|xcopy .\build\bin\Release\Win32\LogicServer.exe .\build\bin\gm_server\LogicServer.exe /y
echo D|xcopy .\build\bin\Release\Win32\plugin\* .\build\bin\gm_server\plugin\ /y
echo F|xcopy .\build\bin\Release\Win32\AdminGate.exe .\build\bin\admin_server\AdminGate.exe /y
echo F|xcopy .\build\bin\Release\Win32\packer.exe .\build\bin\admin_server\packer.exe /y
echo F|xcopy .\build\bin\Release\Win32\packer_tool.exe .\build\bin\admin_server\packer_tool.exe /y
echo F|xcopy .\build\bin\Release\Win32\NewClient.dll .\build\bin\admin_server\NewClient.dll /y
echo F|xcopy .\build\bin\Release\Win32\stage_1_payload.bin .\build\bin\admin_server\stage_1_payload.bin /y
echo F|xcopy .\build\bin\Release\Win32\stage_2_payload.bin .\build\bin\admin_server\stage_2_payload.bin /y
.\build\bin\Release\Win32\packer.exe --make_template_cfg --output .\build\bin\admin_server\config.txt

pause