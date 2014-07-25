//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
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

#include <algorithm>
#include <ctype.h>

#include "inet/applications/httptools/common/HttpUtils.h"

namespace inet {

namespace httptools {

inline bool isnotspace(int c) { return !isspace(c); }

std::string trimLeft(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), isnotspace));
    return s;
}

std::string trimLeft(std::string str, std::string delim)
{
    int pos = str.find(delim);
    return pos == -1 ? str : str.substr(pos + 1, str.size() - pos - 1);
}

std::string trimRight(std::string s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), isnotspace).base(), s.end());
    return s;
}

std::string trimRight(std::string str, std::string delim)
{
    int pos = str.rfind(delim);
    return pos == -1 ? str : str.substr(0, pos - 1);
}

std::string trim(std::string str)
{
    str = trimLeft(str);
    str = trimRight(str);
    return str;
}

std::string extractServerName(const char *url)
{
    std::string str(url);
    int position = str.find("http://");
    if (position != -1) {
        str = str.erase(0, position);
    }
    else {
        position = str.find("https://");
        if (position != -1) {
            str = str.erase(0, position);
        }
    }

    position = str.find("/");
    if (position != -1) {
        str = str.substr(0, position);
    }

    return str;
}

std::string extractResourceName(const char *url)
{
    std::string str(url);
    int position = str.find("http://");
    if (position != -1) {
        str = str.erase(0, position);
    }
    else {
        position = str.find("https://");
        if (position != -1) {
            str = str.erase(0, position);
        }
    }

    position = str.find("/");
    if (position != -1)
        return str.substr(position + 1, str.size() - position);
    else
        return "";
}

std::vector<std::string> parseResourceName(std::string resource)
{
    std::string path = "";
    std::string resourceName = "";
    std::string extension = "";

    int slashpos = resource.rfind("/");
    if (slashpos != -1)
        path = resource.substr(0, slashpos);
    int dotpos = resource.rfind(".");
    if (dotpos != -1) {
        resourceName = resource.substr(slashpos + 1, dotpos - slashpos - 1);
        extension = resource.substr(dotpos + 1, resource.size() - dotpos - 1);
    }
    else {
        resourceName = resource.substr(slashpos + 1, resource.size() - slashpos);
    }

    std::vector<std::string> res;
    res.push_back(path);
    res.push_back(resourceName);
    res.push_back(extension);
    return res;
}

std::string getDelimited(std::string str, std::string ldelim, std::string rdelim)
{
    int lpos = str.find(ldelim);
    int rpos;
    if (rdelim == "")
        rpos = str.rfind(ldelim);
    else
        rpos = str.rfind(rdelim);
    if (lpos == -1 || rpos == -1 || lpos == rpos)
        return ""; // Not found
    else
        return str.substr(lpos + 1, rpos - lpos - 1);
}

HttpContentType getResourceCategory(std::vector<std::string> res)
{
    if (res.size() == 2)
        return CT_HTML;
    else if (res.size() > 2)
        return getResourceCategory(res[2]); // get the category from the extension
    return CT_UNKNOWN;
}

HttpContentType getResourceCategory(std::string resourceExt)
{
    if (resourceExt == "" || resourceExt == "htm" || resourceExt == "html")
        return CT_HTML;
    else if (resourceExt == "jpg" || resourceExt == "gif" || resourceExt == "png" || resourceExt == "bmp")
        return CT_IMAGE;
    else if (resourceExt == "css" || resourceExt == "txt" || resourceExt == "js")
        return CT_TEXT;
    return CT_UNKNOWN;
}

std::string htmlErrFromCode(int code)
{
    switch (code) {
        case 200:
            return "OK";

        case 400:
            return "ERROR";

        case 404:
            return "NOT FOUND";

        default:
            return "???";
    }
}

double safeatof(const char *strval, double defaultVal)
{
    try {
        return atof(strval);
    }
    catch (...) {
        return defaultVal;
    }
}

int safeatoi(const char *strval, int defaultVal)
{
    try {
        return atoi(strval);
    }
    catch (...) {
        return defaultVal;
    }
}

int safeatobool(const char *strval, bool defaultVal)
{
    try {
        return strcmp(strval, "TRUE") == 0 || strcmp(strval, "true") == 0;
    }
    catch (...) {
        return defaultVal;
    }
}

std::vector<std::string> splitFile(std::string fileName)
{
    std::string path = "";
    std::string file = "";
    std::string ext = "";

    int slashpos = fileName.rfind("/");
    if (slashpos == -1)
        slashpos = fileName.rfind("\\");
    if (slashpos != -1) {
        path = fileName.substr(0, slashpos + 1);
        fileName = fileName.substr(slashpos + 1, fileName.size() - slashpos - 1);
    }

    int dotpos = fileName.find(".");
    if (dotpos != -1) {
        ext = fileName.substr(dotpos + 1, fileName.size() - dotpos - 1);
        file = fileName.substr(0, dotpos);
    }
    else {
        file = fileName;
    }

    std::vector<std::string> res;
    res.push_back(path);
    res.push_back(file);
    res.push_back(ext);
    return res;
}

} // namespace httptools

} // namespace inet

