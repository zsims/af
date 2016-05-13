MKDIR build32
PUSHD build32
cmake %* .. -G "Visual Studio 14 2015"
POPD
PAUSE