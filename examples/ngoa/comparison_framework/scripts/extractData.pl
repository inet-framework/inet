#!/usr/bin/perl -w
#
# Perl script for extracting interesting statistics from OMNeT++ scala
# files resulting from the simulation of candidate NGOA architectures
#
# It stores extracted statistics in a CSV-format file for further
# processing by GNU R.
#
# (C) 2010 Kyeong Soo (Joseph) Kim
#


# for better catching the bugs
use strict;

# for checking numeric variables
use Scalar::Util qw(looks_like_number);


# check argument count and print usage if needed
my $argcnt = $#ARGV + 1;
if ($argcnt < 1) {
	die "Usage: $0 scalar_file_name (or \"regexp_pattern\") ... \n";
}

# initialize variables
my @infiles = ();	# a list of files matching a given pattern
foreach my $i (0 .. $#ARGV) {
	push(@infiles, <$ARGV[$i]>);
}
my @results = ();	# array of processed statistics
my $config = "";	# scalar file basename minus run number
# - iteration variables
my $N = -1;					# number of ONUs (subscribers)
my $n = -1;					# number of hosts (users) per ONU (subscriber)
my $dr = -1;				# rate of distribution fiber [Gbps]
my $fr = -1;				# rate of feeder fiber [Gbps] (for PONs)
my $tx = -1;				# number of OLT transmitters
my $br = -1;				# rate of backbone link [Gbps]
my $bd = -1;				# delay in backbone network [ms]
my $repetition = -1;		# repetition (with a different seed for RNGs)

