#!/usr/bin/perl
#
# Helps enforce naming convention for variable, type, function etc names.
#
# Usage: suggest_identrenaming listfile
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

foreach $fname (@fnames)
{
    # read files
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);
    $fulltxt .= $txt;
}

# zap comments
$fulltxt =~ s|/\*.*?\*/|\n|gs;
$fulltxt =~ s|\s*//.*?\n|\n|gs;

# joinbackslashed lines
$fulltxt =~ s|\\\n||gs;

# zap string constants
$fulltxt =~ s|\\\\||gs;
$fulltxt =~ s|\\"||gs;
$fulltxt =~ s|"[^"\n]*"|""|gs;

# zap Define_Module_Like (2nd arg is a NED name)
$fulltxt =~ s|Define_Module_Like\s*\(.*?\)|\n|gs;

# zap dllexport stuff
$fulltxt =~ s|\b[A-Z]+_API\b||gs;

#writefile('$$$',$fulltxt);

for $i (("NULL","FILE",
         "WATCH","WATCH_LIST","WATCH_VECTOR","WATCH_SET","WATCH_MAP","WATCH_MULTIMAP",
         "WATCH_PTRLIST","WATCH_PTRVECTOR","WATCH_PTRSET","WATCH_PTRMAP","WATCH_PTRMULTIMAP",
         "FT_BASIC","FT_BASIC_ARRAY","FT_INVALID","FT_STRUCT","FT_STRUCT_ARRAY",
         "EXECUTE_ON_STARTUP","Enter_Method","Enter_Method_Silent",
         "FSM_Steady","FSM_Transient","FSM_Switch","FSM_Goto",
         "Define_Function","Define_Function2","Define_AdvFunction",
         "Register_Class","Define_Network","Define_Channel",
         "Define_Module","Define_Module_Like","Module_Class_Members")) {
    $macrostypes{$i}="ignore";
}

$fulltxt =~ s|\bINET_API\b||gm;

# identify defines and type names
$fulltxt =~ s|^\s*#define\s+([A-Za-z0-9_]+)\s.*|$macrostypes{$1}="def";""|gme;
$fulltxt =~ s|^\s*#define\s+([A-Za-z0-9_]+)\b.*|$macrostypes{$1}="macro";""|gme;
$fulltxt =~ s!^\s*(?:static\s+)?const\s+(unsigned)?\s*(bool|char|short|int|long|float|double|u?int\d+_t)\s+([A-Za-z0-9_]+)\b.*!$macrostypes{$3}="const";""!gme;
$fulltxt =~ s|\bclass\s+([A-Za-z0-9_]+)\b|$macrostypes{$1}="class";$&|gse;
$fulltxt =~ s|\bstruct\s+([A-Za-z0-9_]+)\b|$macrostypes{$1}="struct";$&|gse;
$fulltxt =~ s|\benum\s+(?:class\s+)?([A-Za-z0-9_]+)\b|$macrostypes{$1}="enum";$&|gse;
$fulltxt =~ s|\bunion\s+([A-Za-z0-9_]+)\b|$macrostypes{$1}="union";$&|gse;
$fulltxt =~ s|\btypedef\s+[^;]+\b([A-Za-z0-9_]+)\s*;|$macrostypes{$1}="typedef";$&|gse;

# collect constants from enums as well
$fulltxt =~ s|\benum\s+(?:class\s+)?([A-Za-z0-9_]+)?\s*({.*?})|$enumbodies.=$2;$&|gse;
$enumbodies =~ s|=[^,}]*||gs;
$enumbodies =~ s|([A-Za-z_][A-Za-z0-9_]*)|$macrostypes{$1}="enumval";$&|gse;

# zap preprocessor directives
$fulltxt =~ s|^\s*#.*?$|\n|gm;

#writefile('$$$1',$fulltxt);

# collect full list of identifiers that occur in the program
$fulltxt =~ s|([A-Za-z_][A-Za-z0-9_]*)|$identifier{$1}=1;""|gse;

# identifiers which are not in %macrostypes are method and variable names
for $i (keys(%identifier)) {$varsfuncs{$i}=1 if (!defined($macrostypes{$i}));}

