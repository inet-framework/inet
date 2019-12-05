//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#include "inet/common/queue/AlgorithmicDropperBase.h"

namespace inet {

void AlgorithmicDropperBase::initialize()
{
    numGates = gateSize("out");
    for (int i = 0; i < numGates; ++i) {
        cGate *outGate = gate("out", i);
        cGate *connectedGate = outGate->getPathEndGate();
        if (!connectedGate)
            throw cRuntimeError("ThresholdDropper out gate %d is not connected", i);
        IQueueAccess *outModule = dynamic_cast<IQueueAccess *>(connectedGate->getOwnerModule());
        if (!outModule)
            throw cRuntimeError("ThresholdDropper out gate %d should be connected a simple module implementing IQueueAccess", i);
        outQueues.push_back(outModule);
        outQueueSet.insert(outModule);
    }
}

void AlgorithmicDropperBase::handleMessage(cMessage *msg)
{
    cPacket *packet = check_and_cast<cPacket *>(msg);
    if (shouldDrop(packet)) {
        // rfc-3168, pages 6-7:
        // The ECN-Capable Transport (ECT) codepoints '10' and '01' are set by the
        // data sender to indicate that the end-points of the transport protocol
        // are ECN-capable; we call them ECT(0) and ECT(1) respectively...
        // ...The not-ECT codepoint '00' indicates a packet that is not using ECN.
        // The CE codepoint '11' is set by a router to indicate congestion to
        // the end nodes.  Routers that have a packet arriving at a full queue
        // drop the packet, just as they do in the absence of ECN.
        //
        // The ECN Field in IP:
        //  +-----+-----+
        //  | ECN FIELD |
        //  +-----+-----+
        //    ECT   CE         [Obsolete] RFC 2481 names for the ECN bits.
        //     0     0         Not-ECT
        //     0     1         ECT(1)
        //     1     0         ECT(0)
        //     1     1         CE
        //
        // rfc-2884, pages 4+5:
        // RED could set a Congestion Experienced (CE) bit in the packet header
        // instead of dropping the packet...
        // ...
        // the CE bit of an ECN-Capable packet should only be set
        // if the router would otherwise have dropped the packet as an
        // indication of congestion to the end nodes. When the router's buffer
        // is not yet full and the router is prepared to drop a packet to inform
        // end nodes of incipient congestion, the router should first check to
        // see if the ECT bit is set in that packet's IP header.  If so, then
        // instead of dropping the packet, the router MAY instead set the CE bit
        // in the IP header.

        IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(msg);
        int CE = datagram->getExplicitCongestionNotification();
        // check if ECT and mark the packet instead of dropping it
        if (CE != 0) {
            EV_INFO << "ECN-Capable Transport... set CE instead of dropping\n";
            datagram->setExplicitCongestionNotification(3); //set CE
            sendOut(packet);
        } else {
            dropPacket(packet);
        }
    } else
        sendOut(packet);
}

void AlgorithmicDropperBase::dropPacket(cPacket *packet)
{
    // TODO statistics
    delete packet;
}

void AlgorithmicDropperBase::sendOut(cPacket *packet)
{
    int index = packet->getArrivalGate()->getIndex();
    send(packet, "out", index);
}

int AlgorithmicDropperBase::getLength() const
{
    int len = 0;
    for (const auto & elem : outQueueSet)
        len += (elem)->getLength();
    return len;
}

int AlgorithmicDropperBase::getByteLength() const
{
    int len = 0;
    for (const auto & elem : outQueueSet)
        len += (elem)->getByteLength();
    return len;
}

} // namespace inet

