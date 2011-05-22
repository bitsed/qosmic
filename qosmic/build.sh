#!/bin/bash

function die()
{
	echo $1 ; exit -1 ;
}

if test "$1" = "clean" ; then
	make distclean
	rm -rf .obj .moc .ui .res
else
	echo "Building Qosmic"
	qmake || die "error while running qmake"
	make || die "error while building qosmic"
	echo
	echo "now type ./qosmic to run the application"
	echo
fi

