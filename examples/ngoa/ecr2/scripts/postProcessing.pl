#!/usr/bin/perl -w
#
# Perl script for post processing of OMNeT++ scala files resulting
# from the simulation of candidate NGOA architectures
#
# It stores results in separate output files for delay, throughput,
# and transfer rates against the number of hosts per ONU ($n) and
# repetition.
#
# (C) 2009-2010 Kyeong Soo (Joseph) Kim
#


# for better catching the bugs
use strict;


# check argument count and print usage if needed
my $argcnt = $#ARGV + 1;
if ($argcnt < 1) {
	die "Usage: $0 \"regexp_pattern_for_scala_files\"\n";
}

# initialize variables
my @infiles = <$ARGV[0]>; # a list of files matching a given pattern
my %outfiles;
my $numInFilesProcessed = 0;	# counter for the number of input files processed

# extract & process statistics from the given scalar files
# and put them into separate output files
foreach my $infile (@infiles) {

	my @names = split("-", $infile);
	my $config = $names[0];		# configuration name without run number
	
#########################################################################
# input file processing
#########################################################################
	
	open(INFILE, "<", $infile)
		or die "Can't open $infile for input: $!";

	# initilaize iteration variables to unused values
	my $N = -1;					# number of ONUs (subscribers)
	my $n = -1;					# number of hosts (users) per ONU (subscriber)
	my $dr = -1;				# rate of distribution fiber [Gbps]
	my $fr = -1;				# rate of feeder fiber [Gbps] (for PONs)
	my $tx = -1;				# number of OLT transmitters
	my $br = -1;				# rate of backbone link [Gbps]
	my $bd = -1;				# delay in backbone network [ms]
	my $repetition = -1;		# repetition (with a different seed for RNGs)
	
	# initialize variables for statistics

	# my $sum_delay = 0;
	# my $num_delay = 0;
	# my $sum_throughput = 0;
	# my $num_throughput = 0;
	# my $sum_transfer_rate = 0;
	# my $num_transfer_rate = 0;

	# for FTP
	my @ftp_num_sessions = ();
	my @ftp_delay = ();
	my @ftp_throughput = ();
	my @ftp_transfer_rate = ();

	# for HTTP
	my @http_num_sessions = ();
	my @http_delay = ();
	my @http_throughput = ();
	my @http_transfer_rate = ();

	# for UDP video streaming
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
			$sum_decodable_frame_rate += $arr[2];
			$num_decodable_frame_rate++;
		}
	}	# end of while ()

	# close input file
	close INFILE
		or warn $! ? "Error closing $infile: $!"
		: "Exit status $? from $infile";

#########################################################################
# output file processing
#########################################################################
	
	# get a base name of the output file based on iteration variables
#	my $outfile = $config;
	my $outfile = "";
	if ($N >= 0) {
		if ($outfile) {	# $outfile in not empty
			$outfile = $outfile . "_N" . $N;
		}
		else {
			$outfile = "N" . $N;
		}
	}
