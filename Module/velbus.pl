#!/usr/bin/env perl

use strict;
use warnings;
use POSIX qw(floor strftime);

use Data::Dumper;

sub enum { # {{{
	my ($key, %hash) = @_;
	return $hash{$key} if defined $hash{$key};
	return sprintf "Unknown[0x%02x]", $key;
} # }}}

sub twos_complement { # {{{
	my (@byte) = @_;
	my $ret = 0;
	if( $byte[0] & 0x80 ) {
		# negative
		while( @byte ) {
			$ret <<= 8;
			$ret |= (shift @byte) ^ 0xff;
		}
		$ret = -($ret + 1);
	} else {
		# positive
		while( @byte ) {
			$ret <<= 8;
			$ret |= shift @byte;
		}
	}
	return $ret;
} # }}}

my @parser;
my %cmd;

push @parser, sub {
	my ($prio, $addr, $rtr, $data) = @_;
	if( @{$data} >= 1 ) {
		my $cmd = $data->[0];
		if( defined $cmd{$cmd} ) {
			return $cmd{$cmd}->($prio, $addr, $rtr, $data);
		}
	}
	return undef;
};

$cmd{0xfa} = sub {
	my ($prio, $addr, $rtr, $data) = @_;
	return undef unless $rtr  == 0;
	return undef unless @{$data} == 2;

	my $what = enum( $data->[1],
			0x00 => "generic",
			0x03 => "Blind=1",
			0x0c => "Blind=2",
			0x01 => "Relay=1",
			0x02 => "Relay=2",
			0x04 => "Relay=3",
			0x08 => "Relay=4",
			0x10 => "Relay=5",
		);

	return sprintf("Prio=%d ModuleStatusRequest to 0x%02x: %s", $prio, $addr, $what);
};

$| = 1; # autoflush STDOUT 
while(<>) {
	next unless m/^(\d+).(\d+)( \S+ Rx|Tx)(( [0-9a-fA-F]{2})+)$/;
	my ($timestamp_int, $timestamp_frac, $prefix, $hexdump) = ($1, $2, $3, $4);
	my @pdu = map { hex $_ } split / /, substr($hexdump, 1);

	next unless $pdu[0] == 0x0f; # STX
	next unless ($pdu[1] & 0xfc) == 0xf8;
	my $prio = $pdu[1] & 0x03;
	my $addr = $pdu[2];
	next unless ($pdu[3] & 0xb0) == 0x00;
	my $rtr = ($pdu[3] & 0x40) >> 6;
	my $len = ($pdu[3] & 0x0f);
	my @data = @pdu[ 4..(4+$len-1) ];
	my $crc = $pdu[ 4+$len ];
	next unless $pdu[ 4+$len+1 ] == 0x04; # ETX

	my $our_crc = 0;
	map { $our_crc += $_ } @pdu[ 0..(4+$len-1) ];
	$our_crc = (-$our_crc) & 0xff;
	next unless $crc == $our_crc;

    	my @timestamp_int = localtime($timestamp_int);
	my $timestamp = strftime("%Y-%m-%d %H:%M:%S", @timestamp_int) .
		"." . $timestamp_frac .
		strftime(" %z", @timestamp_int); 

	$_ = sprintf "%s%s Prio=%d Addr=0x%02x RTR=%d %s\n",
		$timestamp, $prefix, $prio, $addr, $rtr,
		join(' ', map { sprintf "%02x", $_ } @data);

	for my $p (@parser) {
		my $temp = &{$p}($prio, $addr, $rtr, \@data);
		if( defined $temp ) {
			$_ = "${timestamp}${prefix} $temp\n";
			last; # `for` iteration
		}
	}

} continue {
	print $_;
}

# vim: set foldmethod=marker:
