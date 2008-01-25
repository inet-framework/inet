#
# Process raw nedfiles.lst produced by dir /s /b:
#    - make paths relative to INET Fw root directory
#    - remove NED files in the Examples, Unsupported and Tests directories
#

$listfile = $ARGV[0];
$rootdir = $ARGV[1];
die "usage: processNEDFileList <listfile> <inet-root-dir>" if ($listfile eq '' || $rootdir eq '');

open(IN, $listfile) || die "cannot open $listfile";
open(OUT, ">$listfile.new") || die "cannot open $listfile.new for write";
while (<IN>)
{
    if (!/.ned$/ ) {next;}
    s|^\Q$rootdir\E\\*||g;
    s|\\|/|g;
    if (/^Examples\// || /^Unsupported\// || /^Obsolete\// || /^Tests\//) {next;}
    print OUT;
}
close(IN);
close(OUT);

unlink("$listfile.bak");
rename($listfile, "$listfile.bak") || die "cannot rename $listfile to .bak";
rename("$listfile.new", $listfile) || die "cannot rename $listfile.new to $listfile";