$out = "";
$out .= "defines:\n".  collect("def")."\n\n";
$out .= "const:\n".    collect("const")."\n\n";
$out .= "enumval:\n".  collect("enumval")."\n\n";
$out .= "macros:\n".   collect("macro")."\n\n";
$out .= "classes:\n".  collect("class")."\n\n";
$out .= "structs:\n".  collect("struct")."\n\n";
$out .= "enums:\n".    collect("enum")."\n\n";
$out .= "unions:\n".   collect("union")."\n\n";
$out .= "typedef:\n".  collect("typedef")."\n\n";
$out .= "varsfuncs:\n    ".join("\n    ",sort keys(%varsfuncs))."\n\n";

writefile("ident.txt", $out);

# rename variables and methods (result is stored in %map)
for $i (keys(%varsfuncs)) {
#    next;
    $i2 = lcfirst(camelize($i));
    if ($i ne $i2) {
        $map{$i} = $i2;
        $inversemap{$i2} .= "$i ";
    }
}

# rename constants and type names (result is stored in %map)
for $i (keys(%macrostypes)) {
    $type = $macrostypes{$i};
    if ($i eq "DYMOMetricType") { print "BZBZBZBZ $i $type\n"; }
    if ($type eq "ignore") {
        if ($i eq "DYMOMetricType") { print "BZBZBZBZ-ignore $i $type\n"; }
        next;
    }
    if ($type eq "def" || $type eq "macro" || $type eq "const" || $type eq "enumval") {
        if ($i eq "DYMOMetricType") { print "BZBZBZBZ-def... $i $type\n"; }
        next;
        $i2 = uc($i);
    } else {
        if ($i eq "DYMOMetricType") { print "BZBZBZBZ-else $i $type\n"; }
        if ($i =~ /^(u|u_)?(int|short|long|char)[0-9]*(_t)?$/) {
            # something like u_int32_t, leave it alone
            $i2 = $i;
        } else {
            $i2 = camelize($i);
        }
    }
    if ($i ne $i2) {
        $map{$i} = $i2;
        $inversemap{$i2} .= "$i ";
    }
}

# look for conflicts
$out = "# review, and edit if necessary\n";
$out .= "\n# Conflicts:\n\n";
for $i (sort(keys(%inversemap))) {
    # two different renamings both resulted in $i?
    $forgetit = 0;
    if (split(" ",$inversemap{$i})>=2) {
        print "CONFLICT: $i $inversemap{$i}  (renamings of $inversemap{$i}would all result in $i)\n";
        $out .= "# CONFLICT: * $i  * $inversemap{$i}  (renamings of $inversemap{$i}would all result in $i)\n";
###        $out .= "$i -> $map{$i}\n" if (defined($map{$i}));
        $forgetit = 1;
    }
    # a renamed ident conflicts with an existing ident?
    if (defined($identifier{$i})) {
        print "CONFLICT: $inversemap{$i} $i  ($inversemap{$i}is not safe to rename because $i already exists)\n";
        $out .=  "# CONFLICT: * $inversemap{$i} * $i  ($inversemap{$i}is not safe to rename because $i already exists)\n";
        $out .= "$inversemap{$i}-> $i\n";
        $forgetit = 1;
    }
    # revoke renaming suggestion
    if ($forgetit) {
        for $k (split(" ",$inversemap{$i})) {
            undef($map{$k});
        }
        undef($inversemap{$i});
    }
}

# print mapping
$out .= "\n# OK:\n\n";
for $i (sort {uc($a) cmp uc($b)} keys(%map)) {
    $out .= "$i -> $map{$i}\n" if (defined($map{$i}));
}

writefile("mapping.txt", $out);

print "see ident.txt and mapping.txt for results\n";
print "edit mapping.txt, then run do_identrenaming to execute it\n";

sub collect ()
{
    my $value = shift;
    my $ret = "";
    for $i (sort(keys(%macrostypes))) {
        $ret .= $i."\n    " if ($macrostypes{$i} eq $value);
    }
    $ret;
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

sub camelize()
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
    $str;
}
