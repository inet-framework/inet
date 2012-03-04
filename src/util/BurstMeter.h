///
/// @file   BurstMeter.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/3/2012
///
/// @brief  Declares 'BurstMeter' class for measuring frame/packet bursts.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_BURSTMETER_H
#define __INET_BURSTMETER_H

#include <omnetpp.h>
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "MACAddress.h"

///
/// @class BurstMeter
/// @brief Implements BurstMeter module for measuring frame/packet bursts
///
class BurstMeter : public cSimpleModule
{
  protected:
    simsignal_t packetBurstSignal;
    unsigned long burst;
    unsigned long numBursts;
    unsigned long sumBursts;
    MACAddress macAddress;      // MAC address of the last frame
    // TODO: Include processing of other frame formats
    simtime_t receptionTime;    // reception time of the last frame/packet
    simtime_t maxFrameGap;      // maximum frame gap in time allowed between two consecutive Etherent frames of a same burst
    int maxIFG;                 // maximum frame gap in number of Ethernet IFGs

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif
