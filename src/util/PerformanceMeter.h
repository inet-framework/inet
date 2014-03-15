///
/// @file   PerformanceMeter.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/1/2012
///
/// @brief  Declares 'PerformanceMeter' class for measuring frame/packet end-to-end
///         delay and throughput.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_PERFORMANCEMETER_H
#define __INET_PERFORMANCEMETER_H

#include <omnetpp.h>
#include "INETDefs.h"

///
/// @class PerformanceMeter
/// @brief Measure frame/packet end-to-end delay and throughput
///
class INET_API PerformanceMeter : public cSimpleModule
{
  protected:
    // config
    simtime_t startTime;            // start time
    simtime_t measurementInterval;  // measurement interval

    // global statistics
    double sumPacketDelays;
    unsigned long numPackets;
    unsigned long numBits;

    // current measurement interval
//    double intvSumPacketDelays;
    unsigned long intvlNumPackets;
    unsigned long intvlNumBits;

    // signals for statistics
    simsignal_t packetDelaySignal;
    simsignal_t bitThruputSignal;
    simsignal_t packetThruputSignal;
    /* cOutVector bitpersecVector; */
    /* cOutVector pkpersecVector; */

    // timer
    bool measurementStarted;
    cMessage *measurementTimer;

  public:
    PerformanceMeter();
    virtual ~PerformanceMeter();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif
