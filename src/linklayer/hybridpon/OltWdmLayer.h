///
/// @file   WdmLayer.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/21/2010
///
/// @brief  Declares 'WdmLayer' class for a hybrid TDM/WDM-PON.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __WDM_LAYER_H
#define __WDM_LAYER_H

#include <omnetpp.h>
#include "HybridPon.h"

class WdmLayer: public cSimpleModule {
	int inBaseId; // base ID of demuxg$i gate vector
	int outBaseId; // base ID of demuxg$o gate vector
	//	inu	gateSize;	// size of demuxg gate vector

	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
};

#endif  // ____WDM_LAYER_H
