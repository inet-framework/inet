#
#
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
#print join(' ', @fnames);

foreach $fname (@fnames)
{
    print "reading $fname...\n" if ($verbose);

    open(FILE, "$fname") or die "cannot open $fname";
    my $count = 0;
    my @vec = ();
    my $line;
    foreach $line (<FILE>) {
        $line =~ s/[\r\n]//g;
        next if ($line eq "");
        if ( $line =~ /^((scalar)|(statistic))/ ) {
            $count++;
        }
        $vec[$count] .= $line."\n";
    }

    my $head = shift @vec;
    my $txt = $head."\n".join('',sort(@vec));
    close FILE;

    writefile($fname, $txt);
}


sub writefile ()
{
    my $fname = shift;
    my $content = shift;
    open FILE, ">$fname" || die "cannot open $fname for write";
    print FILE $content;
    close FILE;
}




