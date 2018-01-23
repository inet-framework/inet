#!/usr/bin/perl
#
# Usage: collect_statistics listfile
#

$verbose = 1;

$listfile = $ARGV[0];
die "no listfile specified" if ($listfile eq '');

# parse listfile
print "reading $listfile...\n" if ($verbose);
open(INFILE, $listfile) || die "cannot open $listfile";
@fnames = ();
while (<INFILE>) {
    chomp;
    s/\r$//; # cygwin/mingw perl does not do CR/LF translation
    push(@fnames,$_);
}

%statistics = ();

sub camelize
{
    my $str = shift;

    # remove trailer '_t'
    $str =~ s/_t$//g;
    $str =~ s/_t(Descriptor)$/$1/g;

    #special cases:
    $str =~ s/AODVRREP/AodvRrep/g;
    $str =~ s/^(AODV|BGP|OSPF|PIM|RTCP|RTP)/ucfirst(lc($1))/e;

    $str =~ s/(?<=.)_(?!(Base$|var$|$))/#/g;
    $str =~ s/(?<=#)([a-z])/uc($1)/ge;

    $str = ucfirst($str);
    $str =~ s/(([A-Z][A-Z]+((v\d)?|\d+))(?=[A-Z#_]|$))/ucfirst(lc($1))/ge;
    $str =~ s/#//g;
    lcfirst($str);
}

sub addItem
{
    my $txtName = shift;
    my $newTxtName = $txtName;
    $newTxtName = camelize($newTxtName);
    $statistics{$txtName} = $newTxtName;
}

foreach $fname (@fnames)
{
    $fname1 = $fname;
    $fname1 =~ s|^(\.\./)+src/inet/||;
    # read files
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);

    # zap comments
    $txt =~ s|/\*.*?\*/|\n|gs;
    $txt =~ s|\s*//.*?\n|\n|gs;

    # joinbackslashed lines
    $txt =~ s|\\\n||gs;

    # @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    $txt =~ s|^\s*\@statistic\[(\w+)\]|addItem($1);""|gme;
}

# print items

$out = "### rename statistics:\n";

for $i (sort(keys(%statistics))) {
    $out .= "### " if ($i eq $statistics{$i});
    $out .= "$i -> $statistics{$i}\n";
}

writefile("statistics_rename.txt", $out);

sub readfile ()
{
    my $fname = shift;
    my $content;
    open FILE, "$fname" || die "cannot open $fname";
    read(FILE, $content, 1000000);
    close FILE;
    $content;
}

sub writefile ()
{
    my $fname = shift;
    my $content = shift;
    open FILE, ">$fname" || die "cannot open $fname for write";
    print FILE $content;
    close FILE;
}


