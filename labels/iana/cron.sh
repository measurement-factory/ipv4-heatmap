#!/bin/sh
set -e
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin
export PATH
cd `dirname $0`

fetch -q http://www.iana.org/assignments/ipv4-address-space
perl grok.pl < ipv4-address-space > iana-labels.txt
yymmdd=`date +%Y-%m-%d`
svn -q -m "version $yymmdd" commit ipv4-address-space iana-labels.txt
