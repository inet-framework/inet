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
#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(std::string inString, std::string delimiter)
{
this->inputStr = inString;
this->delimiter = delimiter;


s = new char[inputStr.length() + 1];
inputStr.copy(s,std::string::npos); //npos denotes end of string
// add terminator null character (reqd. for C-style strings)
s[inputStr.length()] = 0;

// get a constant reference to a c-style string for delimiters
delim = delimiter.c_str();
// get the first token (s gets modified by strtok()
token = strtok(s, delim);
}

std::string *StringTokenizer::nextToken()
{
if (token == (char *) NULL) return NULL;
tokenLength = strlen(token);
std::string *next = new std::string(token, tokenLength);
token = strtok(NULL, delim);
return next;
}


