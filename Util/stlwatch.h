//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
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


#ifndef _STLWATCH_H__
#define _STLWATCH_H__

#include <omnetpp.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;


//
// Internal class
//
class cVectorWatcherBase : public cObject
{
  public:
    cVectorWatcherBase(const char *name) : cObject(name) {}

    virtual void info(char *buf);
    virtual string detailedInfo() const;

    virtual const char *elemTypeName() const = 0;
    virtual int size() const = 0;
    virtual string at(int i) const = 0;
    virtual cStructDescriptor *createDescriptor();
};


//
// Internal class
//
template<class T>
class cVectorWatcher : public cVectorWatcherBase
{
  protected:
    vector<T>& v;
  public:
    cVectorWatcher(const char *name, vector<T>& var) : cVectorWatcherBase(name), v(var) {}
    virtual const char *elemTypeName() const {return opp_typename(typeid(T));}
    virtual int size() const {return v.size();}
    virtual string at(int i) const {stringstream out; out << v[i]; return out.str();}
};

template <class T> createVectorWatcher(const char *varname, vector<T>& v)
{
    new cVectorWatcher<T>(varname, v);
}

#define WATCH_vector(v)   createVectorWatcher(#v,(v))


#endif

