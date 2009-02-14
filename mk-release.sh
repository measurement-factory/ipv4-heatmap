#!/bin/sh
set -e

MANIFEST="
LICENSE
Makefile
annotate.c
bbox.c
bbox.h
hilbert.c
morton.c
hsv2rgb.h
ipv4-heatmap.1
ipv4-heatmap.c
legend.c
shade.c
text.c
cidr.h
cidr.c
labels/iana/iana-labels.txt
labels/iana/ipv4-address-space
labels/iana/reserved
labels/iana/rfc1918.shades
demos/ipv4-classful/classful.shade
demos/ipv4-classful/Makefile
demos/ipv4-classful/classful.annotations
"

if svn diff | grep . ; then
	echo "bailing due to uncommitted changes"
	exit 1
fi

YYMMDD=`date +%Y%m%d`
mkdir ipv4-heatmap-$YYMMDD

ls $MANIFEST | cpio -pdv ipv4-heatmap-$YYMMDD
sed -i .bak -e "s/^#undef RELEASE_VER/#define RELEASE_VER \"$YYMMDD\"/" \
	ipv4-heatmap-$YYMMDD/ipv4-heatmap.c
rm ipv4-heatmap-$YYMMDD/ipv4-heatmap.c.bak
(cd ipv4-heatmap-$YYMMDD; chmod 644 $MANIFEST)
tar czf /tmp/ipv4-heatmap-$YYMMDD.tar.gz ipv4-heatmap-$YYMMDD
tar tzvf /tmp/ipv4-heatmap-$YYMMDD.tar.gz 
