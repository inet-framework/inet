//
//......
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

template <class T> void createVectorWatcher(const char *varname, vector<T>& v)
{
    new cVectorWatcher<T>(varname, v);
}


//
// Internal class
//
template<class T>
class cPointerVectorWatcher : public cVectorWatcher<T>
{
  public:
    cPointerVectorWatcher(const char *name, vector<T>& var) : cVectorWatcher<T>(name, var) {}
    virtual string at(int i) const {stringstream out; out << *v[i]; return out.str();}
};

template <class T> void createPointerVectorWatcher(const char *varname, vector<T>& v)
{
    new cPointerVectorWatcher<T>(varname, v);
}

#define WATCH_VECTOR(v)   createVectorWatcher(#v,(v))

#define WATCH_PTRVECTOR(v)   createPointerVectorWatcher(#v,(v))


#endif

