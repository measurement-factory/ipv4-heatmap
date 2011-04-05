#!/usr/bin/perl

# input is http://www.iana.org/assignments/ipv4-address-space
# output is the same info, but in a better format

use strict;
use warnings;
use Net::CIDR::Lite;
use XML::Simple;
use Data::Dumper;

my $XML = XMLin("-");

# if $when is specified, then blocks assigned after $when
# will be changed to "IANA - Reserved".  Format of $when
# is YYYYMMDD -- ugh.
#
my $when = shift;

my $cidr = Net::CIDR::Lite->new;
my $last_desc = undef;

sub clean_desc {
	my $desc = shift;
	my $sdate = shift;
	my $block = shift;
	my $status = shift;

	# Clean up descriptions

	if ('Multicast' eq $desc) {
		1;
	} elsif ('RESERVED' eq $status) {
		$desc = 'Reserved';
	} elsif ('UNALLOCATED' eq $status) {
		$desc = 'Unallocated';
	}

	$desc =~ s/\s+$//g;
	$desc =~ s/\s+\(.*\)$//;
	$desc =~ s/\s+See \[.*$//;
	$desc =~ s/ARIN - Cable Block/Cable/;
	$desc =~ s/IANA - Private Use/RFC1918/;
	$desc =~ s/IANA - (.*)/$1/;
	$desc =~ s/RIPE NCC/RIPE/;
	$desc =~ s/Level 3 Communications, Inc\./Level3/;
	$desc =~ s/General Electric Company/GE/;
	$desc =~ s/Army Information Systems Center/US Army/;
	$desc =~ s/IANA - Private Use/RFC1918/;
	$desc =~ s/AT\&T Bell Laboratories/AT\&T/;
	$desc =~ s/Xerox Corporation/Xerox/;
	$desc =~ s/Hewlett-Packard Company/HP/;
	$desc =~ s/Digital Equipment Corporation/DEC/;
	$desc =~ s/Apple Computer Inc\./Apple/;
	$desc =~ s/Defense Information Systems Agency/DISA/;
	$desc =~ s/AT\&T Global Network Services/AT\&T/;
	$desc =~ s/Halliburton Company/Halliburton/;
	$desc =~ s/MERIT Computer Network/Merit/;
	$desc =~ s/Performance Systems International/PSI/;
	$desc =~ s/Eli Lily \& Company/Eli Lily/;
	$desc =~ s/Interop Show Network/Interop/;
	$desc =~ s/Prudential Securities Inc\./Prudential/;
	$desc =~ s/DoD Network Information Center/DoD NIC/;
	$desc =~ s/US Postal Service/USPS/;

	#$desc = "Legacy ($1)" if ($desc =~ /^Administered by (\w+)$/);
	$desc = "Various Registries" if ($desc =~ /^Administered by (\w+)$/ && $status =~ /LEGACY/);

	$desc = 'Reserved' if ($when && $sdate > $when);

#	# try to differentiate "Reserved" from "Unallocated" blocks
#	# more ugh
#	if ($desc eq 'Reserved') {
#		if ($block == 0) {
#			1;
#		} elsif ($block == 14) {
#			1;
#		} elsif ($block == 127) {
#			1;
#		} elsif ($block >= 240) {
#			1;
#		} else {
#			$desc = 'Unallocated';
#		}
#	}
	$desc;
}

#$VAR1 = {
#          'xref' => {
#                    'data' => '2',
#                    'type' => 'note'
#                  },
#          'designation' => 'IANA - Local Identification',
#          'status' => 'RESERVED',
#          'date' => '1981-09',
#          'prefix' => '000/8'
#        };


foreach my $rec (@{$XML->{record}}) {
#print STDERR Dumper($rec);
	$rec->{date} = "1970-01" unless defined $rec->{date};
	my $block = $1 if $rec->{prefix} =~ /(\d+)\/8/;
	my $month = $1 if $rec->{date}	 =~ /\d+-(\d+)/;
	my $year = $1 if $rec->{date}	 =~ /(\d+)-\d+/;
	my $desc = $rec->{designation};
	my $status = $rec->{status};

#print STDERR "$block $month $year $desc\n";

	my $sdate = $year * 10000 + $month * 100;
	$desc = clean_desc($desc, $sdate, $block, $status);

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
