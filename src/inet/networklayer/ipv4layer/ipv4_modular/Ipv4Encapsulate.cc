//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4Encapsulate.h"

#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4OptionsTag_m.h"

namespace inet {

Define_Module(Ipv4Encapsulate);

void Ipv4Encapsulate::initialize(int stage)
{
    queueing::PacketPusherBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);

        defaultTimeToLive = par("timeToLive");
        defaultMCTimeToLive = par("multicastTimeToLive");

        curFragmentId = 0;
    }
}

void Ipv4Encapsulate::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    EV_INFO << "Received " << packet << " from upper layer.\n";
    emit(packetReceivedFromUpperSignal, packet);

    // if no interface exists, do not send datagram
    if (ift->getNumInterfaces() == 0) {
        EV_ERROR << "No interfaces exist, dropping packet\n";
        dropPacket(packet, NO_INTERFACE_FOUND);
        return;
    }

    encapsulate(packet);

    pushOrSendPacket(packet, outputGate, consumer);
}

void Ipv4Encapsulate::encapsulate(Packet *transportPacket)
{
    const auto& ipv4Header = makeShared<Ipv4Header>();

    auto l3AddressReq = transportPacket->removeTag<L3AddressReq>();
    Ipv4Address src = l3AddressReq->getSrcAddress().toIpv4();
    bool nonLocalSrcAddress = l3AddressReq->getNonLocalSrcAddress();
    Ipv4Address dest = l3AddressReq->getDestAddress().toIpv4();

    ipv4Header->setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(transportPacket->getTag<PacketProtocolTag>()->getProtocol()));

    auto hopLimitReq = transportPacket->removeTagIfPresent<HopLimitReq>();
    short ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;
    bool dontFragment = false;
    if (auto& dontFragmentReq = transportPacket->removeTagIfPresent<FragmentationReq>())
        dontFragment = dontFragmentReq->getDontFragment();

    // set source and destination address
    ipv4Header->setDestAddress(dest);

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified()) {
        if (!nonLocalSrcAddress && rt->getInterfaceByAddress(src) == nullptr)
            // if interface parameter does not match existing interface, do not send datagram
            throw cRuntimeError("Wrong source address %s in (%s)%s: no interface with such address",
                    src.str().c_str(), transportPacket->getClassName(), transportPacket->getFullName());

        ipv4Header->setSrcAddress(src);
    }

    // set other fields
    if (auto& tosReq = transportPacket->removeTagIfPresent<TosReq>()) {
        ipv4Header->setTypeOfService(tosReq->getTos());
        if (transportPacket->findTag<DscpReq>())
            throw cRuntimeError("TosReq and DscpReq found together");
        if (transportPacket->findTag<EcnReq>())
            throw cRuntimeError("TosReq and EcnReq found together");
        transportPacket->addTag<TosInd>()->setTos(ipv4Header->getTypeOfService());
    }
    if (auto& dscpReq = transportPacket->removeTagIfPresent<DscpReq>()) {
        ipv4Header->setDscp(dscpReq->getDifferentiatedServicesCodePoint());
        transportPacket->addTag<DscpInd>()->setDifferentiatedServicesCodePoint(ipv4Header->getDscp());
    }
    if (auto& ecnReq = transportPacket->removeTagIfPresent<EcnReq>()) {
        ipv4Header->setEcn(ecnReq->getExplicitCongestionNotification());
        transportPacket->addTag<EcnInd>()->setExplicitCongestionNotification(ipv4Header->getEcn());
    }

    ipv4Header->setIdentification(curFragmentId++);
    ipv4Header->setMoreFragments(false);
    ipv4Header->setDontFragment(dontFragment);
    ipv4Header->setFragmentOffset(0);

    if (ttl != -1) {
        ASSERT(ttl > 0);
    }
    else if (ipv4Header->getDestAddress().isLinkLocalMulticast())
        ttl = 1;
    else if (ipv4Header->getDestAddress().isMulticast())
        ttl = defaultMCTimeToLive;
    else
        ttl = defaultTimeToLive;
    ipv4Header->setTimeToLive(ttl);

    if (auto& optReq = transportPacket->removeTagIfPresent<Ipv4OptionsReq>()) {
        for (size_t i = 0; i < optReq->getOptionArraySize(); i++) {
            auto opt = optReq->removeOption(i);
            ipv4Header->addOption(opt);
            ipv4Header->addChunkLength(B(opt->getLength()));
        }
    }

    ASSERT(ipv4Header->getChunkLength() <= IPv4_MAX_HEADER_LENGTH);
    ipv4Header->setHeaderLength(ipv4Header->getChunkLength());
    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + transportPacket->getDataLength());
    ipv4Header->setCrcMode(crcMode);
    ipv4Header->setCrc(0);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            ipv4Header->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            ipv4Header->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            ipv4Header->setCrc(0);
            // crc will be calculated in fragmentAndSend()
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }

    // TODO setting Ipv4 options is currently not supported

    insertNetworkProtocolHeader(transportPacket, Protocol::ipv4, ipv4Header);
}

} // namespace inet
