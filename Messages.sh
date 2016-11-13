#! /bin/bash 
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp fileitemaction/*.cpp *.h -o $podir/kimtoy.pot
rm -f rc.cpp
