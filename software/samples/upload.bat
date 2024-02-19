echo off
if %1.==. goto fail
set arg1=%1
..\build\release\riscvtool.exe %arg1% -sendfile \\.\COM3
goto end

:fail
echo Usage: upload.bat binaryfilename

:end
