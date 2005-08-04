//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "opp_utils.h"

namespace OPP_Global
{

std::string ltostr(long i)
{
    std::ostringstream os;
    os << i;
    return os.str();
}

std::string dtostr(double d)
{
    std::ostringstream os;
    os << d;
    return os.str();
}

double atod(const char *s)
{
    char *e;
    double d = ::strtod(s,&e);
    if (*e)
        throw new cException("invalid cast: '%s' cannot be interpreted as a double", s);
    return d;
}

unsigned long atoul(const char *s)
{
    char *e;
    unsigned long d = ::strtoul(s,&e,10);
    if (*e)
        throw new cException("invalid cast: '%s' cannot be interpreted as an unsigned long", s);
    return d;
}

}
