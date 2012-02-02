//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
    double d = ::strtod(s, &e);
    if (*e)
        throw cRuntimeError("Invalid cast: '%s' cannot be interpreted as a double", s);
    return d;
}

unsigned long atoul(const char *s)
{
    char *e;
    unsigned long d = ::strtoul(s, &e, 10);
    if (*e)
        throw cRuntimeError("Invalid cast: '%s' cannot be interpreted as an unsigned long", s);
    return d;
}

std::string stripnonalnum(const char *s)
{
    std::string result;
    for (; *s; s++)
        if (isalnum(*s))
            result += *s;
    return result;
}


}
