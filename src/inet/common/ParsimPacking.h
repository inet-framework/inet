//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PARSIMPACKING_H
#define __INET_PARSIMPACKING_H

#include <type_traits>
#include <omnetpp.h>

namespace omnetpp {

//
// signed char is a distinct type from char in C++, but OMNeT++ packing.h
// only provides doParsimPacking for char and unsigned char.
//
inline void doParsimPacking(omnetpp::cCommBuffer *b, signed char a) { b->pack((char)a); }
inline void doParsimPacking(omnetpp::cCommBuffer *b, const signed char *a, int n) { b->pack((const char *)a, n); }
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, signed char& a) { b->unpack((char&)a); }
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, signed char *a, int n) { b->unpack((char *)a, n); }

//
// Generic packing/unpacking for pointers to cObject subclasses.
// Used by generated code for owned object pointer fields (e.g., Ipv6NdOption *option[]).
//
template<typename T, typename std::enable_if<std::is_base_of<cObject, T>::value, int>::type = 0>
void doParsimPacking(omnetpp::cCommBuffer *b, T *obj)
{
    if (obj == nullptr) {
        b->pack("");
    } else {
        b->pack(obj->getClassName());
        obj->parsimPack(b);
    }
}

template<typename T, typename std::enable_if<std::is_base_of<cObject, T>::value && !std::is_const<T>::value, int>::type = 0>
void doParsimUnpacking(omnetpp::cCommBuffer *b, T *&obj)
{
    opp_string className;
    b->unpack(className);
    if (className.empty()) {
        obj = nullptr;
    } else {
        cObject *tmp = createOneIfClassIsKnown(className.c_str());
        if (!tmp)
            throw cRuntimeError("Parsim error: Cannot create object of class '%s'", className.c_str());
        obj = check_and_cast<T *>(tmp);
        obj->parsimUnpack(b);
    }
}

}  // namespace omnetpp

#endif
