mkdir join.tmp || exit 1
cd join.tmp  || exit 1
tar -xzvf ../pseint-l64-wx3-$1.tgz  || exit 1
tar -xzvf ../pseint-l64-wx2-$1.tgz  || exit 1
tar -czvf ../pseint-l64-$1.tgz pseint  || exit 1
cd ..  || exit 1
rm -rf join.tmp  || exit 1
