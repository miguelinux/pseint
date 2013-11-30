#!/bin/sh

if [ "$1" = "" ]; then 
  echo "Use: $0 langcode"
else
  
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
    
    if ! test -e ../$1/$A; then
      $ebin --lang=$1 ../temp/$B ../$1/$C >/dev/null
    else
      eibin --lang=$1 ../temp/$B ../temp/$C >/dev/null
      diff ../temp/$C ../$1/$C
    fi
  done
fi  