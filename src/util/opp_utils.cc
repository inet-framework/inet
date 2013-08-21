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

#if OMNETPP_VERSION < 0x0403
inline char *nextToken(char *&rest)
{
    if (!rest) return NULL;
    char *token = rest;
    rest = strchr(rest, '.');
    if (rest) *rest++ = '\0';
    return token;
}
cModule *getModuleByPath(cModule *startModule, const char *path)
{
    if (!path || !path[0])
        return NULL;

    // determine starting point
    bool isrelative = (path[0] == '.' || path[0] == '^');
    cModule *modp = isrelative ? startModule : simulation.getSystemModule();
    if (path[0] == '.')
        path++; // skip initial dot

    // match components of the path
    opp_string pathbuf(path);
    char *rest = pathbuf.buffer();
    char *token = nextToken(rest);
    bool isfirst = true;
    while (token && modp)
    {
        char *lbracket;
        if (!token[0])
            ; /*skip empty path component*/
        else if (!isrelative && isfirst && modp->isName(token))
            /*ignore network name*/;
        else if (token[0] == '^' && token[1] == '\0')
            modp = modp->getParentModule(); // if modp is the root, return NULL
        else if ((lbracket=strchr(token,'[')) == NULL)
            modp = modp->getSubmodule(token);
        else
        {
            if (token[strlen(token)-1] != ']')
                throw cRuntimeError(startModule, "getModuleByPath(): syntax error (unmatched bracket?) in path `%s'", path);
            int index = atoi(lbracket+1);
            *lbracket = '\0'; // cut off [index]
            modp = modp->getSubmodule(token, index);
        }
        token = nextToken(rest);
        isfirst = false;
    }

    return modp;  // NULL if not found
}
#endif


}
