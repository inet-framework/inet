//
//......
//


#ifndef _WATCH2_H__
#define _WATCH2_H__

#include <iostream>
#include <sstream>
#include <omnetpp.h>


template<class T>
class cWatch2 : public cObject
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
    virtual void info(char *buf)
    {
        std::stringstream out;
        out << className() << " " << name() << " = " << r;
        strcpy(buf,out.str().c_str());
    }
    //virtual cStructDescriptor *createDescriptor();
};

class cWatch2_stdstring : public cObject
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
    void info(char *buf)
    {
        std::stringstream out;
        out << className() << " " << name() << " = \"" << r << "\"";
        strcpy(buf,out.str().c_str());
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


