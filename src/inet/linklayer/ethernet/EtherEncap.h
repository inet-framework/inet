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
#include "inet/networklayer/common/InterfaceEntry.h"

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
    InterfaceEntry *interfaceEntry = nullptr;

    struct Socket
    {
        int socketId = -1;
        MacAddress localAddress;
        MacAddress remoteAddress;
        const Protocol *protocol = nullptr;
        int vlanId = -1;

        Socket(int socketId) : socketId(socketId) {}
        bool matches(Packet *packet, const Ptr<const EthernetMacHeader>& ethernetMacHeader);
    };

    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual ~EtherEncap();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void processCommandFromHigherLayer(Request *msg) override;
    virtual void processPacketFromHigherLayer(Packet *msg) override;
    virtual void processPacketFromMac(Packet *packet) override;
    virtual void handleSendPause(cMessage *msg);

    virtual void refreshDisplay() const override;

  public:
    /**
     * Inserts the FCS chunk to end of packet. Fill the fcsMode and set fcs to 0.
     */
    static const Ptr<const EthernetMacHeader> decapsulateMacHeader(Packet *packet);
};

} // namespace inet

#endif // ifndef __INET_ETHERENCAP_H

