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

%replacements = (
    # parameters
    "local_port" => "localPort",
    "dest_port" => "destPort",
    "message_length" => "messageLength",
    "message_freq" => "messageFreq",
    "dest_addresses" => "destAddresses",
);

foreach $fname (@fnames)
{
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);

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

