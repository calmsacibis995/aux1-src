
# transcript/sh/psroff.sysv
# Copyright (c) 1985 Adobe Systems Incorporated
# PostScript is a trademark of Adobe Systems Incorporated
# RCSID: $Header: psroff.sysv,v 2.1 85/11/24 12:32:15 shore Rel $
# @(#)Copyright Apple Computer 1987\tVersion 1.3 of psroff.sh on 87/09/11 17:00:33
#
# run ditroff in an System V environment to print on a PostScript printer
#
# pstroff - ditroff | psdit [| lp]
#

ditroff=troff
psdit=psdit
nospool= dopt= fil= spool= dit=
printer=-d${LPDEST-PostScript}
while test $# != 0
do	case "$1" in
	-t)	nospool=1 ;;
	-Tpsc)	;;
	-T*)	echo only -Tpsc is valid 1>&2 ; exit 2 ;;
	-n*|-m|-w|-s)	spool="$spool $1" ;;
	-r|-h)	spool="$spool -o$1" ;;
	-d*)	printer=$1 ;;
	-)	fil="$fil $1" ;;
	-*)	dopt="$dopt $1" ;;
	*)	fil="$fil $1" ; jobname=${jobname-$1} ;;
	esac
	shift
done
if test "$jobname" = "" ; then
	jobname="Troff"
fi
spool="lp $printer -t'$jobname' $spool"
if test "$fil" = "" ; then
	fil="-"
fi
dit="$ditroff -Tpsc  $dopt $fil "

if test "$nospool" = "1" ; then
	$dit | $psdit
else
	$dit | $psdit | $spool
fi
