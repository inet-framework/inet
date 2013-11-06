#!/usr/bin/perl

$id = '__HEADER__';
$out{$id} = '';

while ($sor = <STDIN>)
{
    if ($sor =~ /^scalar / or $sor =~ /^statistic /)
    {
        $id = $sor;
        $out{$id} = $sor;
    }
    else
    {
        $out{$id} .= $sor;
    }
}

foreach $key (sort (keys(%out)))
{
   print $out{$key};
}

