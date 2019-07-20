@echo off
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
call vcvars32
@echo on
msbuild %1.vcxproj /p:OutDir=build\%1\ /p:IntDir=build\%1\ /p:Configuration=Release
