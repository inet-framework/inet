#!/usr/bin/perl

use strict;
use File::Glob;

# -----

my $dryrun = 0;

# -----

@ARGV >= 1 or die "usage: ".$0." quagga_root_dir|source_file [-predoxy]\n";

my $par = shift @ARGV;

my $file_src;
my $root_dir;

-f $par and $file_src = $par;
-d $par and $root_dir = $par;

($file_src || $root_dir) or die "source file or directory must be specified";

my $predoxy;

while(@ARGV > 0)
{
  $par = shift @ARGV;
  
  if($par eq "-predoxy")
  {
    $predoxy = 1;
  }
  else
  {
    die "unknown parameter $par";
  }
}


# -----

my %files;
my %modified;
my %paths;

my %vars;
my %vars_init;
my %vars_type;
my %vars_args;

my %outs;

if($predoxy)
{
  readinputfiles();

  my $foo = 0;
    
  foreach my $key (keys %files)
  {
    print STDERR "processing $key...\n";

    my @lines = split(/\n/, $files{$key});

    # handle aliases

    my $mod = 0;

    for(my $i = 0; $i < @lines; $i++)
    {
      if($lines[$i] =~ /^\s*ALIAS\s*\(/)
      {
	my $b = 0;
	do
	{
	  ($i < @lines) or die "end of ALIAS not found";
 
	  $b += ($lines[$i] =~ s/\(/\(/g);
	  $b -= ($lines[$i] =~ s/\)/\)/g);

	  ++$i;
	}
	while($b > 0);

	($b == 0) or die "too many right parentheses in ALIAS";
	($lines[$i-1] =~ /\)\s*$/) or die "trailing characters after ALIAS";

	# add trailing semicolon
	$lines[$i-1] =~ s/^(.*\))/$1;/;

	print STDERR "$key: line ".($i-1)." <-- XXX CHECK THIS\n";
	
	$modified{$key} = 1;

	++$mod;
      }
    }

    if($mod)
    {
      print STDERR "$key: number of ALIAS handled: $mod\n";
    }

    # handle anonymous structs
    
    $mod = 0;

    for(my $i = 0; $i < @lines; $i++)
    {
      if($lines[$i] =~ /^(static\s+)?struct\s*(\{)?$/)
      {
	my $static = $1;
	my $bracket = $2;

	++$foo;
	
	$lines[$i] =~ s/^(static\s+)?(struct\s*)\s?(\{)?$/$2 FOO$foo $3/;

	print STDERR "$key: line $i <-- XXX CHECK THIS\n";

	do
	{
	  ++$i;
	}
	while($lines[$i] !~ /^\s*\}/);

	$lines[$i] =~ s/^((\s*)\})/$1;\n$2${static}struct FOO$foo /;

	print STDERR "$key: line $i <-- XXX CHECK THIS\n";

        $modified{$key} = 1;

	++$mod;
      }
    }

    if($mod)
    {
      print STDERR "$key: number of anonymous structs handled: $mod\n";

      # to get correct line boundaries
      @lines = split(/\n/, join("\n", @lines));
    }

    # handle struct interface
    
    $mod = 0;

    for(my $i = 0; $i < @lines; $i++)
    {
      my $s = ($lines[$i] =~ s/\bstruct interface\b/struct interface_FOO/g);
      if($s)
      {
	$modified{$key} = 1;
	$mod += $s;
      }
    }

    if($mod)
    {
      print STDERR "$key: number of struct interface renamed: $mod\n";
    }

    # handle warning directive

    $mod = 0;

    for(my $i = 0; $i < @lines; $i++)
    {
      if($lines[$i] =~ m/^\s*#\s*warning\s+"/)
      {
	$lines[$i] = "#ifndef _MSC_VER\n".$lines[$i]."\n#endif";

	$modified{$key} = 1;
	++$mod;
      }
    }

    if($mod)
    {
      print STDERR "$key: number of #warning handled: $mod\n";

      # to get correct line boundaries
      @lines = split(/\n/, join("\n", @lines));
    }
    

    # handle route_map_rule_cmd and other to-be-static methods
    
    my @todo;
    
    for(my $i = 0; $i < @lines; $i++)
    {
      if($lines[$i] =~ /^struct route_map_rule_cmd\s+[\w_]+\s+=/)
      {
	for(my $j = $i + 3; $j <= $i + 5; $j++)
	{
	  my $x = $lines[$j];
	  $x =~ s/^\s*//;
	  $x =~ s/\s*,\s*$//;
	  push @todo, $x;
	}
      }
    }

    push @todo, "sighup";
    push @todo, "sigint";
    push @todo, "sigusr1";
    push @todo, "config_write_debug";

    # mark methods in todo array as static

    $mod = 0;
    
    for(my $i = 0; $i < @lines; $i++)
    {
      foreach my $f (@todo)
      {
	if($lines[$i] =~ /^$f\s*\(/)
	{
	  # prepend static
	  $lines[$i-1] = "static ".$lines[$i-1];

	  print STDERR "$key: line ".($i-1)." <-- XXX CHECK THIS\n";
	  print STDERR "$key: method $f marked as static\n";

	  $modified{$key} = 1;

	  ++$mod;

	  last;
	}
      }
    }

    # mark variables in todovar as static
    
    my @todovar;

    push @todovar, "char config_default";
    push @todovar, "struct option longopts";
    push @todovar, "zebra_capabilities_t _caps_p";
    push @todovar, "struct +FOO[0-9]+ +route_info";

    for(my $i = 0; $i < @lines; $i++)
    {
      foreach my $f (@todovar)
      {
	if($lines[$i] =~ /^$f/)
	{
	  # prepend static
	  $lines[$i] = "static ".$lines[$i];

	  print STDERR "$key: line $i <-- XXX CHECK THIS\n";
	  print STDERR "$key: variable $f marked as static\n";

	  $modified{$key} = 1;

	  ++$mod;

	  last;
	}
      }
    }

    if($modified{$key})
    {
      $files{$key} = join("\n", @lines)."\n";
    }

  }

  writefiles();

  exit(0);
}

my $glob_variable_name = "__activeVars";
my $glob_struct_name = "GlobalVars";

while(my $line = <STDIN>)
{
  chomp $line;
  my @f = split(/;/, $line);
  if($f[0] eq '@')
  {
    # output redirection
    
    my $fh = $f[1]; # stream identifier
    my $fn = $f[2];

    if($outs{$fh})
    {
      close($outs{$fh});
    }

    local *FH;
    open(FH, $fn) or die "unable to open $fn";
    $outs{$fh} = *FH;
  }
  elsif($f[0] eq 'w')
  {
    # echo string
    
    my $fh = $f[1]; # stream identifier
    my $str = decode($f[2]); # string to output

    do_output($str."\n", $outs{$fh});
  }
  elsif($f[0] eq 'K')
  {
    my $n = $f[1]; # syscall

    do_output("#define $n  oppsim_$n\n", $outs{'K_1'});
    
    # get rid of gcc warning
    $n =~ s/\(.*//;

    do_output("#undef $n\n", $outs{'K_2'});
  }
  elsif($f[0] eq 'I')
  {
    my $n = $f[1]; # source file
    my $x = $f[2]; # included header

    if(fileexists($x))
    {
      # zebra.h and serveral other local headers are included with <name>
      print STDERR "$n: ignoring local header $x\n";
      next;
    }

    readfile($n);

    my @lines = split(/\n/, $files{$n});

    my $mod = 0;

    for(my $i = 0; $i < @lines; $i++)
    {
      if($lines[$i] =~ /^\s*#\s*include\s+["<]$x[">]/)
      {
	$lines[$i] =  "#ifdef NATIVE_KERNEL\n".
		      "#include \"usyscalls.h\"\n".
		      "#include \"globalvars_off.h\"\n".
		      $lines[$i]."\n".
		      "#include \"syscalls.h\"\n".
		      "#include \"globalvars_on.h\"\n".
		      "#endif";
	++$mod;
      }
    }

    $files{$n} = join("\n", @lines)."\n";

    print STDERR "$n: $x $mod substitution(s) made\n";
    $modified{$n} = 1;
  }
  elsif($f[0] eq 'L')
  {
    my $d = $f[1]; # local variable definition
    my $v = $f[2]; # local variable name

    readinputfiles();

    foreach my $key (keys %files)
    {
      my @lines = split(/\n/, $files{$key});
      my $m = 0;
      for(my $i = 0; $i < @lines; $i++)
      {
	if($lines[$i] =~ /^\s*$d;$/)
	{
	  # we believe this statement defines local variable, so we...
	  ++$m;

	  # ...remove macro definiton
	  $lines[$i] = "#undef\t$v\n".$lines[$i];

	  # ...until closing bracket
	  my $b = 1;
	  do
	  {
	    $i < @lines or die "unable to find closing bracket";

	    ++$i;

	    $b += ($lines[$i] =~ s/{/{/g);
	    $b -= ($lines[$i] =~ s/}/}/g);

	  } while($b > 0);
	  $b == 0 or die "too many closing brackets found";

          # turn global variable back on
	  $lines[$i] .= "\n#define\t$v\t${v}__VAR";
	}
      }
      if($m)
      {
	print STDERR "$key: $m occurances of local variable $v were handled\n";
	$files{$key} = join("\n", @lines)."\n";
	$modified{$key} = 1;
      }
    }
  }
  elsif($f[0] eq 'S')
  {
    my $s = $f[1]; # structure name

    # replace "struct $s" with "struct_$s" in every source file

    readinputfiles();

    foreach my $key (keys %files)
    {
      my $n = ($files{$key} =~ s/\bstruct $s\b/struct_$s/msg);
      if($n)
      {
        print STDERR "$key: $n substitutions made on struct $s\n";
	$modified{$key} = 1;
      }
    }

    my $def = "#define  struct_$s struct ${s}_FOO\n";
    do_output($def, $outs{$f[0]});
  }
  elsif($f[0] eq 'T')
  {
    my $v = $f[1]; # variable name
    my $fn = $f[2]; # filename
    my $fl = $f[3]; # line number

    # trim filename
    $fn =~ s/^.*\///;

    # read file into memory
    readfile($fn);

    # rename variable on given position
    my @lines = split(/\n/, $files{$fn});
    ($lines[$fl-1] =~ s/\b$v\b/${v}__item/g) == 1 or die "patching failed: file=$fn var=$v line=".$lines[$fl-1];
    $files{$fn} = join("\n", @lines)."\n";
    $modified{$fn} = 1;

    # replace "->$v" (and ".$v") with "->$v__item" in every source file

    readfiles("*.c");
    readfiles("*.h");

    foreach my $key (keys %files)
    {
      my $n = ($files{$key} =~ s/->$v\b/->${v}__item/msg);
      $n += ($files{$key} =~ s/\.$v\b/.${v}__item/msg);
      if($n)
      {
        print STDERR "$key: $n substitutions made on struct member $v\n";
	$modified{$key} = 1;
      }
    }
  }
  elsif($f[0] eq 'G')
  {
    my $v = $f[1]; # variable name
    my $fn = $f[2]; # filename
    my $fl = $f[3]; # line number
    my $t = $f[4]; # variable type
    my $a = $f[5]; # arguments
    my $init = decode($f[6]); # initializer

    if($vars{$v})
    {
      # variable exist, type must be equal
      (($vars_type{$v} eq $t) && ($vars_args{$v} eq $a)) or die "type collision for global variable $v\n old type ".$vars_type{$v}." ".$vars_args{$v}."\n new type $t $a";
    }

    $vars_type{$v} = $t;
    $vars_args{$v} = $a;

    $vars{$v} = "$t ${v}__X$a";

    # get directory
    $fn =~ /^.*\/(.*?)\// or die "unable to retrieve directory name";
    my $dir = $1;

    if($init)
    {
      # if there is initialization of this variable...
      
      # ...then it must be unique in the given directory
      $vars_init{$dir}{$v} and die "error: multiple initialization of $v in directory $dir";


      if($t eq "struct route_map_rule_cmd")
      {
	# initializer references static routines, we must use memcpy in this case
	$vars_init{$dir}{$v} = "memcpy";
      }
      else
      {
        $vars_init{$dir}{$v} = $init;
      }
    }

    # variables defined via macros are not physically present in the sources
    ($fl eq "none") and next;

    # trim filename
    $fn =~ s/^.*\///;
    
    # read file into memory
    readfile($fn);

    # rename variable on given position
    my @lines = split(/\n/, $files{$fn});
    ($lines[$fl-1] =~ s/\b$v\b/${v}_$dir/g) == 1 or die "patching failed: file=$fn var=$v line=".$lines[$fl-1];
    $files{$fn} = join("\n", @lines)."\n";
    $modified{$fn} = 1;
  }
  elsif($f[0] eq 'P')
  {
    my $v = $f[1]; # variable/argument name
    my $fn = $f[2]; # filename
    my $fs = $f[3]; # function body start
    my $fe = $f[4]; # function body end
    my $fm = $f[5]; # function name

    # trim filename
    $fn =~ s/^.*\///;
    
    # read file into memory
    readfile($fn);

    # rename variable on given position
    my $n = 0;
    my @lines = splitlines($files{$fn});

    if($fs == $fe && $fm eq "ALIAS")
    {
      # we have to find the beginning first
      while($lines[$fs-1] !~ /^\s*ALIAS\s*\(/)
      {
	--$fs;
      }
    }

    $lines[$fs-1] = "#undef\t$v\n".$lines[$fs-1];
    $lines[$fe-1] .= "\n#define\t$v\t${v}__VAR";

    $files{$fn} = join("\n", @lines)."\n";
    $modified{$fn} = 1;
  }
  else
  {
    die "unknown command ".$f[0];
  }
}

writefiles();

if(scalar(keys %vars))
{
  flushvars();
}

# the end, only subroutines below this point

sub flushvars()
{
  my @g = keys %vars;

  # globalvars.c

  do_output("#include \"globalvars.h\"\n\n", $outs{'G_c'});

  do_output("struct $glob_struct_name * ${glob_struct_name}_createActiveSet()\n{\n", $outs{'G_c'});
  do_output("\tstruct $glob_struct_name * ptr = (struct $glob_struct_name *)malloc(sizeof(struct $glob_struct_name));\n", $outs{'G_c'});
  do_output("\tmemset(ptr, 0, sizeof(struct $glob_struct_name));\n", $outs{'G_c'});
  do_output("\treturn ptr;\n", $outs{'G_c'});
  do_output("}\n\n", $outs{'G_c'});

  do_output("int ${glob_struct_name}_errno()\n{\n", $outs{'G_c'});
  do_output("\treturn $glob_variable_name->errno__X;\n", $outs{'G_c'});
  do_output("}\n\n", $outs{'G_c'});

  foreach my $dir (keys %vars_init)
  {
    # generate initializing method for every directory

    # extern forwards...
     
    foreach my $key (keys %{$vars_init{$dir}})
    {
      if($vars_init{$dir}{$key} =~ /^memcpy$/)
      {
	do_output("extern ".$vars_type{$key}." ${key}_${dir}".$vars_args{$key}.";\n", $outs{'G_c'});
      }
      elsif($vars_init{$dir}{$key} =~ /{/)
      {
      }
      else
      {
      }
    }
    do_output("\n", $outs{'G_c'});

    do_output("void ${glob_struct_name}_initializeActiveSet_${dir}()\n{\n", $outs{'G_c'});

    # vars...

    foreach my $key (keys %{$vars_init{$dir}})
    {
      if($vars_init{$dir}{$key} =~ /^memcpy$/)
      {
      }
      elsif($vars_init{$dir}{$key} =~ /{/)
      {
        do_output("\t".$vars_type{$key}." ${key}__T".$vars_args{$key}." = ".$vars_init{$dir}{$key}.";\n", $outs{'G_c'});
      }
      else
      {
      }
    }

    # assignments...

    foreach my $key (keys %{$vars_init{$dir}})
    {
      if($vars_init{$dir}{$key} =~ /^memcpy$/)
      {
        do_output("\tmemcpy(&$glob_variable_name->${key}__X, &${key}_${dir}, sizeof(${key}_${dir}));\n", $outs{'G_c'});
      }
      elsif($vars_init{$dir}{$key} =~ /{/)
      {
        do_output("\tmemcpy(&$glob_variable_name->${key}__X, &${key}__T, sizeof(${key}__T));\n", $outs{'G_c'});
      }
      else
      {
        do_output("\t$glob_variable_name->${key}__X = ".$vars_init{$dir}{$key}.";\n", $outs{'G_c'});
      }
    }

    do_output("}\n\n", $outs{'G_c'});
  }

  # globalvars.h

  do_output("#ifndef\t__GLOBALVARS_H__\n", $outs{'G_h'});
  do_output("#define\t__GLOBALVARS_H__\n\n", $outs{'G_h'});

  do_output("#include \"allheaders.h\"\n\n", $outs{'G_h'});

  do_output("#undef errno\n\n", $outs{'G_h'});

  #   structure

  do_output("\nstruct $glob_struct_name\n{\n", $outs{'G_h'});
  foreach my $key (@g)
  {
    do_output("\t".$vars{$key}.";\n", $outs{'G_h'});
  }
  do_output("};\n\n", $outs{'G_h'});

  do_output("extern struct $glob_struct_name * $glob_variable_name;\n\n", $outs{'G_h'});

  do_output("#ifdef __cplusplus\n", $outs{'G_h'});
  do_output("extern \"C\" {\n", $outs{'G_h'});
  do_output("#endif\n\n", $outs{'G_h'});

  do_output("extern struct $glob_struct_name * ${glob_struct_name}_createActiveSet();\n\n", $outs{'G_h'});
  do_output("extern int ${glob_struct_name}_errno();\n\n", $outs{'G_h'});

  foreach my $dir (keys %vars_init)
  {
    do_output("void ${glob_struct_name}_initializeActiveSet_${dir}();\n\n", $outs{'G_h'});
  }
  do_output("#ifdef __cplusplus\n", $outs{'G_h'});
  do_output("};\n", $outs{'G_h'});
  do_output("#endif\n\n", $outs{'G_h'});

  #   macros

  do_output("#ifndef\t__GLOBALVARS_DEFS\n", $outs{'G_h'});
  do_output("#define\t__GLOBALVARS_DEFS\n", $outs{'G_h'});
  foreach my $key (@g)
  {
    do_output("#define	${key}__VAR  ($glob_variable_name->${key}__X)\n", $outs{'G_h'});
    do_output("#define	$key  ${key}__VAR\n", $outs{'G_h'});
  }
  do_output("#endif\n\n", $outs{'G_h'});

  do_output("#endif\n", $outs{'G_h'});

  # globalvars_on & globalvars_off

  foreach my $key (@g)
  {
    $key eq "errno" and next;

    do_output("#define $key  ${key}__VAR\n", $outs{'G_on'});
    do_output("#undef $key\n", $outs{'G_off'});
  }
}


sub do_output($$)
{
  my $x = shift; # string to output
  my $h = shift; # file handle

  if($h)
  {
    print $h $x;
  }
  else
  {
    print $x;
  }
}

sub fileexists($)
{
  my $file = shift;

  # look in subdirectories
  my @list = glob($root_dir."/*/".$file);
  @list == 1 and return 1;

  # or fully qualified
  -r $root_dir."/".$file and return 1;

  return 0;
}

sub readfiles($)
{
  my $mask = shift;

  $root_dir or die "directory must be specified in this mode";

  my @list = glob($root_dir."/*/".$mask);

  foreach my $key (@list)
  {
    $key =~ s/^.*\///;
    readfile($key);
  }
}

sub readfile($)
{
  my $file = shift;

  # do nothing if file already in memory
  defined $files{$file} and return;

  # find file on disk
  my @list;

  if($root_dir)
  {
    if($file =~ /^\//)
    {
      # absolute path, use as-is
      push @list, $file;
    }
    else
    {
      # relative path
      @list = glob($root_dir."/*/".$file);
    }
  }
  else
  {
    push @list, $file;
  }
  @list == 1 or die "troubles finding file $file (".scalar(@list).")";

  # remember file path
  $paths{$file} = $list[0];  

  # read file into memory
  open(FH, "<".$list[0]) or die "unable to read $file";
  my $bk = $/;
  undef $/;
  $files{$file} = <FH>;
  $/ = $bk;
  close(FH);
}

sub writefiles()
{
  my $suffix = $dryrun? ".new": "";

  foreach my $n (keys %modified)
  {
    my $fname = $paths{$n}.$suffix;
    open(FH,">$fname") or die "unable to write $fname";    
    print FH $files{$n};
    close(FH);
  }
}

sub decode($)
{
  my $str = shift;
  $str =~ s/%([A-Fa-f\d]{2})/chr hex $1/eg;
  return $str;
}

sub splitlines($)
{
  my $text = shift;

  my @lines = split(/\n/, $text);

  # handle inserted lines (prepend them to previous) to make
  # line indexing work as expected 

  for(my $i = $#lines; $i > 0; $i--)
  {
    if($lines[$i] =~ /^#define\s+([\w_]+)\s+([\w_]+)__VAR$/)
    {
      my $var = $1;

      $var eq $2 or die "this is strange ($1 and $2)... ".$lines[$i];

      # append to previous
      $lines[$i-1] .= "\n".$lines[$i];

      # remove from array
      splice @lines, $i, 1;

      # find corresponding undef
      do
      {
	--$i;
      } while($i >= 0 && $lines[$i] !~ /^#undef\s+$var$/);

      $i >= 0 or die "#undef $var not found";

      # prepend to next
      $lines[$i+1] = $lines[$i] . "\n" . $lines[$i+1];

      # remove from array
      splice @lines, $i, 1;
    }
  }

  return @lines;
}

sub readinputfiles()
{
    if($root_dir)
    {
      readfiles("*.c");
      readfiles("*.h");
    }
    else
    {
      readfile($file_src);
    }
}

