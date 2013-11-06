#!/usr/bin/perl

while ($sor = <STDIN>)
{
    if ($sor eq "\n")
    {
        last;
    }
}

while ($sor = <STDIN>)
{
    $sor =~ s/\.mac \t/ \t/g;
    print $sor;
}
