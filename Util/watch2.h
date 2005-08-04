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

#ifndef _WATCH2_H__
#define _WATCH2_H__

#include <iostream>
#include <sstream>
#include <omnetpp.h>
#include "INETDefs.h"


template<class T>
class INET_API cWatch2 : public cObject
{
  private:
    const T& r;
  public:
    cWatch2(const char *name, const T& x) : cObject(name), r(x)
    {
    }
    virtual const char *className() const
    {
        return opp_typename(typeid(T));
    }
    virtual std::string info()
    {
        std::stringstream out;
        out << className() << " " << name() << " = " << r;
        return out.str();
    }
    //virtual cStructDescriptor *createDescriptor();
};

class INET_API cWatch2_stdstring : public cObject
{
  private:
    const std::string& r;
  public:
    cWatch2_stdstring(const char *name, const std::string& x) : cObject(name), r(x)
    {
    }
    virtual const char *className() const
    {
        return "std::string";
    }
    virtual std::string info()
    {
        std::stringstream out;
        out << className() << " " << name() << " = \"" << r << "\"";
        return out.str();
    }
};


template <class T> cObject *createWatch2(const char *varname, const T& v) {
    return new cWatch2<T>(varname, v);
}

cObject *createWatch2(const char *varname, const std::string& v) {
    return new cWatch2_stdstring(varname, v);
}

#define WATCH2(v)   createWatch2(#v,(v))


#endif


