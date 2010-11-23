///
/// @file   LambdaSplitter.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-02-25
///
/// @brief  Declares LambdaSplitter class, modelling AWG in a hybrid TDM/WDM-PON.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __LAMBDA_SPLITTER_H
#define __LAMBDA_SPLITTER_H

#include "HybridPon.h"

class LambdaSplitter: public cSimpleModule
{
protected:
	int numPorts;

protected:
	// OMNeT++
	virtual void initialize(void);
	virtual void handleMessage(cMessage *msg);
	virtual void finish(void);
};

#endif  // __LAMBDA_SPLITTER_H
