#!/usr/bin/perl
#
# replaces occurrences of @INSERTFILE(filename) in html files with
# contents of the file.
#
# Usage: perl proc.pl <directory>
#
# Author: Andras Varga, 2003
#

$dir = $ARGV[0];
die "no directory specified" if ($dir eq '');

$fnamepatt = "$dir/*.html";
foreach $fname (glob($fnamepatt))
{
        # read file
        open(INFILE, $fname) || die "cannot open $fname";
        read(INFILE, $html, 1000000) || die "cannot read $fname";

        # replacement
        $html =~ s|\@INSERTFILE\((.*?)\)|load_file($1)|gse;

        # save file
        open(FILE,">$fname");
        print FILE $html;
        close FILE;
}

sub load_file ()
{
    my $fname = shift;
    my $content;
    open FILE, "$dir/$fname" || die "cannot open $dir/$fname";
    read(FILE, $content, 1000000) || die "cannot read $dir/$fname";
    close FILE;
    $content;
}
