// ***************************************************************************
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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
// ***************************************************************************

#ifndef __httpUtils_H_
#define __httpUtils_H_

#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
# include <io.h>
# include <stdio.h>
#else
# include <unistd.h>
#endif


/** @brief type of returned content. Used in http responses. */
enum CONTENT_TYPE_ENUM {rt_unknown, rt_html_page, rt_image, rt_text};

std::string trimLeft( std::string str );
std::string trimRight( std::string str );
std::string trimLeft( std::string str, std::string delim );
std::string trimRight( std::string str, std::string delim );
std::string trim( std::string str );
std::string extractServerName( const char *path );
std::string extractResourceName( const char *path );
std::string getDelimited(std::string str, std::string ldelim, std::string rdelim = "");
std::vector<std::string> parseResourceName(std::string resource);
CONTENT_TYPE_ENUM getResourceCategory(std::vector<std::string> res);
CONTENT_TYPE_ENUM getResourceCategory(std::string resourceExt);
std::string htmlErrFromCode(int code);
double safeatof(const char* strval, double defaultVal = 0.0);
int safeatoi(const char* strval, int defaultVal = 0);
int safeatobool(const char* strval, bool defaultVal = false);
std::vector<std::string> splitFile(std::string fileName);
bool fileExists( const char *file );

#endif /* __httpUtils_H_ */
