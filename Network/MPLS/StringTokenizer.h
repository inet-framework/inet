/*******************************************************************
*
*	This library is free software, you can redistribute it 
*	and/or modify 
*	it under  the terms of the GNU Lesser General Public License 
*	as published by the Free Software Foundation; 
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful, 
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#ifndef STRING_TOKENIZER_H
#define STRING_TOKENIZER_H

#include <string>

class StringTokenizer
{
private:
std::string inputStr;
std::string delimiter;
char *s;
const char* delim;
char * token;
int tokenLength;
public:
StringTokenizer(std::string inString, std::string delimiter);
std::string *nextToken();
};

#endif //STRING_TOKENIZER_H

