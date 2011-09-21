#! /bin/bash 
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/kimtoy.pot
rm -f rc.cpp
