///
/// @file   OnuMacLayer.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Declares 'OnuMacLayer' class for a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __ONU_MAC_LAYER_H
#define __ONU_MAC_LAYER_H

//#include <assert.h>
#include "HybridPon.h"

///
/// @class OnuMacLayer
/// @brief Implements MAC layer in a hybrid TDM/WDM-PON ONU.
/// @ingroup hybridpon
///
class OnuMacLayer: public cSimpleModule
{
protected:
	// NED parameters
	//	int channel;	///< wavelength channel number assigned to an ONU
	int queueSize; ///< size of FIFO queue [bits]

	// configuration variables
	double lineRate; ///< line rate of optical channel

	// status variables
	int busyQueue; ///< current queue length [bits]
	cQueue queue; ///< FIFO queue holding frames from UNIs

protected:
	// event handling functions
	void handleEthernetFrameFromUni(EtherFrame *frame);
	void handleDataPonFrameFromPon(HybridPonDsDataFrame *frame);
	void handleGrantPonFrameFromPon(HybridPonDsGrantFrame *frame);

	// OMNeT++
	virtual void initialize(void);
	virtual void handleMessage(cMessage *msg);
	virtual void finish(void);
};

#endif  // __ONU_MAC_LAYER_H
