//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_HTTPUTILS_H
#define __INET_HTTPUTILS_H

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#ifdef _WIN32
# include <io.h>
# include <stdio.h>
#else // ifdef _WIN32
# include <unistd.h>
#endif // ifdef _WIN32

#include "inet/applications/httptools/common/HttpMessages_m.h"    // for HttpContentType

namespace inet {

namespace httptools {

std::string trimLeft(std::string str);
std::string trimRight(std::string str);
std::string trimLeft(std::string str, std::string delim);
std::string trimRight(std::string str, std::string delim);
std::string trim(std::string str);
std::string extractServerName(const char *url);
std::string extractResourceName(const char *url);
std::string getDelimited(std::string str, std::string ldelim, std::string rdelim = "");
std::vector<std::string> parseResourceName(std::string resource);
HttpContentType getResourceCategory(std::vector<std::string> res);
HttpContentType getResourceCategory(std::string resourceExt);
std::string htmlErrFromCode(int code);

double safeatof(const char *strval, double defaultVal = 0.0);
int safeatoi(const char *strval, int defaultVal = 0);
int safeatobool(const char *strval, bool defaultVal = false);
std::vector<std::string> splitFile(std::string fileName);

} // namespace httptools

} // namespace inet

#endif

