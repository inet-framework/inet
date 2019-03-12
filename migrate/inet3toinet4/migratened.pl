#
#
#

use File::Basename;
my $dirname = dirname(__FILE__);
require $dirname."/replacements.pl";

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
#print join(' ', @fnames);

foreach $fname (@fnames)
{
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);

    # process $txt:
    foreach my $from (keys(%replSignals1)) {
        my $to = $replSignals1{$from};
        $txt =~ s/\b$from\b/$to/sg;
    }

    foreach my $from (keys(%replSignals2)) {
        my $to = $replSignals2{$from};
        $txt =~ s/\b$from\b/$to/sg;
    }

    foreach my $i (keys(%replStatistics)) {
        my $ii = '@statistic['.$i.']';
        my $rr = '@statistic['.$replStatistics{$i}.']';
        $txt =~ s|\Q$ii\E|$rr|gs;
    }

    writefile($fname, $txt);
    #writefile("$fname.new", $txt);
}


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

