/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Björn Wiberg <bjorn.wiberg@home.se>
 *
 *****************************************************************************/

/*
  This code was partially borrowed from Per Kristian Hove
  <Per.Hove@math.ntnu.no>, who originally posted it to the rdesktop
  mailing list at rdesktop.org.

  It solves the problem of non-existent <endian.h> on some systems by
  generating it.

  (Compile, run, and redirect the output to endian.h.)
*/

#include <stdio.h>

int litend(void)
{
    int i = 0;
    ((char *) (&i))[0] = 1;
    return (i == 1);
}

int bigend(void)
{
    return !litend();
}

int main(int argc, char **argv)
{
    printf("#ifndef ENDIAN_H\n");
    printf("#define ENDIAN_H\n");
    printf("#define __LITTLE_ENDIAN 1234\n");
    printf("#define __BIG_ENDIAN    4321\n");
    printf("#define __BYTE_ORDER __%s_ENDIAN\n", litend()? "LITTLE" : "BIG");
    printf("#endif\n");
    return 0;
}
