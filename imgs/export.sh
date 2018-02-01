for A in $3; do
  for B in $2; do
    inkscape $1.svg --export-id=id_$A --export-png=../bin/imgs/$1/$B/$A.png -w $B -h $B
  done
done
