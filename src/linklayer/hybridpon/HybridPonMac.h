///
/// @file   HybridPonMac.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Declares 'HybridPonMac' class for a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2009 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_HYBRIDPONMAC_H
#define __INET_HYBRIDPONMAC_H


#include <assert.h>
#include "HybridPon.h"
#include "HybridPonFrame_m.h"
/* #include "Monitor.h" */


///
/// @class HybridPonMac
/// @brief Implements MAC module in a hybrid TDM/WDM-PON ONU.
/// @ingroup hybridpon
///
class HybridPonMac : public cSimpleModule
{
protected:
	// NED parameters
//	int channel;	///< wavelength channel number assigned to an ONU
    int queueSize;	///< size of FIFO queue [bits]

	// status variables
    int busyQueue;	///< current queue length [bits]
    cQueue queue;	///< FIFO queue holding frames from UNIs
/* 	Monitor *monitor;   // 'Monitor' node */

protected:
	// event handling functions
    void handleEthernetFrameFromUni(EtherFrame *frame);
    void handleDataFromPon(HybridPonDsDataFrame *frame);
    void handleGrantFromPon(HybridPonDsGrantFrame *frame);

    // functions from OMNeT++
    virtual void initialize(void);
    virtual void handleMessage(cMessage *msg);
    virtual void finish(void);
};


#endif  // __INET_HYBRIDPONMAC_H