# extract & process statistics from the given scalar files
# and put them into separate output files
foreach my $infile (@infiles) {

	unless ($config) {
		my @names = split("-", $infile);
		$config = $names[0];		# configuration name without run number
	}
	
#########################################################################
### input file processing
#########################################################################
	
	open(INFILE, "<", $infile)
		or die "Can't open $infile for input: $!";
	
	# initialize variables for statistics
	# - FTP
	my @ftp_num_sessions = ();
	my @ftp_delay = ();
	my @ftp_throughput = ();
	my @ftp_transfer_rate = ();
	# - HTTP
	my @http_num_sessions = ();
	my @http_delay = ();
	my @http_throughput = ();
	my @http_transfer_rate = ();
	# - UDP video streaming
	my $sum_decodable_frame_rate = 0;
	my $num_decodable_frame_rate = 0;
	
	while (my $line = <INFILE>) {
		chomp $line;

		# process attributes for output file naming
		if ($line =~ /attr iterationvars2/) {
			my @arr = split(/\s+/, $line);
			foreach my $pair (@arr) {
				$pair =~ tr/[,"]//d; # remove trailing commas and double quotes				
				if ($pair =~ /\$N/) {
					my @arr2 = split(/=/, $pair);
					$N = $arr2[1];
				}
				elsif ($pair =~ /\$n/) {
					my @arr2 = split(/=/, $pair);
					$n = $arr2[1];
				}
				elsif ($pair =~ /\$dr/) {
					my @arr2 = split(/=/, $pair);
					$dr = $arr2[1];
				}
				elsif ($pair =~ /\$fr/) {
					my @arr2 = split(/=/, $pair);
					$fr = $arr2[1];
				}
				elsif ($pair =~ /\$tx/) {
					my @arr2 = split(/=/, $pair);
					$tx = $arr2[1];
				}
				elsif ($pair =~ /\$br/) {
					my @arr2 = split(/=/, $pair);
					$br = $arr2[1];
				}
				elsif ($pair =~ /\$bd/) {
					my @arr2 = split(/=/, $pair);
					$bd = $arr2[1];
				}
				elsif ($pair =~ /\$repetition/) {
					my @arr2 = split(/=/, $pair);
					$repetition = $arr2[1];
				}
			}	# end of foreach()
		}	# end of attribute processing
		
		# get "number of finished sessions"
		if ($line =~ /number of finished sessions/) {
			my @arr = split(/\t/, $line);
			if ($line =~ /ftpApp/) {
				push(@ftp_num_sessions, $arr[2]);
			} elsif ($line =~ /httpApp/) {
				push(@http_num_sessions, $arr[2]);
			}
		}

		# get "average session delay"
		if ($line =~ /average session delay/) {
			my @arr = split(/\t/, $line);
			if ($line =~ /ftpApp/) {
				push(@ftp_delay, $arr[2]);
			} elsif ($line =~ /httpApp/) {
				push(@http_delay, $arr[2]);
			}
		}

		# get "average session throughput"
		if ($line =~ /average session throughput/) {
			my @arr = split(/\t/, $line);
			if ($line =~ /ftpApp/) {
				push(@ftp_throughput, $arr[2]);
			} elsif ($line =~ /httpApp/) {
				push(@http_throughput, $arr[2]);
			}
		}

		# get "mean session transfer rate"
		if ($line =~ /mean session transfer rate/) {
			my @arr = split(/\t/, $line);
			if ($line =~ /ftpApp/) {
				push(@ftp_transfer_rate, $arr[2]);
			} elsif ($line =~ /httpApp/) {
				push(@http_transfer_rate, $arr[2]);
			}
		}

		# get "decodable frame rate (Q)"
		if ($line =~ /decodable frame rate/) {
			my @arr = split(/\t/, $line);
			if ($arr[2] ne "nan") {
				$sum_decodable_frame_rate += $arr[2];
				$num_decodable_frame_rate++;
			}
		}
	}	# end of while ()

	# close input file
	close INFILE
		or warn $! ? "Error closing $infile: $!"
		: "Exit status $? from $infile";

#########################################################################	
### store extracted & processed statistics in the results array
#########################################################################

	### FTP
	my ($ftp_avg_delay, $ftp_avg_throughput, $ftp_avg_transfer_rate) = ("", "", "");
	# check whether the number of finished sessions is bigger than zero
	if ($#ftp_num_sessions >= 0) {
		# check whether the sizes of arrays are equal
		unless (
			($#ftp_delay == $#ftp_num_sessions) &&
			($#ftp_throughput == $#ftp_num_sessions) &&
			($#ftp_transfer_rate == $#ftp_num_sessions)
			) {
			die "Error in processing FTP statistics";
		}
		# calcuate the number of finished sessions
		my $ftp_sum_num_sessions = 0;
		for my $i (0 .. $#ftp_num_sessions) {
			$ftp_sum_num_sessions += $ftp_num_sessions[$i];
		}
		# averaging session delay, throughput, and transfer rate
		my $ftp_sum_delay = 0;
		my $ftp_sum_throughput = 0;
		my $ftp_sum_transfer_rate = 0;
		for my $i (0 .. $#ftp_num_sessions) {
			$ftp_sum_delay += $ftp_delay[$i]*$ftp_num_sessions[$i];
			$ftp_sum_throughput += $ftp_throughput[$i]*$ftp_num_sessions[$i];
			$ftp_sum_transfer_rate += $ftp_transfer_rate[$i]*$ftp_num_sessions[$i];
		}
		$ftp_avg_delay = $ftp_sum_delay/$ftp_sum_num_sessions;
		$ftp_avg_throughput = $ftp_sum_throughput/$ftp_sum_num_sessions;
		$ftp_avg_transfer_rate = $ftp_sum_transfer_rate/$ftp_sum_num_sessions;
	}

	### HTTP
	my ($http_avg_delay, $http_avg_throughput, $http_avg_transfer_rate) = ("", "", "");
	# check whether the number of finished sessions is bigger than zero
	if ($#http_num_sessions >= 0) {
		# check whether the sizes of arrays are equal
		unless (
			($#http_delay == $#http_num_sessions) &&
			($#http_throughput == $#http_num_sessions) &&
			($#http_transfer_rate == $#http_num_sessions)
			) {
			die "Error in processing HTTP statistics";
		}
		# calcuate the number of finished sessions
		my $http_sum_num_sessions = 0;
		for my $i (0 .. $#http_num_sessions) {
			$http_sum_num_sessions += $http_num_sessions[$i];
		}
		# averaging session delay, throughput, and transfer rate
		my $http_sum_delay = 0;
		my $http_sum_throughput = 0;
		my $http_sum_transfer_rate = 0;
		for my $i (0 .. $#http_num_sessions) {
			$http_sum_delay += $http_delay[$i]*$http_num_sessions[$i];
			$http_sum_throughput += $http_throughput[$i]*$http_num_sessions[$i];
			$http_sum_transfer_rate += $http_transfer_rate[$i]*$http_num_sessions[$i];
		}
		$http_avg_delay = $http_sum_delay/$http_sum_num_sessions;
		$http_avg_throughput = $http_sum_throughput/$http_sum_num_sessions;
		$http_avg_transfer_rate = $http_sum_transfer_rate/$http_sum_num_sessions;
	}

	### UDP video streaming
	# averaging decodable frame rate (Q)
	my $avg_decodable_frame_rate = "";
	if ($num_decodable_frame_rate > 0) {
		$avg_decodable_frame_rate = $sum_decodable_frame_rate/$num_decodable_frame_rate;
	}

	### store statistics into the results array
	# get interation variables used and convert them into a comma-separated string
	my $itervars = "";
	$itervars .= ($N >= 0) ? "$N," : "";
	$itervars .= ($n >= 0) ? "$n," : "";
	$itervars .= ($dr >= 0) ? "$dr," : "";
	$itervars .= ($fr >= 0) ? "$fr," : "";
	$itervars .= ($tx >= 0) ? "$tx," : "";
	$itervars .= ($br >= 0) ? "$br," : "";
	if ($bd >= 0) {	# replace bd (backbond delay) with rtt
		my $rtt = ($bd+0.3)*2;
		$itervars .= "$rtt,";
	}
	$itervars .= ($repetition >= 0) ? "$repetition," : "";

	# store the processed statistics into a string
	my $stats = "";
	$stats .= looks_like_number($ftp_avg_delay) ? sprintf("%le,", $ftp_avg_delay) : "NA,";
	$stats .= looks_like_number($ftp_avg_throughput) ? sprintf("%le,", $ftp_avg_throughput) : "NA,";
	$stats .= looks_like_number($ftp_avg_transfer_rate) ? sprintf("%le,", $ftp_avg_transfer_rate) : "NA,";
	$stats .= looks_like_number($http_avg_delay) ? sprintf("%le,", $http_avg_delay) : "NA,";
	$stats .= looks_like_number($http_avg_throughput) ? sprintf("%le,", $http_avg_throughput) : "NA,";
	$stats .= looks_like_number($http_avg_transfer_rate) ? sprintf("%le,", $http_avg_transfer_rate) : "NA,";
	$stats .= looks_like_number($avg_decodable_frame_rate) ? sprintf("%le", $avg_decodable_frame_rate) : "NA";	# no comma at the end

	# store the string in the array
	my $result = $itervars . $stats;
	push(@results, $result);

}	# end of input file processing

#########################################################################
### output file processing
#########################################################################
	
# get a base name from the configuration
my $outfile = $config . ".data";

# get interation variables used and convert them into column names for data
my $colnames = "";
$colnames .= ($N >= 0) ? "N," : "";
$colnames .= ($n >= 0) ? "n," : "";
$colnames .= ($dr >= 0) ? "dr," : "";
$colnames .= ($fr >= 0) ? "fr," : "";
$colnames .= ($tx >= 0) ? "tx," : "";
$colnames .= ($br >= 0) ? "br," : "";
$colnames .= ($bd >= 0) ? "rtt," : "";
$colnames .= ($repetition >= 0) ? "repetition," : "";

# append statistic names to column names
$colnames .= "ftp.delay,ftp.throughput,ftp.transferrate,";
$colnames .= "http.delay,http.throughput,http.transferrate,";
$colnames .= "video.dfr";

open(OUTFILE, ">", $outfile)
	or die "Can't open $outfile for output: $!";
printf(OUTFILE "%s\n", $colnames);
for my $i (0 .. $#results) {
	printf(OUTFILE "%s\n", $results[$i]);
}
close OUTFILE
	or warn $! ? "Error closing $outfile: $!"
	: "Exit status $? from $outfile";
