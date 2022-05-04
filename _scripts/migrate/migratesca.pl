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
    $txt = readfile($fname);

    # process $txt:
    $txt =~ s/(\.arp) "ARP requests sent" (\d+)\n/$1 sentReq:count $2\nattr title "ARP request sent, count"\n/sg;
    $txt =~ s/(\.arp) "ARP replies sent" (\d+)\n/$1 sentReply:count $2\nattr title "ARP replies sent, count"\n/sg;
    $txt =~ s/(\.arp) "failed ARP resolutions" (\d+)\n/$1 failedARPResolution:count $2\nattr title "ARP failed resolutions, count"\n/sg;
    $txt =~ s/(\.arp) "ARP resolutions" (\d+)\n/$1 completedARPResolution:count $2\nattr title "ARP completed resolutions, count"\n/sg;
#    $txt =~ s/(\.queue) "" (\d+)\n/$1 :count $2\nattr title ""\n/sg;
    $txt =~ s/(\.queue) "packets received by queue" (\d+)\n/$1 rcvdPk:count $2\nattr interpolationmode none\nattr title "received packets, count"\n/sg;
    $txt =~ s/(\.queue) "packets dropped by queue" (\d+)\n/$1 dropPk:count $2\nattr interpolationmode none\nattr source dropPkByQueue\nattr title "dropped packets, count"\n/sg;

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




