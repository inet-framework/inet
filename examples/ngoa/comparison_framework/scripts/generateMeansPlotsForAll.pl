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
my %exts = (
	"Average Delay of FTP Sessions [sec]" => "ftp_dly",
	"Average Throughput of FTP Sessions [Byte/sec]" => "ftp_thr",
	"Mean Transfer Rate of FTP Sessions [Byte/sec]" => "ftp_trf",
	"Average Delay of HTTP Sessions [sec]" => "http_dly",
	"Average Throughput of HTTP Sessions [Byte/sec]" => "http_thr",
	"Mean Transfer Rate of HTTP Sessions [Byte/sec]" => "http_trf",
	"Average Decodable Frame Rate of Streaming Videos (Q)" => "video_dfr",
);

# set time stamp
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
my $timestamp = sprintf("%4d-%02d-%02d %02d:%02d:%02d", $year+1900, $mon+1, $mday, $hour, $min, $sec);

# generate group means plots into pdf files
my $title = $infile_base . " \($timestamp\)";
open(RPIPE, "| R --no-save --quiet")
	or die "Can't fork: $!";
print RPIPE "library(gplots);\n";

foreach my $stat (keys %exts) {
	my $infile = $infile_base . ".$exts{$stat}";
# 	my $rc = system("$shellScript $infile '$stat' '$title'");
#     die "system() failed with status $rc" unless ($rc == 0);
	my $rscript=<<EOF;
data <- read.table("$infile", col.names = c("numHosts", "repetition", "value"));
pdf(file="$infile.pdf", width=10, height=10);
plotmeans(data\$value ~ data\$numHosts, main="$title", xlab="Number of Hosts per ONU", ylab = "$stat", n.label=FALSE);
dev.off();
EOF

	print RPIPE $rscript;
}

close RPIPE
	or warn $! ? "Error closing pipe with R: $!"
	: "Exit status $? from pipe with R";

# merge pdf files into one
my $pdf = $infile_base . ".pdf";
my $pdfs = $infile_base . ".ftp_dly.pdf";
$pdfs .= " $infile_base" . ".ftp_thr.pdf";
$pdfs .= " $infile_base" . ".ftp_trf.pdf";
$pdfs .= " $infile_base" . ".http_dly.pdf";
$pdfs .= " $infile_base" . ".http_thr.pdf";
$pdfs .= " $infile_base" . ".http_trf.pdf";
$pdfs .= " $infile_base" . ".video_dfr.pdf";
my $rc = system("pdftk $pdfs cat output $pdf");
die "system() failed with status $rc" unless ($rc == 0);

# delete the original single pdf files
foreach my $stat (keys %exts) {
	my $pdffile = $infile_base . ".$exts{$stat}.pdf";
	unlink($pdffile);
}
