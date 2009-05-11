// $Id$
//------------------------------------------------------------------------------
//	LambdaSplitter.h --
//
//	This file declares the 'LambdaSplitter' modelling an AWG
//  in the path of hybrid TDM/WDM-PON.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


#ifndef __LAMBDA_SPLITTER_H
#define __LAMBDA_SPLITTER_H


#include <omnetpp.h>
#include "HybridPon.h"
#include "OpticalFrame_m.h"


class LambdaSplitter: public cSimpleModule {
	int numPorts;
/* 	string distances; */
/* 	TimeVector RTT; */

	virtual void initialize(void);
	virtual void handleMessage(cMessage *msg);
	virtual void finish(void);
};


#endif  // __LAMBDA_SPLITTER_H
