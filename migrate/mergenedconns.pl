#
# Edits NED files to merge two one-way connections to one.
# It looks for the following pattern:
#
#    a.ggg$o --> b.ggg$i;
#    a.ggg$i <-- b.ggg$o;
#
# and replaces it with
#
#    a.ggg <--> b.ggg;
#
# Handles channels, gate++/gate[i], submod-to-parent connections etc.
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

    # if this is a network (simplification: we check if file contains at least one network)
    if ($txt =~ /^ *network\b/m)
    {
        # replace "in" gate with "pppg$i", "ethIn" with "ethg$i", etc
        $txt =~ s/\.in\b/.pppg\$i/g;
        $txt =~ s/\.out\b/.pppg\$o/g;
        $txt =~ s/\.ethIn\b/.ethg\$i/g;
        $txt =~ s/\.ethOut\b/.ethg\$o/g;
    }

    # Then merge $i/$o connection pairs
    $gateo = '(\w+(\[.*?\])?\.)?\w+\$o(\+\+)?(\[.*?\])?';  # 4 nested subexprs
    $gatei = '(\w+(\[.*?\])?\.)?\w+\$i(\+\+)?(\[.*?\])?';

    $rightarrow = ' *--> *(\w* *({.*?} *)?--> *)?';  # 2 nested subexprs
    $leftarrow =  ' *<-- *(\w* *({.*?} *)?<-- *)?';

    while ($txt =~ /(${gateo})(${rightarrow})(${gatei});/m  ||
           $txt =~ /(${gatei})(${rightarrow})(${gatei});/m  ||
           $txt =~ /(${gateo})(${rightarrow})(${gateo});/m  ||
           $txt =~ /(${gatei})(${leftarrow})(${gatei});/m   ||
           $txt =~ /(${gateo})(${leftarrow})(${gateo});/m)
    {
        $leftg = $1;
        $chan = $6;
        $rightg = $9;
        mergeconn($leftg, $chan, $rightg);
    }

    $txt =~ s/--TBD-->/-->/g;
    $txt =~ s/<--TBD--/<--/g;

    writefile($fname, $txt);
    #writefile("$fname.new", $txt);
}


sub mergeconn ()
{
    my $leftg = shift;
    my $chan = shift;
    my $rightg = shift;

    #print "found $& ==> $leftg; $chan; $rightg\n";

    $conntext = "$leftg$chan$rightg";

    $replacement = "$leftg$chan$rightg";
    $replacement =~ s/-->/<-->/g;
    $replacement =~ s/\$[io]//g;

    $leftginv = $leftg;
    $chaninv = $chan;
    $rightginv = $rightg;

    $leftginv =~ s/\$i/\$X/;
    $leftginv =~ s/\$o/\$i/;
    $leftginv =~ s/\$X/\$o/;

    $rightginv =~ s/\$i/\$X/;
    $rightginv =~ s/\$o/\$i/;
    $rightginv =~ s/\$X/\$o/;

    $chaninv =~ s/-->/-X-/g;
    $chaninv =~ s/<--/-->/g;
    $chaninv =~ s/-X-/<--/g;

    $delete1 = "$leftginv$chaninv$rightginv;";
    $delete2 = "$rightginv$chan$leftginv;";

    print " $conntext  TO:  $replacement\n";
    #print "   delete: $delete1\n";
    #print "   delete: $delete2\n";

    # if we find the opposite direction too, go ahead
    if ($txt =~ /\Q$delete1\E/ || $txt =~ /\Q$delete2\E/) {
        print "   REPLACED!\n";
        $txt =~ s/\Q$conntext\E/$replacement/g;
        $txt =~ s/ *\Q$delete1\E *\n//gs;
        $txt =~ s/ *\Q$delete2\E *\n//gs;
    } else {
        print "   SKIPPED (opposite direction not found, or has different channel)\n";
        $failconntext = $conntext;
        $failconntext =~ s/-->/--TBD-->/g;
        $failconntext =~ s/<--/<--TBD--/g;
        $txt =~ s/\Q$conntext\E/$failconntext/g;
    }
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




