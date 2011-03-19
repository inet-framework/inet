#!/usr/bin/perl -w
#
# Perl script for generating plots for the number of messages present
# in the system during the simulation.
# 
# (C) 2010 Kyeong Soo (Joseph) Kim
#

# for better catching the bugs
use strict;


# check argument count and print usage if needed
my $argcnt = $#ARGV + 1;
if ($argcnt < 1) {
    print "Usage: $0 \"regexp_patterns_for_simulation_output_files\"\n";
	exit 1;
}

# initialize variables
my @infiles = <$ARGV[0]>; # a list of files matching a given pattern
my $numInFilesProcessed = 0;	# counter for the number of input files processed
my %pdfs = ();	# hash of run number and individual pdf file name
my $pdf = "";	# file name for merged pdfs

# set time stamp
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
my $timestamp = sprintf("%4d-%02d-%02d %02d:%02d:%02d", $year+1900, $mon+1, $mday, $hour, $min, $sec);

foreach my $infile (@infiles) {

	open(INFILE, "<", $infile)
		or die "Can't open $infile for input: $!";

	# initialize variables (per file)
	my $run = "";
	my $config = "";
	my @times = ();
	my @msgs = ();

	while (my $line = <INFILE>) {
		chomp $line;

		# get run number
		if ($line =~ /run #/) {
			$line =~ s/^\s+//;	# remove leading white space
			my @arr = split(/\s+/, $line);
			$run = $arr[6];
			$run =~ s/^#//;		# remove leading '#'
			$run =~ s/\.+//;	# remove trailing dots
		}

		# get configuration
		if ($line =~ /configuration/) {
			$line =~ s/^\s+//;	# remove leading white space
			my @arr = split(/\s+/, $line);
			chop($arr[4]);		# remove trailing comma
			$config = $arr[4];
		}

		# get simulated times
		if ($line =~ /T=/) {
			$line =~ s/^\s+//;	# remove leading white space
			my @arr1 = split(/\s+/, $line);
			my @arr2 = split(/=/, $arr1[3]);
			push(@times, $arr2[1]);
		}

		# get number of messages
		if ($line =~ /present:/) {
			$line =~ s/^\s+//;	# remove leading white space
			my @arr = split(/\s+/, $line);
			push(@msgs, $arr[4]);
		}
	}	# end of while ()

	# close input file
	close INFILE
		or warn $! ? "Error closing $infile: $!"
		: "Exit status $? from $infile";

	# check data integrity
	unless ($#times == $#msgs) {
		die "Error in processing $infile";
	}

	# get output file name based on configuration and run number
	my $outfile = $config . "-" . $run . ".msg";

	open(OUTFILE, ">", $outfile)
		or die "Can't open $outfile for output: $!";

	for my $i (0 .. $#times) {
		printf(OUTFILE "%le\t%d\n", $times[$i], $msgs[$i]);
	}

	# close output file
	close OUTFILE
		or warn $! ? "Error closing $outfile: $!"
		: "Exit status $? from $outfile";

	# generate a plot
	my $rScript = <<EOF;
data <- read.table("$outfile");
pdf(file="$outfile.pdf", width=10, height=10);
plot(data, main="$config-$run ($timestamp)", xlab="Time [sec]", ylab="Number of Messages", type="l", panel.first=grid(col="black", lwd=2));
dev.off();
EOF

	open(RPIPE, " | R --no-save --quiet")
		or die "can't fork: $!";
	print RPIPE $rScript;
	close RPIPE
		or warn $! ? "Error closing pipe with R: $!"
		: "Exit status $? from pipe with R";

	# delete output file
	unlink($outfile);

	# merge pdf file names
	if ($numInFilesProcessed == 0) {
		$pdf = $config . ".msg.pdf";
		$pdfs{$run} = "$outfile.pdf";
	}
	else {
		$pdfs{$run} = "$outfile.pdf";
	}

	# update counter
	$numInFilesProcessed++;

}	# end of foreach ()

# merge pdf files into one
my $joined_pdfs = "";
for my $key ( sort {$a <=> $b} keys %pdfs ) {
	$joined_pdfs .= "$pdfs{$key} ";
}
my $rc = system("pdftk $joined_pdfs cat output $pdf");
die "system() failed with status $rc" unless ($rc == 0);

# delete the original single pdf files
foreach my $key (keys %pdfs) {
	unlink($pdfs{$key});
}
