#!/usr/bin/perl
#
# Usage: collect_signals listfile
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

@signals = ();

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

    # zap Define_Module_Like (2nd arg is a NED name)
    $txt =~ s|Define_Module_Like\s*\(.*?\)|\n|gs;

    # zap dllexport stuff
    $txt =~ s|\b[A-Z]+_API\b||gs;

    $txt =~ s|\bINET_API\b||gm;

    # registerSignal("NF_PP_TX_BEGIN");
    # simsignal_t NF_PP_TX_END = cComponent::registerSignal("NF_PP_TX_END");
    $txt =~ s|^\s*(?:const\s+)?(?:simsignal_t\s+)?((?:(?:[A-Za-z0-9_]+)::)*)([A-Za-z0-9_]+)\s*=\s*(?:cComponent::)?registerSignal\("([^"]+)"\)|push(@signals,"\"\"\"$3\"\"\"\t$2\t$1\t$fname1");""|gme;

    $txt =~ s|^(.*registerSignal\(.*)$|push(@othersignals,"$1\t$fname1");""|gme;
}

# print signals

###print join("\n",@signals);
print "Unrecognised registersignals calls:\n";
print join("\n",sort(@othersignals)),"\n";

$out = "Name\tVariable\tNamespace\tFilename\n";
$out .= join("\n", sort(@signals));
$out .= "\n";

writefile("signals.csv", $out);

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


