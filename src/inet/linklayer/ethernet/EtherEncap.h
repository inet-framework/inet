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

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace inet {

/**
 * Performs Ethernet II encapsulation/decapsulation. More info in the NED file.
 */
class INET_API EtherEncap : public Ieee8022Llc
{
  protected:
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
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
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void processCommandFromHigherLayer(cMessage *msg);
    virtual void processPacketFromHigherLayer(Packet *msg);
    virtual void processFrameFromMAC(Packet *packet);
    virtual void handleSendPause(cMessage *msg);

    virtual void refreshDisplay() const override;

    virtual const Ptr<const EthernetMacHeader> decapsulateMacLlcSnap(Packet *packet);

  public:
    static void addPaddingAndFcs(Packet *packet, FcsMode fcsMode, B requiredMinByteLength = MIN_ETHERNET_FRAME_BYTES);
    static void addFcs(Packet *packet, FcsMode fcsMode);

    static const Ptr<const EthernetMacHeader> decapsulateMacHeader(Packet *packet);
};

} // namespace inet

#endif // ifndef __INET_ETHERENCAP_H

