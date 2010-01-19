// $Id$
//------------------------------------------------------------------------------
//	Splitter.h --
//
//	This file declares the 'Splitter' class for use in the ONUs
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


#ifndef __SPLITTER_H
#define __SPLITTER_H


#include <omnetpp.h>
#include "HybridPon.h"


class Splitter : public cSimpleModule
{
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};


#endif  // __SPLITTER_H
