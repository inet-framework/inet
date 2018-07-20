#!/usr/bin/perl
#
# Generates one new mappingfile from two mappingfile, that equivalent with renamings mapping1file, after mapping2file file.
#
# Usage: merge_identrenaming mapping1file mapping2file
#

$verbose = 1;

if ($#ARGV < 2) {
    print STDERR "usage: merge_identrenaming mapping1file mapping2file outputfile\n";
    exit(1);
}

$mapping1file = $ARGV[0];
$mapping2file = $ARGV[1];
$outfile = $ARGV[2];

# parse mapping1file
print "reading $mapping1file...\n" if ($verbose);
$mapping1 = readfile($mapping1file);
$mapping1 =~ s|^\s*(.*?)\s*->\s*(.*?)\s*$|$map1{$1}=$2;""|gme;

# parse mapping2file
print "reading $mapping2file...\n" if ($verbose);
$mapping2 = readfile($mapping2file);
$mapping2 =~ s|^\s*(.*?)\s*->\s*(.*?)\s*$|$map2{$1}=$2;""|gme;

# debug: print map1
print "map1:\n";
for $i (sort(keys(%map1))) { print "  $i -> $map1{$i}\n"; }

# debug: print map2
print "\nmap2:\n";
for $i (sort(keys(%map2))) {print "  $i -> $map2{$i}\n";}

foreach my $k (keys(%map1)) {
    my $v = $map1{$k};
    if (exists $map2{$v}) {
        $map1{$k} = $map2{$v};
        delete($map2{$v});
    }
}

%map = (%map1, %map2);

$txt = "";

for $i (sort(keys(%map))) {
    $txt .= "$i -> $map{$i}\n" if ($i ne $map{$i});
}

writefile($outfile, $txt);

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

