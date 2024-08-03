cd /d %~dp0
del /s /q symbol\
mkdir bin\packer
mkdir symbol
copy *.pdb symbol\
copy *.map symbol\
copy config.json bin\
copy client.dll bin\packer
copy packer.exe bin\packer
copy packer_tool.exe bin\packer\·â×°Æ÷.exe
