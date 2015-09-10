/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INET_ETHERENCAP_H
#define __INET_ETHERENCAP_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ethernet/Ethernet.h"

namespace inet {

// Forward declarations:
class EtherFrame;

/**
 * Performs Ethernet II encapsulation/decapsulation. More info in the NED file.
 */
class INET_API EtherEncap : public cSimpleModule
{
  protected:
    int seqNum;

    // statistics
    long totalFromHigherLayer;    // total number of packets received from higher layer
    long totalFromMAC;    // total number of frames received from MAC
    long totalPauseSent;    // total number of PAUSE frames sent
    static simsignal_t encapPkSignal;
    static simsignal_t decapPkSignal;
    static simsignal_t pauseSentSignal;
    bool useSNAP;    // true: generate EtherFrameWithSNAP, false: generate EthernetIIFrame

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void processPacketFromHigherLayer(cPacket *msg);
    virtual void processFrameFromMAC(EtherFrame *msg);
    virtual void handleSendPause(cMessage *msg);

    virtual void updateDisplayString();
};

} // namespace inet

#endif // ifndef __INET_ETHERENCAP_H

