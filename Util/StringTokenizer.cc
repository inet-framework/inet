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

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <assert.h>
#include <stdlib.h>
#include "StringTokenizer.h"


StringTokenizer::StringTokenizer(const char *s, const char *delim)
{
    delimiter = delim;
    str = new char[strlen(s)+1];
    strcpy(str,s);
    strend = str+strlen(str);
    rest = str;
}

StringTokenizer::~StringTokenizer()
{
    delete str;
}

void StringTokenizer::setDelimiter(const char *delim)
{
    delimiter = delim;
}

const char *StringTokenizer::nextToken()
{
    if (!rest)
        return NULL;
    char *token = strtok(rest, delimiter.c_str());
    rest = token ? token+strlen(token)+1 : NULL;
    if (rest && rest>=strend)
        rest = NULL;
    return token;

}

std::vector<std::string> StringTokenizer::asVector()
{
    const char *s;
    std::vector<std::string> v;
    while ((s=nextToken())!=NULL)
        v.push_back(std::string(s));
    return v;
}


