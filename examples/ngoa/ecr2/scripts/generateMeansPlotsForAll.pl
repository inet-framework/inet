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
    die "Usage: generateMeansPlotsForAll.pl common_base_name_of_data_files";
}

# initialize variables
my $infile_base = $ARGV[0];
my $home = $ENV{'HOME'};
my $shellScript = $home . "/inet-hnrl/examples/ngoa/ecr2/scripts/generateMeansPlot.sh";
my %exts = (
	"Average Delay of FTP Sessions [sec]" => "ftp_dly",
	"Average Throughput of FTP Sessions [Byte/sec]" => "ftp_thr",
	"Mean Transfer Rate of FTP Sessions [Byte/sec]" => "ftp_trf",
	"Average Delay of HTTP Sessions [sec]" => "http_dly",
	"Average Throughput of HTTP Sessions [Byte/sec]" => "http_thr",
	"Mean Transfer Rate of HTTP Sessions [Byte/sec]" => "http_trf",
	"Average Decodable Frame Rate of Streaming Videos (Q)" => "video_dfr",
);

# generate group means plots into pdf files
foreach my $stat (keys %exts) {
	my $infile = $infile_base . ".$exts{$stat}";
	my $rc = system("$shellScript $infile '$stat'");
    die "system() failed with status $rc" unless ($rc == 0);
}

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
