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

#ifndef __STRING_TOKENIZER_H__
#define __STRING_TOKENIZER_H__

#include <string>
#include <vector>

/**
 * String tokenizer class, based on strtok()
 */
class StringTokenizer
{
  private:
    char *str;
    std::string delimiter;
    bool atFirstToken;
    static bool active;
  public:
    StringTokenizer(const char *s, const char *delim=" ");
    ~StringTokenizer();
    void setDelimiter(const char *s);
    const char *nextToken();
    std::vector<std::string> asVector();
};

#endif

