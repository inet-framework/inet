#
#
#

use File::Basename;
my $dirname = dirname(__FILE__);
require $dirname."/replacements.pl";


$listfname = $ARGV[0];
open(LISTFILE, $listfname) || die "cannot open $listfname";
while (<LISTFILE>)
{
    chomp;
    s/\r$//; # cygwin/mingw perl does not do CR/LF translation

    $fname = $_;

    if ($fname =~ /_m\./) {
        print "skipping $fname...\n";
        next;
    }

    print "processing $fname... ";

    $txt = readfile($fname);

    my $origtxt = $txt;

    # process $txt:

    # do signal renamings
    foreach my $from (keys(%replSignals1)) {
        my $to = $replSignals1{$from};
        $txt =~ s/\b${from}\b/${to}/sg;
    }

    foreach my $from (keys(%replSignals2)) {
        my $to = $replSignals2{$from};
        $txt =~ s/\b${from}\b/${to}/sg;
    }

    # do statistic renamings
    foreach my $i (keys(%replStatistics)) {
        my $ii = '@statistic['.$i.']';
        my $rr = '@statistic['.$replStatistics{$i}.']';
        $txt =~ s|\Q${ii}\E|${rr}|gs;
    }

    # do class renamings
    foreach my $from (keys(%replClasses)) {
        my $to = $replClasses{$from};
        $txt =~ s/\b${from}\b/${to}/sg;
    }

    if ($txt eq $origtxt) {
        print "unchanged\n";
    } else {
        writefile($fname, $txt);
        print "DONE\n";
    }
}

# BEWARE OF BOGUS REPLACEMENTS INSIDE COMMENTS!!
# getAddress(), getData(), getPayload() !!!!
print "\nConversion done. You may safely re-run this script as many times as you want.\n";

sub readfile ()
{
    my $fname = shift;
    my $content;
    open FILE, "$fname" || die "cannot open $fname";
    read(FILE, $content, 1000000) || die "cannot read $fname";
    close FILE;
    $content;
}

sub writefile ()
{
    my $fname = shift;
    my $content = shift;
    open FILE, ">$fname" || die "cannot open $fname for write";
    print FILE $content || die "cannot write $fname";
    close FILE;
}

