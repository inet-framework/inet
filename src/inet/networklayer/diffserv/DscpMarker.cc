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

#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"
#include "inet/networklayer/diffserv/Dscp_m.h"
#include "inet/networklayer/diffserv/DscpMarker.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif // ifdef WITH_IPv6

namespace inet {

using namespace DiffservUtil;

Define_Module(DscpMarker);

simsignal_t DscpMarker::packetMarkedSignal = registerSignal("packetMarked");

void DscpMarker::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        parseDSCPs(par("dscps"), "dscps", dscps);
        if (dscps.empty())
            dscps.push_back(DSCP_BE);
        while ((int)dscps.size() < gateSize("in"))
            dscps.push_back(dscps.back());

        numRcvd = 0;
        numMarked = 0;
        WATCH(numRcvd);
        WATCH(numMarked);
    }
}

void DscpMarker::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void DscpMarker::pushPacket(Packet *packet, cGate *inputGate)
{
    numRcvd++;
    int dscp = dscps.at(inputGate->getIndex());
    if (markPacket(packet, dscp)) {
        emit(packetMarkedSignal, packet);
        numMarked++;
    }
    pushOrSendPacket(packet, outputGate, consumer);
}

void DscpMarker::refreshDisplay() const
{
    char buf[50] = "";
    if (numRcvd > 0)
        sprintf(buf + strlen(buf), "rcvd: %d ", numRcvd);
    if (numMarked > 0)
        sprintf(buf + strlen(buf), "mark:%d ", numMarked);
    getDisplayString().setTagArg("t", 0, buf);
}

bool DscpMarker::markPacket(Packet *packet, int dscp)
{
    EV_DETAIL << "Marking packet with dscp=" << dscpToString(dscp) << "\n";

    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();

    //TODO processing link-layer headers when exists

#ifdef WITH_IPv4
    if (protocol == &Protocol::ipv4) {
        packet->trimFront();
        const auto& ipv4Header = packet->removeAtFront<Ipv4Header>();
        ipv4Header->setDscp(dscp);
        Ipv4::insertCrc(ipv4Header);
        packet->insertAtFront(ipv4Header);
        return true;
    }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (protocol == &Protocol::ipv6) {
        packet->trimFront();
        const auto& ipv6Header = packet->removeAtFront<Ipv6Header>();
        ipv6Header->setDscp(dscp);
        packet->insertAtFront(ipv6Header);
        return true;
    }
#endif // ifdef WITH_IPv6

    return false;
}

} // namespace inet
