#!/bin/sh

if [ "$1" = "" ]; then 
  echo "Use: $0 langcode"
else

  make -C ../../psexport -f Makefile.lnx || exit 1
  make -C ../../pseint -f Makefile.lnx || exit 1

  if ! test -e $1; then
    mkdir $1
  fi
  cd psc
  
  ebin=../../../bin/psexport
  ibin=../../../bin/pseint
  
  for A in *.psc; do
  
    echo "=== $A"
  
    B=$(echo $A|sed 's/psc/psd/')
    C=$(echo $A|sed 's/psc/'$1'/')
    
    $ibin --allowddims --nouser $A --draw ../temp/$B
    
    if ! test -e ../$1/$C; then
      $ebin --for-testing --lang=$1 ../temp/$B ../$1/$C >/dev/null
    else
      $ebin --for-testing --lang=$1 ../temp/$B ../temp/$C >/dev/null
      if ! diff --strip-trailing-cr ../temp/$C ../$1/$C; then read; fi
    fi
  done
fi  