#!/usr/bin/perl

my %M = qw (Jan 1 Feb 2 Mar 3 Apr 4 May 5 Jun 6 Jul 7 Aug 8 Sep 8 Oct 10 Nov 11 Dec 12);

# input is http://www.iana.org/ipaddress/ip-addresses.htm
# output is the same info, but in a better format

use strict;
use warnings;
use Net::CIDR::Lite;

# if $when is specified, then blocks assigned after $when
# will be changed to "IANA - Reserved"
#
my $when = shift;

my $cidr = Net::CIDR::Lite->new;
my $last_desc = undef;

while (<>) {
        chomp;
	next unless (/^(\d\d\d)\/8\s+(\w\w\w) (\d\d)\s+(.*)/);
	my $block = $1;
	my $month = $2;
	my $year = $3;
	my $desc = $4;

	$year += $year < 70 ? 2000 : 1900;

	$desc =~ s/\s+$//g;
	$desc =~ s/\s+\(.*\)$//;
	$desc =~ s/\s+See \[.*$//;
	$desc =~ s/IANA - Multicast/Multicast/;

	my $prefix = sprintf "%d.0.0.0/8", $block;
	if ($last_desc && $desc ne $last_desc) {
		foreach $prefix ($cidr->list) {
			printf "%s\t%s\n", $prefix, $last_desc;
		}
		$cidr = undef;
		$cidr = Net::CIDR::Lite->new;
		$last_desc = undef;
	}
	$cidr->add($prefix);
	$last_desc = $desc;
}
		foreach my $prefix ($cidr->list) {
			printf "%s\t%s\n", $prefix, $last_desc;
		}
