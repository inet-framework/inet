///
/// @file   DelayMeter.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/1/2012
///
/// @brief  Declares 'DelayMeter' class for measuring frame/packet end-to-end delay.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_DELAYMETER_H
#define __INET_DELAYMETER_H

#include <omnetpp.h>

///
/// @class DelayMeter
/// @brief Implements DelayMeter module for measuring frame/packet end-to-end delay
///
class DelayMeter : public cSimpleModule
{
  protected:
    simsignal_t packetDelaySignal;
    unsigned long numPackets;
    double sumPacketDelays;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif
