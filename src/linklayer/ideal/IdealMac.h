//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe
//

#ifndef __INET_IDEALMAC_H
#define __INET_IDEALMAC_H


#include "INETDefs.h"

#include "MACAddress.h"
#include "RadioState.h"
#include "WirelessMacBase.h"

class IdealMacFrame;
class InterfaceEntry;
class IPassiveQueue;

/**
 * Implements a MAC suitable for use with IdealRadio.
 *
 * See the NED file for details.
 */
class INET_API IdealMac : public WirelessMacBase
{
  protected:
    static simsignal_t radioStateSignal;
    static simsignal_t dropPkNotForUsSignal;

    // parameters
    int headerLength;       // IdealAirFrame header length in bytes
    double bitrate;         // [bits per sec]
    bool promiscuous;       // promiscuous mode
    MACAddress address;     // MAC address

    IPassiveQueue *queueModule;
    cModule *radioModule;

    RadioState::State radioState;
    int outStandingRequests;
    simtime_t lastTransmitStartTime;

  protected:
    /** implements MacBase functions */
    //@{
    virtual void flushQueue();
    virtual void clearQueue();
    virtual InterfaceEntry *createInterfaceEntry();
    //@}

    virtual void startTransmitting(cPacket *msg);
    virtual bool dropFrameNotForUs(IdealMacFrame *frame);
    virtual IdealMacFrame *encapsulate(cPacket *msg);
    virtual cPacket *decapsulate(IdealMacFrame *frame);
    virtual void initializeMACAddress();

    // get MSG from queue
    virtual void getNextMsgFromHL();

    //cListener:
    virtual void receiveSignal(cComponent *src, simsignal_t id, long x);

    /** implements WirelessMacBase functions */
    //@{
    virtual void handleSelfMsg(cMessage *msg);
    virtual void handleUpperMsg(cPacket *msg);
    virtual void handleCommand(cMessage *msg);
    virtual void handleLowerMsg(cPacket *msg);
    //@}

  public:
    IdealMac();
    virtual ~IdealMac();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
};

#endif
