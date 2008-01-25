$fname="inetconfig.vc-SAMPLE";
$outfname="inetconfig.vc";
$variable="OMNETPP_ROOT";

# find OMNeT++ in the PATH
$path=$ENV{'PATH'};
$value='';
foreach $dir (split(';',$path)) {
  if (-f "$dir/nedtool.exe") {
     $value = $dir;
     last;
  }
}
if ($value eq '') {die "OMNeT++ not found in the PATH";}
$value =~ s/\\bin\\?$//;  # chop off "\bin"  at the end
if (! -f "$value\\configuser.vc") {die "configuser.vc not found at expected location, in $value";}

# substitute it into the input file, and save it as output file
open(INFILE, "$fname") || die "cannot open $fname";
read(INFILE, $config, 1000000) || die "cannot read $fname";
close INFILE;

if ($config =~ /^ *$variable *=/m) {
    # replace only 1st occurrence
    $config =~ s/^ *$variable( *)=.*$/$variable\1=$value/m;
} else {
    die "$fname doesn't contain $variable;";
}

open(OUTFILE, ">$outfname") || die "cannot open $outfname for write;";
print OUTFILE $config || die "cannot write $outfname;";
close OUTFILE;



