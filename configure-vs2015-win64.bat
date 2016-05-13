MKDIR build64
PUSHD build64
cmake %* .. -G "Visual Studio 14 2015 Win64"
POPD
PAUSE
