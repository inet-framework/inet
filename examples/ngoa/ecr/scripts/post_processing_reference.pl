#!/usr/bin/perl
#
# Perl script for post processing simulation data for ECR reference model
# in OMNeT++ scala files
#
# It prints out results to corresponding output files against access
# rate ($dr=$fr).
#
# (C) 2009 Kyeong Soo (Joseph) Kim

@infiles = <$ARGV[0]>; # a list of files matching a given pattern

# # DEBUG
# print @files;

# initialize array of output file names
my @outfiles = ();


# extracting data from scalar files and put them into new output files
foreach $infile (@infiles) {

	my @names = split("-", $infile);
	my $config = $names[0];		# configuration name without run number
	
# 	# DEBUG
# 	print "$config\n";
	
	open(INFILE, "<", $infile)
		or die "Can't open $infile for input: $!";

	# initilaize iteration variables to unused values
	my $N = -1;					# number of subscribers
	my $n = -1;					# number of sessions per subscriber
	my $dr = -1;				# rate of distribution fiber [Mbps]
	my $fr = -1;				# rate of feeder fiber [Mbps]
	my $br = -1;				# rate of backbone network [Tbps]
	my $bd = -1;				# delay in backbone network [ms]
	
	# initialize variables for statistics
	my $sum_delay = 0;
	my $num_delay = 0;
	my $sum_throughput = 0;
	my $num_throughput = 0;
	my $sum_transfer_rate = 0;
	my $num_transfer_rate = 0;
	
	while ($line = <INFILE>) {
		chomp $line;

		# process attributes for output file naming
		if ($line =~ /attr iterationvars2/) {
			my @arr = split(/\s+/, $line);
			foreach $pair (@arr) {
				$pair =~ tr/[,"]//d; # remove trailing commas and double quotes				
				if ($pair =~ /\$N/) {
					my @arr2 = split(/=/, $pair);
					$N = $arr2[1];
					
# 					# DEBUG
# 					print "N = $N\n";
				}
				elsif ($pair =~ /\$n/) {
					my @arr2 = split(/=/, $pair);
					$n = $arr2[1];
					
# 					# DEBUG
# 					print "n = $n\n";
				}
				elsif ($pair =~ /\$dr/) {
					my @arr2 = split(/=/, $pair);
					$dr = $arr2[1];
					
# 					# DEBUG
# 					print "dr = $dr\n";
				}
				elsif ($pair =~ /\$fr/) {
					my @arr2 = split(/=/, $pair);
					$fr = $arr2[1];
					
# 					# DEBUG
# 					print "fr = $fr\n";
				}
				elsif ($pair =~ /\$br/) {
					my @arr2 = split(/=/, $pair);
					$br = $arr2[1];
					
# 					# DEBUG
# 					print "br = $br\n";
				}
				elsif ($pair =~ /\$bd/) {
					my @arr2 = split(/=/, $pair);
					$bd = $arr2[1];
					
# 					# DEBUG
# 					print "bd = $bd\n";
				}									
			}
		}						# end of attribute processing
		
		# get average of "average session delay" over clients
		if ($line =~ /average session delay/) {
			my @arr = split(/\t/, $line);
# 			print "$arr[2]\n";
			$sum_delay += $arr[2];
			$num_delay++;
		}

		# get average of "average session throughput" over clients
		if ($line =~ /average session throughput/) {
			my @arr = split(/\t/, $line);
#			print "$arr[2]\n";
			$sum_throughput += $arr[2];
			$num_throughput++;
		}

		# get average of "mean session transfer rate" over clients
		if ($line =~ /mean session transfer rate/) {
			my @arr = split(/\t/, $line);
#			print "$arr[2]\n";
			$sum_transfer_rate += $arr[2];
			$num_transfer_rate++;
		}
	}	# end of input file processing

	# close input file
	close INFILE
		or warn $! ? "Error closing $infile: $!"
		: "Exit status $? from $infile";
	
	# get a base name of the output file based on iteration variables
	my $outfile = $config;
	if ($N >= 1) {
		$outfile = $outfile . "_N" . $N;
	}
 	if ($n >= 1) {
 		$outfile = $outfile . "_n" . $n;
	}
# 	if ($dr >= 0) {
# 		$outfile = $outfile . "_dr" . $dr;
# 	}
# 	if ($fr >= 0) {
# 		$outfile = $outfile . "_fr" . $fr;
# 	}
	if ($br >= 0) {
		$outfile = $outfile . "_br" . $br;
	}
	if ($bd >= 0) {
		$outfile = $outfile . "_bd" . $bd;
	}
	
# 	# DEBUG
# 	print "outfile = $outfile\n";

	### print results to a corresponding output file against $dr$

	# average session delay
	my $outfile_dly = $outfile . ".dly";
	open(OUTFILE, ">>", $outfile_dly)
		or die "Can't open $outfile_dly for output: $!";
	printf(OUTFILE "%d\t%le\n", $dr, $sum_delay/$num_delay);
	close OUTFILE
		or warn $! ? "Error closing $outfile_dly: $!"
		: "Exit status $? from $outfile_dly";
	unless ($outfile_dly ~~ @outfiles) {
		push(@outfiles, $outfile_dly); # store the file name for later processing
	}

	# average session throughput
	my $outfile_thr = $outfile . ".thr";
	open(OUTFILE, ">>", $outfile_thr)
		or die "Can't open $outfile_thr for output: $!";
	printf(OUTFILE "%d\t%le\n", $dr, $sum_throughput/$num_throughput);
	close OUTFILE
		or warn $! ? "Error closing $outfile_thr: $!"
		: "Exit status $? from $outfile_thr";
	unless ($outfile_thr ~~ @outfiles) {
		push(@outfiles, $outfile_thr); # store the file name for later processing
	}

	# mean session transfer rate
	my $outfile_trf = $outfile . ".trf";
	open(OUTFILE, ">>", $outfile_trf)
		or die "Can't open $outfile_trf for output: $!";
	printf(OUTFILE "%d\t%le\n", $dr, $sum_transfer_rate/$num_transfer_rate);
	close OUTFILE
		or warn $! ? "Error closing $outfile_trf: $!"
		: "Exit status $? from $outfile_trf";
	unless ($outfile_trf ~~ @outfiles) {
		push(@outfiles, $outfile_trf); # store the file name for later processing
	}

}	# end of 1st for loop


# sorting output files based on the value of the 1st column
foreach $outfile (@outfiles) {

#	# DEBUG
#	print "outfile = $outfile\n";

	my $rc = system("sort -o $outfile -n $outfile");
	die "system() failed with status $rc" unless $rc == 0;

}	# end of 2nd for loop
