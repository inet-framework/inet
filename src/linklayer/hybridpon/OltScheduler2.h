///
/// @file   OltScheduler2.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jul/09/2012
///
/// @brief  Declares 'OltScheduler2' and its derived classes for a hybrid
///			TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __OLT_SCHEDULER2_H
#define __OLT_SCHEDULER2_H

#include <sstream>
#include "HybridPon.h"

///
/// @class      OltScheduler2
/// @brief      Implements 'OltScheduler2' module in a hybrid TDM/WDM-PON OLT
///             with tunable transmitters and fixed receivers.
/// @ingroup    hybridpon
///
class OltScheduler2 : public cSimpleModule
{
    protected:
        typedef std::vector<cQueue *> QueueVector;

    protected:
        // OLT NED parameters
        int numTransmitters; ///< number of tunable transmitters

        // configuration variables
        double lineRate;    ///< line rate of optical channel
        int numOnus;    ///< number of ONUs (= number of WDM channels)

        // state
        int numTxsAvailable;    ///< number of available tunable transmitters

        // message
        HybridPonMsgVector releaseTxMsg;    ///< vector of self messages with channel (ONU index) information

    public:
        OltScheduler2();
        virtual ~OltScheduler2();

    protected:
        // Misc.

        // Event handling
        virtual void handleEthernetFrameFromSni(EtherFrame *frame) = 0; // pure virtual function
        virtual void handleEndTxMsg(HybridPonMessage *msg) = 0;;

        // OMNeT++
        virtual void initialize(void);
        virtual void handleMessage(cMessage *msg);
        virtual void finish(void);
};

///
/// @class      OltScheduler2MCDRR
/// @brief      Implements 'OltScheduler2MCDRR' (Multi-Channel Deficit Round-Robin)
///             module for a hybrid TDM/WDM-PON OLT.
/// @ingroup    hybridpon
///
class OltScheduler2MCDRR : public OltScheduler2
{
    protected:
        // type definitions for member variables
        typedef std::list<int> IntList;

    protected:
        // configuration
        int frameCapacity; ///< per subqueue

        // state
        bool continued; ///< flag indicating whether the current run is a continuation of the previous one or not
        int currentQueueIndex;  ///< index of a queue whose HOL frame is scheduled for TX during the last scheduling
        BoolVector isChannelAvailable; ///< vector of channel availability
        IntVector deficitCounters;  ///< vector of deficit counters in DRR scheduling
        IntVector quanta;   ///< vector of quantum  in DRR scheduling
        IntList activeList; ///< list of indices of active queues
        QueueVector queues; ///< VOQs

        // statistics
        IntVector numQueueReceived;
        IntVector numQueueDropped;

    public:
//        OltScheduler2MCDRR();
        virtual ~OltScheduler2MCDRR();

    protected:
        // Misc.
        virtual bool enqueue(EtherFrame *frame);
        virtual EtherFrame *dequeue();

        // Event handling
        virtual void handleEthernetFrameFromSni(EtherFrame *frame);
        virtual void handleEndTxMsg(HybridPonMessage *msg);

        // OMNeT++
        virtual void initialize(void);
        virtual void finish(void);
};

///
/// @class      OltScheduler2MCDRRv2
/// @brief      Implements 'OltScheduler2MCDRRv2' (Multi-Channel Deficit Round-Robin ver. 2)
///             module for a hybrid TDM/WDM-PON OLT.
/// @ingroup    hybridpon
///
class OltScheduler2MCDRRv2 : public OltScheduler2MCDRR
{
    protected:
        // state
        IntVector numPktsScheduled;  ///< vector of the number of packets scheduled per channel

    protected:
        // Misc.
        /* virtual bool enqueue(EtherFrame *frame); */
        virtual EtherFrame *dequeue();

        // Event handling
        /* virtual void handleEthernetFrameFromSni(EtherFrame *frame); */
        virtual void handleEndTxMsg(HybridPonMessage *msg);

        // OMNeT++
        virtual void initialize(void);
};

///
/// @class      OltScheduler2MCDRRv3
/// @brief      Implements 'OltScheduler2MCDRRv3' (Multi-Channel Deficit Round-Robin ver. 3)
///             module for a hybrid TDM/WDM-PON OLT.
/// @ingroup    hybridpon
///
class OltScheduler2MCDRRv3 : public OltScheduler2MCDRRv2
{
    protected:
        // Misc.
        virtual EtherFrame *dequeue();
};

#endif  // __OLT_SCHEDULER2_H
