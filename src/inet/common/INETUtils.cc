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

#include "inet/common/INETUtils.h"

namespace inet {

namespace utils {

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

std::string hex(uint16_t l)
{
    std::ostringstream os;
    os << std::hex << l;
    return os.str();
}

std::string hex(int16_t l)
{
    std::ostringstream os;
    os << std::hex << l;
    return os.str();
}

std::string hex(uint32_t l)
{
    std::ostringstream os;
    os << std::hex << l;
    return os.str();
}

std::string hex(int32_t l)
{
    std::ostringstream os;
    os << std::hex << l;
    return os.str();
}

std::string hex(uint64_t l)
{
    std::ostringstream os;
    os << std::hex << l;
    return os.str();
}

std::string hex(int64_t l)
{
    std::ostringstream os;
    os << std::hex << l;
    return os.str();
}

long hex(const char *s)
{
    return strtol(s, nullptr, 16);
}

unsigned long uhex(const char *s)
{
    return strtoul(s, nullptr, 16);
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
    for ( ; *s; s++)
        if (isalnum(*s))
            result += *s;

    return result;
}

#define BUFLEN    1024

std::string stringf(const char *fmt, ...)
{
    char buf[BUFLEN];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, BUFLEN, fmt, va);
    buf[BUFLEN - 1] = '\0';
    va_end(va);
    return buf;
}

std::string vstringf(const char *fmt, va_list& args)
{
    char buf[BUFLEN];
    vsnprintf(buf, BUFLEN, fmt, args);
    buf[BUFLEN - 1] = '\0';
    return buf;
}

#undef BUFLEN

cObject *createOneIfClassIsKnown(const char *className, const char *defaultNamespace)
{
    std::string ns = defaultNamespace;
    do {
        std::string::size_type found = ns.rfind("::");
        if (found == std::string::npos)
            found = 0;
        ns.erase(found);
        std::string cn = found ? ns + "::" + className : className;
        cObject *ret = cObjectFactory::createOneIfClassIsKnown(cn.c_str());
        if (ret)
            return ret;
    } while (!ns.empty());
    return nullptr;
}

cObject *createOne(const char *className, const char *defaultNamespace)
{
    cObject *ret = createOneIfClassIsKnown(className, defaultNamespace);
    if (!ret)
        throw cRuntimeError("Class \"%s\" not found -- perhaps its code was not linked in, "
                            "or the class wasn't registered with Register_Class(), or in the case of "
                            "modules and channels, with Define_Module()/Define_Channel()", className);
    return ret;
}

bool fileExists(const char *pathname)
{
    // Note: stat("foo/") ==> error, even when "foo" exists and is a directory!
    struct stat statbuf;
    return stat(pathname, &statbuf) == 0;
}

void splitFileName(const char *pathname, std::string& dir, std::string& fnameonly)
{
    if (!pathname || !*pathname) {
        dir = ".";
        fnameonly = "";
        return;
    }

    // find last "/" or "\"
    const char *s = pathname + strlen(pathname) - 1;
    s--;  // ignore potential trailing "/"
    while (s > pathname && *s != '\\' && *s != '/')
        s--;
    const char *sep = s <= pathname ? nullptr : s;

    // split along that
    if (!sep) {
        // no slash or colon
        if (strchr(pathname, ':') || strcmp(pathname, ".") == 0 || strcmp(pathname, "..") == 0) {
            fnameonly = "";
            dir = pathname;
        }
        else {
            fnameonly = pathname;
            dir = ".";
        }
    }
    else {
        fnameonly = s+1;
        dir = std::string(pathname, s-pathname+1);
    }
}

void makePath(const char *pathname)
{
    if (!fileExists(pathname)) {
        std::string pathprefix, dummy;
        splitFileName(pathname, pathprefix, dummy);
        makePath(pathprefix.c_str());
        // note: anomaly with slash-terminated dirnames: stat("foo/") says
        // it does not exist, and mkdir("foo/") says cannot create (EEXIST):
        if (opp_mkdir(pathname, 0755) != 0 && errno != EEXIST)
            throw cRuntimeError("Cannot create directory '%s': %s", pathname, strerror(errno));
    }
}

void makePathForFile(const char *filename)
{
    std::string pathprefix, dummy;
    splitFileName(filename, pathprefix, dummy);
    makePath(pathprefix.c_str());
}

} // namespace utils

} // namespace inet

