#!/usr/bin/perl -w
#
# Perl script for generating plots for group means of all statistics
# (i.e., average session delay, average session throughput, mean transfer
# rate, and decodable frame rate) with 95% confidence intervals at once.
#
# (C) 2010 Kyeong Soo (Joseph) Kim
#

# for better catching the bugs
use strict;


# check argument count and print usage if needed
my $argcnt = $#ARGV + 1;
if ($argcnt < 1) {
    print "Usage: generateMeansPlotsForAll.pl common_base_name_of_data_files\n";
	exit 1;
}

# initialize variables
my $infile_base = $ARGV[0];
my $home = $ENV{'HOME'};
#my $shellScript = $home . "/inet-hnrl/examples/ngoa/ecr2/scripts/generateMeansPlot.sh";
my %stats = (
	"ftp_dly" => "Average Delay of FTP Sessions [sec]",
	"ftp_thr" => "Average Throughput of FTP Sessions [Byte/sec]",
	"ftp_trf" => "Mean Transfer Rate of FTP Sessions [Byte/sec]",
	"http_dly" => "Average Delay of HTTP Sessions [sec]",
	"http_thr" => "Average Throughput of HTTP Sessions [Byte/sec]",
	"http_trf" => "Mean Transfer Rate of HTTP Sessions [Byte/sec]",
	"video_dfr" => "Average Decodable Frame Rate of Streaming Videos (Q)",
);

# generate group means plots into a pdf file
my $outfile = $infile_base . ".pdf";
open(RPIPE, "| R --no-save --quiet")
	or die "Can't fork: $!";
print RPIPE <<EOF;
library(gplots);
pdf(file="$outfile", width=8.3, height=11.7, paper="a4");
layout(matrix(c(1, 2, 3, 4, 5, 6, 7, 7, 7), 3, 3, byrow=T));
EOF

foreach my $ext (sort keys %stats) {
	my $infile = $infile_base . ".$ext";
	print RPIPE <<EOF;
data <- read.table("$infile", col.names = c("numHosts", "repetition", "value"));
plotmeans(data\$value ~ data\$numHosts, xlab="Number of Hosts per ONU", ylab = "$stats{$ext}", n.label=FALSE, cex.lab=0.8, cex.axis=0.8, cex.sub=0.8);
EOF
}

print RPIPE <<EOF;
dev.off();
EOF

close RPIPE
	or warn $! ? "Error closing pipe with R: $!"
	: "Exit status $? from pipe with R";
