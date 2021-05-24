@echo off
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
call vcvars32
@echo on
call copy-files
rmdir /s /q Release
msbuild keymap2cpp.vcxproj /p:Configuration=Release
Release\keymap2cpp
rmdir /s /q Release
msbuild skin2cpp.vcxproj /p:Configuration=Release
Release\skin2cpp
rmdir /s /q Release
msbuild Free42Binary.vcxproj /p:Configuration=Release
move Release\Free42Binary.exe .
rmdir /s /q Release
msbuild Free42Decimal.vcxproj /p:Configuration=Release
move Release\Free42Decimal.exe .
