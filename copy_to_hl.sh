
TARGET=../hl2-vr-bkp/src/vr_io

cp dependencies/**/*.dll $TARGET
cp dependencies/**/**/*.dll $TARGET
cp Debug/vr.io.dll $TARGET
cp Debug/vr.io.lib $TARGET
cp vr.io/vr_io.h $TARGET