#! /usr/bin/perl
#
# Utility to add namespace to the INET source files. Tries to guess where to best insert
# the "namespace inet {", "}//namespace", and "using namespace inet;" lines.
#
# Needs to be run in each source directory. Result probably needs hand-editing.
#
# --Andras Varga
#

$verbose = 1;

@hfiles = glob("*.h");
@ccfiles = glob("*.cc");

$fnamesuffix = "";
#$fnamesuffix = ".new";  # for testing

foreach $fname (@hfiles)
{
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);

    $txt =~ s!^(.*\n *# *include .*?\n)(.*)(\n#endif)!$1\nnamespace inet {\n$2\n} //namespace\n\n$3!s;

    writefile($fname . $fnamesuffix, $txt);
}

foreach $fname (@ccfiles)
{
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);

    # add "using namespace" after the last #include or #endif
    $txt =~ s!^([^{]*\n *# *[ei][n][dc][il][fu].*?\n)!$1\nusing namespace inet;\n!s;

    writefile($fname . $fnamesuffix, $txt);
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

