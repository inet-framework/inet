//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/pim/Pim.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"

namespace inet {

Define_Module(Pim);
Define_Module(PimChecksumInsertionHook);

Pim::~Pim()
{
}

void Pim::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        if (checksumMode == CHECKSUM_COMPUTED) {
            cModuleType *moduleType = cModuleType::get("inet.routing.pim.PimChecksumInsertionHook");
            auto checksumInsertion = check_and_cast<PimChecksumInsertionHook *>(moduleType->create("checksumInsertion", this));
            checksumInsertion->finalizeParameters();
            checksumInsertion->callInitialize();

            // For IPv6, the checksum also includes the IPv6 "pseudo-header",
            // as specified in RFC 2460, Section 8.1 [5].  This
            // "pseudo-header" is prepended to the PIM header for the purposes
            // of calculating the checksum.  The "Upper-Layer Packet Length"
            // in the pseudo-header is set to the length of the PIM message,
            // except in Register messages where it is set to the length of
            // the PIM register header (8).  The Next Header value used in the
            // pseudo-header is 103.
#ifdef INET_WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(findModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, checksumInsertion);
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

INetfilter::IHook::Result PimChecksumInsertionHook::datagramPostRoutingHook(Packet *packet)
{
    Enter_Method("datagramPostRoutingHook");

    if (packet->findTag<InterfaceInd>())
        return ACCEPT; // FORWARD
    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (*networkProtocol == Protocol::ipv6) {
        const auto& networkHeader = getNetworkProtocolHeader(packet);
        if (*networkHeader->getProtocol() == Protocol::pim) {
            ASSERT(!networkHeader->isFragment());
            packet->eraseAtFront(networkHeader->getChunkLength());
            auto pimPacket = packet->removeAtFront<PimPacket>();
            ASSERT(pimPacket->getChecksumMode() == CHECKSUM_COMPUTED);
            const L3Address& srcAddress = networkHeader->getSourceAddress();
            const L3Address& destAddress = networkHeader->getDestinationAddress();
            Pim::insertChecksum(networkProtocol, srcAddress, destAddress, pimPacket);
            packet->insertAtFront(pimPacket);
            packet->insertAtFront(networkHeader);
        }
    }
    return ACCEPT;
}

void Pim::insertChecksum(const Ptr<PimPacket>& pimPacket)
{
    insertChecksum(&Protocol::ipv4, L3Address(), L3Address(), pimPacket);
}

void Pim::insertChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<PimPacket>& pimPacket)
{
    ChecksumMode checksumMode = pimPacket->getChecksumMode();
    switch (checksumMode) {
        case CHECKSUM_DISABLED:
            // if the CHECKSUM mode is disabled, then the CHECKSUM is 0
            pimPacket->setChecksum(0x0000);
            break;
        case CHECKSUM_DECLARED_CORRECT:
            // if the CHECKSUM mode is declared to be correct, then set the CHECKSUM to an easily recognizable value
            pimPacket->setChecksum(0xC00D);
            break;
        case CHECKSUM_DECLARED_INCORRECT:
            // if the CHECKSUM mode is declared to be incorrect, then set the CHECKSUM to an easily recognizable value
            pimPacket->setChecksum(0xBAAD);
            break;
        case CHECKSUM_COMPUTED: {
            // if the CHECKSUM mode is computed, then compute the CHECKSUM and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            pimPacket->setChecksum(0x0000); // make sure that the CHECKSUM is 0 in the Udp header before computing the CHECKSUM
            auto checksum = computeChecksum(networkProtocol, srcAddress, destAddress, pimPacket);
            pimPacket->setChecksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode: %d", (int)checksumMode);
    }
}

bool Pim::verifyChecksum(const Protocol *networkProtocol, const Ptr<const PimPacket>& pimPacket, Packet *packet)
{
    switch (pimPacket->getChecksumMode()) {
        case CHECKSUM_DISABLED:
            // if the CHECKSUM mode is disabled, then the check passes if the CHECKSUM is 0
            return pimPacket->getChecksum() == 0x0000;
        case CHECKSUM_DECLARED_CORRECT: {
            // if the CHECKSUM mode is declared to be correct, then the check passes if and only if the chunks are correct
            auto totalLength = pimPacket->getChunkLength();
            auto udpDataBytes = packet->peekDataAt(B(0), totalLength - pimPacket->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
            return pimPacket->isCorrect() && udpDataBytes->isCorrect();
        }
        case CHECKSUM_DECLARED_INCORRECT:
            // if the CHECKSUM mode is declared to be incorrect, then the check fails
            return false;
        case CHECKSUM_COMPUTED: {
            auto l3AddressInd = packet->getTag<L3AddressInd>();
            auto srcAddress = l3AddressInd->getSrcAddress();
            auto destAddress = l3AddressInd->getDestAddress();
            auto computedChecksum = computeChecksum(networkProtocol, srcAddress, destAddress, pimPacket);
            return computedChecksum == 0xFFFF;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode");
    }
}

uint16_t Pim::computeChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const PimPacket>& pimPacket)
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
    uint16_t checksum = TcpIpChecksum::checksum(stream.getData());
    return checksum;
}

} // namespace inet

