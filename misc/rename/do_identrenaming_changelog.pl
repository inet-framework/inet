#!/usr/bin/perl
#
# update changelogs for files in listfile with renamings in mappingfile
#
# Usage: do_identrenaming_changelog listfile mappingfile
#

sub fileexists
{
    my $fname = shift;
    print "open ('<$fname');\n" if ($verbose >= 2);
    open (my $fh, "<", $fname) || return 0;
    return 1;
}

sub readfile
{
    my $fname = shift;
    my $content;
    print "open ('<$fname');\n" if ($verbose >= 2);
    open (my $fh, "<", $fname) || die "cannot open $fname $!";
    my $r = read($fh, $content, 1000000);
    print "read $fname: $r ", length($content),"\n" if ($verbose >= 2);
    return $content;
}

sub writefile
{
    my $fname = shift;
    my $content = shift;
    open (FILE, ">$fname") || die "cannot open $fname for write";
    print FILE $content;
    close FILE;
}

sub addfile
{
    my $fpath = shift;
    my $fname = shift;
    my $x = "old";
    if (!exists $dirs{$fpath}) {
        $dirs{$fpath} = $fname;
        $x = "new";
    }
    else {
        $dirs{$fpath} .= "\n".$fname;
    }
    print "---addfile($fpath; $fname); : $x\n" if ($verbose >= 2);
}

$verbose = 2;

$listfile = $ARGV[0];
$mappingfile = $ARGV[1];

if ($#ARGV < 1) {
    print STDERR "usage: do_identrenaming_changelog listfile mappingfile\n";
    exit(1);
}

# parse listfile
print "reading $listfile...\n" if ($verbose);
%dirs = ();
$listfilecontents = readfile($listfile);
$listfilecontents =~ s|^\s*(.*?)/([^/]+)\s*$|addfile($1,$2);""|gme;

# parse mappingfile
print "reading $mappingfile...\n" if ($verbose);
%map = ();
$mapping = readfile($mappingfile);
$mapping =~ s|^\s*(.*?)\s*->\s*(.*?)\s*$|$map{$1}=$2;""|gme;

# debug: print files
if ($verbose >= 2) {
    print "filelist:\n";
    for $i (sort(keys(%dirs))) {
        print "  $i:\n";
        my @files = split("\n", $dirs{$i});
        foreach $j (@files) {
            print "    $i/$j\n";
        }
    }
}

# debug: print map
if ($verbose >= 2) {
    print "mapping list:\n";
    for $i (sort(keys(%map))) {print "  $i -> $map{$i}\n";}
}

# do it

print "\nUpdate ChangeLog files:\n" if ($verbose);
for $dir (sort(keys(%dirs))) {
    if (!fileexists("$dir/ChangeLog")) {
        print "Skip $dir. ChangeLog file is missing\n";
    }
    else {
        print "do $dir. ChangeLog file is exists\n";
        my $changelog = readfile("$dir/ChangeLog");
        my $changelog_bak = $changelog;
        my @files = split("\n", $dirs{$dir});
        my %renames = ();
        foreach $file (@files) {
            my $fname = "$dir/$file";
            print "    $fname\n";
            my $txt = readfile("$fname");
            for $k (keys(%map)) {
                $v = $map{$k};
                if ($txt =~ m|\b$v\b|) {
                    $renames{$k} = $v;
                    print "    Rename $k -> $v found in file $dir/$file\n" if ($verbose >= 2);
                }
            }
        }
        if (keys %renames) {
            # update changelog text:
            $commit = "2017-12-20  Zoltan Bojthe\n\n\tRenaming:\n";
            for $k (sort(keys(%renames))) {
                $commit .= "\t    $k renamed to $renames{$k}\n";
            }
            $commit .= "\n";
            $changelog =~ s/^(====== inet-4.x ======\n\n)/\1$commit/;

            # update changelog file if needed:
            writefile("$dir/ChangeLog.bak", $changelog_bak);
            writefile("$dir/ChangeLog", $changelog);
        }
    }
}

print "done -- backups saved as .bak\n" if ($verbose);


