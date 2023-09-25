s/ +$//g;
s/\bNULL\b/nullptr/g;
s/\%(\d*")([SUX]16_F)"/%\1 \2 "/g;
s/\%(\d*")([SUX]32_F)"/%\1 \2 "/g;
s/\(void\)/()/g;
