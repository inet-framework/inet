///
/// @file   OnuWdmLayer.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/22/2010
///
/// @brief  Declares 'OnuWdmLayer' class for a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __ONU_WDM_LAYER_H
#define __ONU_WDM_LAYER_H

#include <omnetpp.h>
#include "HybridPon.h"

class OnuWdmLayer: public cSimpleModule
{
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
};

#endif  // ____ONU_WDM_LAYER_H
