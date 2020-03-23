# Para armar el paquete de 64bits que funciones tanto con wx2+gtk2 como con wx3+gt3,
# armo con el script pseint-packer.sh dos versiones, una en ubuntu 12.04 con wx2+gt2,
# que usa el tag l64-wx2, y otra en ubuntu 16.04 con wx3+gtk que usa el tag l64-wx3.
# Este script combina esa dos en el paquete final que subo al sitio web.
mkdir join.tmp || exit 1
cd join.tmp  || exit 1
tar -xzvf ../pseint-l64-wx3-$1.tgz  || exit 1
tar -xzvf ../pseint-l64-wx2-$1.tgz  || exit 1
tar -czvf ../pseint-l64-$1.tgz pseint  || exit 1
cd ..  || exit 1
rm -rf join.tmp  || exit 1
