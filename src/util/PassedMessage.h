/* -*- mode:c++ -*- */

#ifndef PASSED_MESSAGE_H
#define PASSED_MESSAGE_H

#include <omnetpp.h>

#include "INETDefs.h"

class INET_API PassedMessage : public cObject {
 public:
    enum gates_t {
        UPPER_DATA,
	UPPER_CONTROL,
	LOWER_DATA,
	LOWER_CONTROL
    };

    enum direction_t {
        INCOMING,
        OUTGOING
    };
    
 public:
    static const char *gateToString(gates_t g) {
        const char *s;
        switch(g) {
        case UPPER_DATA: s = "UPPER_DATA"; break;
        case UPPER_CONTROL: s = "UPPER_CONTROL"; break;
        case LOWER_DATA: s = "LOWER_DATA"; break;
        case LOWER_CONTROL: s = "LOWER_CONTROL"; break;
        default:
            opp_error("PassedMessage::gateToString: got invalid value");
            s = 0;
            break;
        }
        return s;
    }
    
 public:
    // meta information 
    int fromModule;
    gates_t gateType;
    direction_t direction;

    // message information
    int kind;
    const char* name;

    PassedMessage()
    	: cObject()
    	, fromModule(-1)
    	, gateType(UPPER_CONTROL)
    	, direction(INCOMING)
    	, kind(0)
    	, name(NULL)
    {}
    PassedMessage(const PassedMessage& o)
    	: cObject(o)
    	, fromModule(o.fromModule)
    	, gateType(o.gateType)
    	, direction(o.direction)
    	, kind(o.kind)
    	, name(o.name)
    {}

    PassedMessage& operator=(PassedMessage const& copy)
    {
    	PassedMessage tmp(copy); // All resource all allocation happens here.
	                         // If this fails the copy will throw an exception
                                // and 'this' object is unaffected by the exception.
        swap(tmp);
        return *this;
    }
    // swap is usually trivial to implement
    // and you should easily be able to provide the no-throw guarantee.
    void swap(PassedMessage& s)
    {
        // cObject::swap(s); // swap the base class members
        /* Swap all PassedMessage members */
        std::swap(fromModule, s.fromModule);
        std::swap(gateType,   s.gateType);
        std::swap(direction,  s.direction);
        std::swap(kind,       s.kind);
        std::swap(name,       s.name);
    }
};

#endif

