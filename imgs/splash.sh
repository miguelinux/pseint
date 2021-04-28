SIZE=150

mkdir -p ../temp/imgs
inkscape splash.svg --export-id=splash_light --export-png=../temp/imgs/splash_light.png -h $SIZE
inkscape splash.svg --export-id=splash_dark  --export-png=../temp/imgs/splash_dark.png -h $SIZE

W=$(file ../bin/imgs/splash_light.png |cut -d ',' -f 2|sed 's/ //g'|cut -d x -f 1)
H=$(file ../bin/imgs/splash_light.png |cut -d ',' -f 2|sed 's/ //g'|cut -d x -f 2)
let W=$W-4
let H=$H-4

convert -crop ${W}x${H}+1+1 ../temp/imgs/splash_light.png ../bin/imgs/splash_light.png
convert -crop ${W}x${H}+1+1 ../temp/imgs/splash_dark.png  ../bin/imgs/splash_dark.png
