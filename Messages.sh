#! /bin/bash 
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/kimtoy.pot
rm -f rc.cpp
