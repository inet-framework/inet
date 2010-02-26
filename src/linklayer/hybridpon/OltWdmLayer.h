///
/// @file   OltWdmLayer.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/21/2010
///
/// @brief  Declares 'OltWdmLayer' class for a hybrid TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __OLT_WDM_LAYER_H
#define __OLT_WDM_LAYER_H

#include "HybridPon.h"

class OltWdmLayer: public cSimpleModule
{
protected:
	//OMNeT++
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
};

#endif  // ____OLT_WDM_LAYER_H
