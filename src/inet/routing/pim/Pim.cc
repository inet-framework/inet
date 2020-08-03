//
// Copyright (C) 2019 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Zoltan Bojthe

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/routing/pim/Pim.h"

namespace inet {

Define_Module(Pim);

Pim::~Pim()
{
}

void Pim::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        if (crcMode == CRC_COMPUTED) {
            // For IPv6, the checksum also includes the IPv6 "pseudo-header",
            // as specified in RFC 2460, Section 8.1 [5].  This
            // "pseudo-header" is prepended to the PIM header for the purposes
            // of calculating the checksum.  The "Upper-Layer Packet Length"
            // in the pseudo-header is set to the length of the PIM message,
            // except in Register messages where it is set to the length of
            // the PIM register header (8).  The Next Header value used in the
            // pseudo-header is 103.
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(findModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &crcInsertion);
#endif
        }
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerProtocol(Protocol::pim, gate("networkLayerOut"), gate("networkLayerIn"));
    }
}

void Pim::handleMessageWhenUp(cMessage *msg)
{
    throw cRuntimeError("this module doesn't allow incoming packets");
}

void Pim::handleStartOperation(LifecycleOperation *operation)
{
}

void Pim::handleStopOperation(LifecycleOperation *operation)
{
}

void Pim::handleCrashOperation(LifecycleOperation *operation)
{
}

INetfilter::IHook::Result Pim::CrcInsertion::datagramPostRoutingHook(Packet *packet)
{
    if (packet->findTag<InterfaceInd>())
        return ACCEPT;  // FORWARD
    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (*networkProtocol == Protocol::ipv6) {
        const auto& networkHeader = getNetworkProtocolHeader(packet);
        if (*networkHeader->getProtocol() == Protocol::pim) {
            ASSERT(!networkHeader->isFragment());
            packet->eraseAtFront(networkHeader->getChunkLength());
            auto pimPacket = packet->removeAtFront<PimPacket>();
            ASSERT(pimPacket->getCrcMode() == CRC_COMPUTED);
            const L3Address& srcAddress = networkHeader->getSourceAddress();
            const L3Address& destAddress = networkHeader->getDestinationAddress();
            Pim::insertCrc(networkProtocol, srcAddress, destAddress, pimPacket);
            packet->insertAtFront(pimPacket);
            packet->insertAtFront(networkHeader);
        }
    }
    return ACCEPT;
}

void Pim::insertCrc(const Ptr<PimPacket>& pimPacket)
{
    insertCrc(&Protocol::ipv4, L3Address(), L3Address(), pimPacket);
}

void Pim::insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<PimPacket>& pimPacket)
{
    CrcMode crcMode = pimPacket->getCrcMode();
    switch (crcMode) {
        case CRC_DISABLED:
            // if the CRC mode is disabled, then the CRC is 0
            pimPacket->setCrc(0x0000);
            break;
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            pimPacket->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            pimPacket->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            pimPacket->setCrc(0x0000); // make sure that the CRC is 0 in the Udp header before computing the CRC
            auto crc = computeCrc(networkProtocol, srcAddress, destAddress, pimPacket);
            pimPacket->setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

bool Pim::verifyCrc(const Protocol *networkProtocol, const Ptr<const PimPacket>& pimPacket, Packet *packet)
{
    switch (pimPacket->getCrcMode()) {
        case CRC_DISABLED:
            // if the CRC mode is disabled, then the check passes if the CRC is 0
            return pimPacket->getCrc() == 0x0000;
        case CRC_DECLARED_CORRECT: {
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunks are correct
            auto totalLength = pimPacket->getChunkLength();
            auto udpDataBytes = packet->peekDataAt(B(0), totalLength - pimPacket->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
            return pimPacket->isCorrect() && udpDataBytes->isCorrect();
        }
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then the check fails
            return false;
        case CRC_COMPUTED: {
            auto l3AddressInd = packet->getTag<L3AddressInd>();
            auto srcAddress = l3AddressInd->getSrcAddress();
            auto destAddress = l3AddressInd->getDestAddress();
            auto computedCrc = computeCrc(networkProtocol, srcAddress, destAddress, pimPacket);
            return computedCrc == 0xFFFF;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

uint16_t Pim::computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const PimPacket>& pimPacket)
{
    MemoryOutputStream stream;
    if (networkProtocol == &Protocol::ipv6) {
        auto pseudoHeader = makeShared<TransportPseudoHeader>();
        pseudoHeader->setSrcAddress(srcAddress);
        pseudoHeader->setDestAddress(destAddress);
        pseudoHeader->setNetworkProtocolId(networkProtocol->getId());
        pseudoHeader->setProtocolId(IP_PROT_PIM);
        pseudoHeader->setPacketLength(pimPacket->getChunkLength());
        pseudoHeader->setChunkLength(B(40));
        Chunk::serialize(stream, pseudoHeader);
    }
    Chunk::serialize(stream, pimPacket);
    uint16_t crc = TcpIpChecksum::checksum(stream.getData());
    return crc;
}

}    // namespace inet

