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

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/MACProtocolBase.h"

namespace inet {

using namespace physicallayer;

class IdealMacFrame;
class InterfaceEntry;
class IPassiveQueue;

/**
 * Implements a simplified ideal MAC.
 *
 * See the NED file for details.
 */
class INET_API IdealMac : public MACProtocolBase
{
  protected:
    static simsignal_t dropPkNotForUsSignal;

    // parameters
    int headerLength;    // IdealMacFrame header length in bytes
    double bitrate;    // [bits per sec]
    bool promiscuous;    // promiscuous mode
    MACAddress address;    // MAC address
    bool fullDuplex;

    IRadio *radio;
    IRadio::TransmissionState transmissionState;
    IPassiveQueue *queueModule;

    int outStandingRequests;
    cPacket *lastSentPk;
    simtime_t ackTimeout;
    cMessage *ackTimeoutMsg;

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
    virtual void acked(IdealMacFrame *frame);    // called by other IdealMac module, when receiving a packet with my moduleID

    // get MSG from queue
    virtual void getNextMsgFromHL();

    //cListener:
    virtual void receiveSignal(cComponent *src, simsignal_t id, long value);

    /** implements MACProtocolBase functions */
    //@{
    virtual void handleUpperPacket(cPacket *msg);
    virtual void handleLowerPacket(cPacket *msg);
    virtual void handleSelfMessage(cMessage *message);
    //@}

  public:
    IdealMac();
    virtual ~IdealMac();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
};

} // namespace inet

#endif // ifndef __INET_IDEALMAC_H