# 	if ($n >= 1) {
# 		$outfile = $outfile . "_n" . $n;
#	}
	if ($dr >= 0) {
		if ($outfile) {
			$outfile = $outfile . "_dr" . $dr;
		}
		else {
			$outfile = "dr" . $dr;
		}
	}
	if ($fr >= 0) {
		if ($outfile) {
			$outfile = $outfile . "_fr" . $fr;
		}
		else {		
			$outfile = "fr" . $fr;
		}
	}
	if ($tx >= 0) {
		if ($outfile) {
			$outfile = $outfile . "_tx" . $tx;
		}
		else {		
			$outfile = "tx" . $tx;
		}
	}
	if ($br >= 0) {
		if ($outfile) {
			$outfile = $outfile . "_br" . $br;
		}
		else {	
			$outfile = "br" . $br;
		}
	}
	# replace bd (backbond delay) with rtt (overall round-trip delay including 0.3ms delay in access)
	if ($bd >= 0) {
		my $rtt = ($bd+0.3)*2;
		if ($outfile) {
			$outfile = $outfile . "_rtt" . $rtt;
		}
		else {
			$outfile = "rtt" . $rtt;
		}
	}
	
	### print results to a corresponding output file against $n$ and $repetition$

	### for FTP
	
	# calcuate the number of finished sessions for delay, throughput, and transfer rate
	my $ftp_sum_num_sessions = 0;
	for my $i (0 .. $#ftp_num_sessions) {
		$ftp_sum_num_sessions += $ftp_num_sessions[$i];
	}

	# average session delay
	my $ftp_sum_delay = 0;
	unless ($#ftp_delay == $#ftp_num_sessions) {
		die "Error in processing FTP average session delay";
	}
	for my $i (0 .. $#ftp_delay) {
		$ftp_sum_delay += $ftp_delay[$i]*$ftp_num_sessions[$i];
	}
	my $ftp_avg_delay = $ftp_sum_delay/$ftp_sum_num_sessions;

	my $outfile_ftp_dly = $outfile . ".ftp_dly";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_ftp_dly)
			or die "Can't open $outfile_ftp_dly for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_ftp_dly)
			or die "Can't open $outfile_ftp_dly for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $ftp_avg_delay);
	close OUTFILE
		or warn $! ? "Error closing $outfile_ftp_dly: $!"
		: "Exit status $? from $outfile_ftp_dly";
	unless (exists $outfiles{$outfile_ftp_dly}) {
		$outfiles{$outfile_ftp_dly} = 1; # store the file name for later processing
	}

	# average session throughput
	my $ftp_sum_throughput = 0;
	unless ($#ftp_throughput == $#ftp_num_sessions) {
		die "Error in processing FTP average session throughput";
	}
	for my $i (0 .. $#ftp_throughput) {
		$ftp_sum_throughput += $ftp_throughput[$i]*$ftp_num_sessions[$i];
	}
	my $ftp_avg_throughput = $ftp_sum_throughput/$ftp_sum_num_sessions;

	my $outfile_ftp_thr = $outfile . ".ftp_thr";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_ftp_thr)
			or die "Can't open $outfile_ftp_thr for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_ftp_thr)
			or die "Can't open $outfile_ftp_thr for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $ftp_avg_throughput);
	close OUTFILE
		or warn $! ? "Error closing $outfile_ftp_thr: $!"
		: "Exit status $? from $outfile_ftp_thr";
	unless (exists $outfiles{$outfile_ftp_thr}) {
		$outfiles{$outfile_ftp_thr} = 1; # store the file name for later processing
	}

	# mean session transfer rate
	my $ftp_sum_transfer_rate = 0;
	unless ($#ftp_transfer_rate == $#ftp_num_sessions) {
		die "Error in processing FTP mean session transfer rate";
	}
	for my $i (0 .. $#ftp_transfer_rate) {
		$ftp_sum_transfer_rate += $ftp_transfer_rate[$i]*$ftp_num_sessions[$i];
	}
	my $ftp_avg_transfer_rate = $ftp_sum_transfer_rate/$ftp_sum_num_sessions;

	my $outfile_ftp_trf = $outfile . ".ftp_trf";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_ftp_trf)
			or die "Can't open $outfile_ftp_trf for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_ftp_trf)
			or die "Can't open $outfile_ftp_trf for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $ftp_avg_transfer_rate);
	close OUTFILE
		or warn $! ? "Error closing $outfile_ftp_trf: $!"
		: "Exit status $? from $outfile_ftp_trf";
	unless (exists $outfiles{$outfile_ftp_trf}) {
		$outfiles{$outfile_ftp_trf} = 1; # store the file name for later processing
	}

	### for HTTP

	# calcuate the number of finished sessions for delay, throughput, and transfer rate
	my $http_sum_num_sessions = 0;
	for my $i (0 .. $#http_num_sessions) {
		$http_sum_num_sessions += $http_num_sessions[$i];
	}

	# average session delay
	my $http_sum_delay = 0;
	unless ($#http_delay == $#http_num_sessions) {
		die "Error in processing HTTP average session delay";
	}
	for my $i (0 .. $#http_delay) {
		$http_sum_delay += $http_delay[$i]*$http_num_sessions[$i];
	}
	my $http_avg_delay = $http_sum_delay/$http_sum_num_sessions;

	my $outfile_http_dly = $outfile . ".http_dly";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_http_dly)
			or die "Can't open $outfile_http_dly for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_http_dly)
			or die "Can't open $outfile_http_dly for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $http_avg_delay);
	close OUTFILE
		or warn $! ? "Error closing $outfile_http_dly: $!"
		: "Exit status $? from $outfile_http_dly";
	unless (exists $outfiles{$outfile_http_dly}) {
		$outfiles{$outfile_http_dly} = 1; # store the file name for later processing
	}

	# average session throughput
	my $http_sum_throughput = 0;
	unless ($#http_throughput == $#http_num_sessions) {
		die "Error in processing HTTP average session throughput";
	}
	for my $i (0 .. $#http_throughput) {
		$http_sum_throughput += $http_throughput[$i]*$http_num_sessions[$i];
	}
	my $http_avg_throughput = $http_sum_throughput/$http_sum_num_sessions;

	my $outfile_http_thr = $outfile . ".http_thr";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_http_thr)
			or die "Can't open $outfile_http_thr for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_http_thr)
			or die "Can't open $outfile_http_thr for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $http_avg_throughput);
	close OUTFILE
		or warn $! ? "Error closing $outfile_http_thr: $!"
		: "Exit status $? from $outfile_http_thr";
	unless (exists $outfiles{$outfile_http_thr}) {
		$outfiles{$outfile_http_thr} = 1; # store the file name for later processing
	}

	# mean session transfer rate
	my $http_sum_transfer_rate = 0;
	unless ($#http_transfer_rate == $#http_num_sessions) {
		die "Error in processing HTTP mean session rate";
	}
	for my $i (0 .. $#http_transfer_rate) {
		$http_sum_transfer_rate += $http_transfer_rate[$i]*$http_num_sessions[$i];
	}
	my $http_avg_transfer_rate = $http_sum_transfer_rate/$http_sum_num_sessions;

	my $outfile_http_trf = $outfile . ".http_trf";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_http_trf)
			or die "Can't open $outfile_http_trf for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_http_trf)
			or die "Can't open $outfile_http_trf for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $http_avg_transfer_rate);
	close OUTFILE
		or warn $! ? "Error closing $outfile_http_trf: $!"
		: "Exit status $? from $outfile_http_trf";
	unless (exists $outfiles{$outfile_http_trf}) {
		$outfiles{$outfile_http_trf} = 1; # store the file name for later processing
	}

	### for UDP video streaming

	# decodable frame rate (Q)
	my $outfile_video_dfr = $outfile . ".video_dfr";
	if ($numInFilesProcessed == 0) {
		open(OUTFILE, ">", $outfile_video_dfr)
			or die "Can't open $outfile_video_dfr for output: $!";
	}
	else {
		open(OUTFILE, ">>", $outfile_video_dfr)
			or die "Can't open $outfile_video_dfr for output: $!";
	}
	printf(OUTFILE "%d\t%d\t%le\n", $n, $repetition, $sum_decodable_frame_rate/$num_decodable_frame_rate);
	close OUTFILE
		or warn $! ? "Error closing $outfile_video_dfr: $!"
		: "Exit status $? from $outfile_video_dfr";
	unless (exists $outfiles{$outfile_video_dfr}) {
		$outfiles{$outfile_video_dfr} = 1; # store the file name for later processing
	}

	# update counter
	$numInFilesProcessed++;

}	# end of the 1st foreach()


# sorting output files based on the value of the 1st column
foreach my $outfile (keys %outfiles) {
	my $rc = system("sort -o $outfile -n $outfile");
	die "system() failed with status $rc" unless ($rc == 0);
}	# end of the 2nd foreach()
