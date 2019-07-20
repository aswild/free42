set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
call vcvars32
call link-files
msbuild keymap2cpp.vcxproj -p:Configuration=Release
Release\keymap2cpp
msbuild skin2cpp.vcxproj -p:Configuration=Release
Release\skin2cpp
msbuild Free42Binary.vcxproj -p:Configuration=Release
move Release\Free42Binary.exe .
rmdir /s /q Release
msbuild Free42Decimal.vcxproj -p:Configuration=Release
move Release\Free42Decimal.exe .
::rmdir /s /q Release
