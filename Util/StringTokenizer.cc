/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include <assert.h>
#include <stdlib.h>
#include "StringTokenizer.h"


bool StringTokenizer::active = false;

StringTokenizer::StringTokenizer(const char *s, const char *delim)
{
    assert(!active);
    active = true;
    delimiter = delim;
    bool atFirstToken = true;
    str = new char[strlen(s)+1];
    strcpy(str,s);
}

StringTokenizer::~StringTokenizer()
{
    active = false;
    delete str;
}

void StringTokenizer::setDelimiter(const char *delim)
{
    delimiter = delim;
}

const char *StringTokenizer::nextToken()
{
    if (atFirstToken)
    {
        atFirstToken = false;
        return strtok(str, delimiter.c_str());
    }
    return strtok(NULL, delimiter.c_str());
}



