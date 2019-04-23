#ifndef __PACKING_H
#define __PACKING_H

#include <vector>
#include <list>
#include <set>
#include <map>
#include <omnetpp.h>

namespace inet {

namespace icancloud {




void doPacking (cCommBuffer*& buffer, int& number){

}

void doUnpacking (cCommBuffer*& buffer, int& number){

}


//
// Packing/unpacking an std::vector
//
template<typename T, typename A>
void doPacking (cCommBuffer *buffer, /*const*/ std::vector<T,A>& v){
    doPacking(buffer, (int)v.size());
    for (int i=0; i<v.size(); i++)
        doPacking(buffer, v[i]);
}

template<typename T, typename A>
void doUnpacking(cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doUnpacking(buffer, n);
    v.resize(n);
    for (int i=0; i<n; i++)
        doUnpacking(buffer, v[i]);
}


//
// Packing/unpacking an std::list
//
template<typename T, typename A>
void doPacking(cCommBuffer *buffer, /*const*/ std::list<T,A>& l){

    doPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it=l.begin(); it != l.end(); it++)
        doPacking(buffer, *it);
}

template<typename T, typename A>
void doUnpacking(cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doUnpacking(buffer, l.back());
    }
}

//
// Packing/unpacking an std::set
//
template<typename T, typename Tr, typename A>
void doPacking(cCommBuffer *buffer, /*const*/ std::set<T,Tr,A>& s)
{
    doPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); it++)
        doPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doUnpacking(cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doUnpacking(buffer, x);
        s.insert(x);
    }
}


//
// Packing/unpacking an std::map
//
template<typename K, typename V, typename Tr, typename A>
void doPacking(cCommBuffer *buffer, /*const*/ std::map<K,V,Tr,A>& m)
{
    doPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); it++) {
        doPacking(buffer, it->first);
        doPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doUnpacking(cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doUnpacking(buffer, k);
        doUnpacking(buffer, v);
        m[k] = v;
    }
}


} // namespace icancloud
} // namespace inet

#endif
