echo off
if %1.==. goto fail
set arg1=%1
..\build\release\riscvtool.exe %arg1% -run \\.\COM9
goto end

:fail
echo Usage: run.bat binaryname

:end
