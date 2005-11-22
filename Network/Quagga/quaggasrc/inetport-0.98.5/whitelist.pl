#!/usr/bin/perl

use strict;

$#ARGV == 0 or die "usage: $0 whitelist_file\n";

# read whitelits into memory

my %white;

open(FH, $ARGV[0]) or die;
while(my $line = <FH>)
{
  chomp $line;
  $white{$line} = 1;
}
close(FH);

# filter out whitelisted items

my %seen;

while(my $line = <STDIN>)
{
  chomp $line;

  # skip duplicates
  $seen{$line} and next;
  $seen{$line} = 1;

  my @words = split(/;/, $line);

  ## skip cmd_element
  #($words[4] eq "struct cmd_element") and next;

  ## skip cmd_node
  #($words[4] eq "struct cmd_node") and next;

  ## skip route_map_rule_cmd
  #($words[4] eq "struct route_map_rule_cmd") and next;

  # skip whitelisted
  my $match = 0;
  foreach my $key (keys %white)
  {
    if($words[1] =~ /^$key$/)
    {
      $match = 1;
      last;
    }
  }
  $match and next;

  # output variable
  print $line . "\n";
}
