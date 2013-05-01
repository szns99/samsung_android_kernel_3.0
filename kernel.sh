rm .version
make -j$(grep processor /proc/cpuinfo | awk '{field=$NF};END{print field+1}')
rm ../Image/zImage
mkdir -p ../Image
cp arch/arm/boot/zImage ../Image
