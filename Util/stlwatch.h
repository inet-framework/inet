//
//......
//


#ifndef _STLWATCH_H__
#define _STLWATCH_H__

#include <omnetpp.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>


//
// Internal class
//
class cVectorWatcherBase : public cObject
{
  public:
    cVectorWatcherBase(const char *name) : cObject(name) {}

    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    virtual const char *elemTypeName() const = 0;
    virtual int size() const = 0;
    virtual std::string at(int i) const = 0;
    virtual cStructDescriptor *createDescriptor();
};


//
// Internal class
//
template<class T>
class cVectorWatcher : public cVectorWatcherBase
{
  protected:
    std::vector<T>& v;
  public:
    cVectorWatcher(const char *name, std::vector<T>& var) : cVectorWatcherBase(name), v(var) {}
    const char *className() const {return opp_typename(typeid(v));}
    virtual const char *elemTypeName() const {return opp_typename(typeid(T));}
    virtual int size() const {return v.size();}
    virtual std::string at(int i) const {std::stringstream out; out << v[i]; return out.str();}
};

template <class T>
void createVectorWatcher(const char *varname, std::vector<T>& v)
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
    cPointerVectorWatcher(const char *name, std::vector<T>& var) : cVectorWatcher<T>(name, var) {}
    virtual std::string at(int i) const {std::stringstream out; out << *(this->v[i]); return out.str();}
};

template <class T>
void createPointerVectorWatcher(const char *varname, std::vector<T>& v)
{
    new cPointerVectorWatcher<T>(varname, v);
}

//
// Internal class
//
template<class _K, class _V, class _C>
class cMapWatcher : public cVectorWatcherBase
{
  protected:
    std::map<_K,_V,_C>& m;
    mutable typename std::map<_K,_V,_C>::iterator it;
    mutable int itPos;
  public:
    cMapWatcher(const char *name, std::map<_K,_V,_C>& var) : cVectorWatcherBase(name), m(var) {itPos=-1;}
    const char *className() const {return opp_typename(typeid(m));}
    virtual const char *elemTypeName() const {return "struct pair<...,...>";}
    virtual int size() const {return m.size();}
    virtual std::string at(int i) const {
        // std::map doesn't support random access iterator and iteration is slow,
        // so we have to use a trick, knowing that Tkenv will call this function with
        // i=0, i=1, etc...
        if (i==0) {
            it=m.begin(); itPos=0;
        } else if (i==itPos+1 && it!=m.end()) {
            ++it; ++itPos;
        } else {
            it=m.begin();
            for (int k=0; k<i && it!=m.end(); k++) ++it;
            itPos=i;
        }
        if (it==m.end()) {
            return std::string("out of bounds");
        }
        return atIt();
    }
    virtual std::string atIt() const {
        std::stringstream out;
        out << "{" << it->first << "}  ==>  {" << it->second << "}";
        return out.str();
    }
};

template <class _K, class _V, class _C>
void createMapWatcher(const char *varname, std::map<_K,_V,_C>& m)
{
    new cMapWatcher<_K,_V,_C>(varname, m);
}


//
// Internal class
//
template<class _K, class _V, class _C>
class cPointerMapWatcher : public cMapWatcher<_K,_V,_C>
{
  public:
    cPointerMapWatcher(const char *name, std::map<_K,_V,_C>& var) : cMapWatcher<_K,_V,_C>(name, var) {}
    virtual std::string atIt() const {
        std::stringstream out;
        out << "{" << this->it->first << "}  ==>  {" << *(this->it->second) << "}";
        return out.str();
    }
};

template<class _K, class _V, class _C>
void createPointerMapWatcher(const char *varname, std::map<_K,_V,_C>& m)
{
    new cPointerMapWatcher<_K,_V,_C>(varname, m);
}

#define WATCH_VECTOR(v)      createVectorWatcher(#v,(v))

#define WATCH_PTRVECTOR(v)   createPointerVectorWatcher(#v,(v))

#define WATCH_MAP(m)         createMapWatcher(#m,(m))

#define WATCH_PTRMAP(m)      createPointerMapWatcher(#m,(m))

#endif

