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

%signals = ();

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

sub addSignal
{
    my $txtName = shift;
    my $cName = shift;
    my $namespace = shift;
    my $filename = shift;
    my $newTxtName = $txtName;
    $newTxtName =~ s/^NF_//;
    $newTxtName = camelize($newTxtName);
    my $newCName = $cName;
    $newCName =~ s/^NF_//;
    $newCName = camelize($newCName);
    unless ($newCName =~ m/Signal$/) {
        $newCName .= "Signal";
    }
    $signals{"\"$txtName\""} = "\"$newTxtName\"";
    $signals{$cName} = $newCName;
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

    # zap Define_Module_Like (2nd arg is a NED name)
    $txt =~ s|Define_Module_Like\s*\(.*?\)|\n|gs;

    # zap dllexport stuff
    $txt =~ s|\b[A-Z]+_API\b||gs;

    $txt =~ s|\bINET_API\b||gm;

    # registerSignal("NF_PP_TX_BEGIN");
    # simsignal_t NF_PP_TX_END = cComponent::registerSignal("NF_PP_TX_END");
    $txt =~ s|^\s*(?:const\s+)?(?:simsignal_t\s+)?((?:(?:[A-Za-z0-9_]+)::)*)([A-Za-z0-9_]+)\s*=\s*(?:cComponent::)?registerSignal\("([^"]+)"\)|addSignal($3, $2, $1, $fname1);""|gme;

    $txt =~ s|^(.*registerSignal\(.*)$|push(@othersignals,"$1\t$fname1");""|gme;
}

# print signals

###print join("\n",@signals);
print "Unrecognised registersignals calls:\n";
print join("\n",sort(@othersignals)),"\n";

$out = "### rename signals:\n";
$out .= join("\n", sort(@signals));
$out .= "\n";

for $i (sort(keys(%signals))) {
    $out .= "### " if ($i eq $signals{$i});
    $out .= "$i -> $signals{$i}\n";
}

writefile("signals_rename.txt", $out);

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


