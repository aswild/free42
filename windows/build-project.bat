set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
call vcvars32
msbuild %1.vcxproj -p:Configuration=Release
