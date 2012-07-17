///
/// @file   OnuMacLayer2.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jul/6/2012
///
/// @brief  Declares 'OnuMacLayer2' class for a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __ONU_MAC_LAYER2_H
#define __ONU_MAC_LAYER2_H

#include <omnetpp.h>
#include "Ethernet.h"
#include "EtherFrame_m.h"

///
/// @class OnuMacLayer2
/// @brief Implements MAC layer in a hybrid TDM/WDM-PON ONU with its own light source.
/// @ingroup hybridpon
///
class OnuMacLayer2: public cSimpleModule
{
    // MAC transmission state
    enum MacTxState {
        STATE_IDLE          = 1,
//        WAIT_IFG_STATE      = 2,
        STATE_TRANSMITTING  = 2
    };

    // MAC self messages
    enum MacMsgType {
        MSG_TX_END          = 100
    };

    protected:
        // NED parameters
        int queueSize; ///< size of FIFO queue [bits]

        // configuration variables
        double lineRate; ///< line rate of optical channel

        // status variables
        int busyQueue; ///< current queue length [bits]
        cQueue queue; ///< FIFO queue holding frames from UNIs

        // states
        MacTxState txState;  /// state of frame transmission

        // self messages
        cMessage *endTxMsg;

    public:
      OnuMacLayer2();
      virtual ~OnuMacLayer2();

    protected:
        // event handling functions
        void handleEthernetFrameFromUni(EtherFrame *frame);
        virtual void handleEndTxMsg(void);

        // OMNeT++
        virtual void initialize(void);
        virtual void handleMessage(cMessage *msg);
        virtual void finish(void);
};

#endif  // __ONU_MAC_LAYER2_H
