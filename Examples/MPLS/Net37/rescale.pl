# transform p=<x>,<y> coordinates in display strings
$mx=0.8;
$my=0.6;
while (<>) {
   s!p=(\d+),(\d+)!"p=".int($1*$mx).",".int($2*$my)!e;
   print;
}