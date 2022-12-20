
# sh/ptroff.sysv
# Copyright (c) 1985 Adobe Systems Incorporated
# RCSID: $Header: ptroff.sysv,v 2.1 85/11/24 12:32:39 shore Rel $
# @(#)Copyright Apple Computer 1987\tVersion 1.1 of ptroff.sh on 87/05/04 19:05:47
#
# run old troff in a System V environment to print on a PostScript printer
#
# ptroff - otroff | pscat [| lp]

opt= spool= 
family=Times
offset="-y -31"
printer=-d${LPDEST-PostScript}
while test "$1" != ""
do	case "$1" in
	-F)	if test "$#" -lt 2 ; then
			echo '-F takes following font family name' 1>&2
			exit 1 
		fi
		family=$2 ; shift ;;
	-F*)	echo 'use -F familyname' 1>&2 ;
		exit 1 ;;
	-t)	nospool=1 ;;
	-n*|-m|-w|-s)	spool="$spool $1" ;;
	-r|-h)	spool="$spool -o$1" ;;
	-d*)	printer=$1 ;;
	-)	fil="$fil $1" ;;
	-*)	opt="$opt $1" ;;

	*)	fil="$fil $1" ; jobname=${jobname-$1} ;;
	esac
	shift
done
if test "$jobname" = "" ; then
	jobname="otroff"
fi
spool="$printer -t'$jobname' $spool"
if test "$fil" = "" ; then
	fil="-"
fi
troff="otroff -t -Tps $opt /usr/lib/font/ps/${family}.head $fil "
pscat="pscat $offset -F/usr/lib/font/ps/${family}.ct "

if test "$nospool" = "1" ; then
	$troff | $pscat
else
	$troff | $pscat | lp $spool
fi
