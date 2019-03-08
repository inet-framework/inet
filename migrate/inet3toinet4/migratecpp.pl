
use File::Basename;
my $dirname = dirname(__FILE__);
require $dirname."/replacements.pl";

# from msg files

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

    open(INFILE, $fname) || die "cannot open $fname";
    read(INFILE, $txt, 1000000) || die "cannot read $fname";
    close INFILE;

    my $origtxt = $txt;

    # process $txt:

    # do signal renamings
    foreach my $from (keys(%replSignals1)) {
        my $to = $replSignals1{$from};
        $txt =~ s/"${from}"/"${to}"/sg;
        $txt =~ s/\b${from}Signal\b/${to}Signal/sg;
    }

    foreach my $from (keys(%replSignals2)) {
        my $to = $replSignals2{$from};
        $txt =~ s/"${from}"/"${to}"/sg;
        $txt =~ s/\b${from}Signal\b/${to}Signal/sg;
    }

    # do class renamings
    foreach my $from (keys(%replClasses)) {
        my $to = $replClasses{$from};
        $txt =~ s/\b${from}\b/${to}/sg;
    }

    if ($txt eq $origtxt) {
        print "unchanged\n";
    } else {
        open(OUTFILE, ">$fname") || die "cannot open $fname for write";
        print OUTFILE $txt || die "cannot write $fname";
        close OUTFILE;
        print "DONE\n";
    }
}

# BEWARE OF BOGUS REPLACEMENTS INSIDE COMMENTS!!
# getAddress(), getData(), getPayload() !!!!
print "\nConversion done. You may safely re-run this script as many times as you want.\n";

