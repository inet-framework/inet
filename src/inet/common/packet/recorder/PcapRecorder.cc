//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/packet/recorder/PcapRecorder.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/recorder/PcapngWriter.h"
#include "inet/common/packet/recorder/PcapWriter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/common/Signal.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#endif

namespace inet {

// ----

Define_Module(PcapRecorder);

simsignal_t PcapRecorder::packetRecordedSignal = registerSignal("packetRecorded");

PcapRecorder::~PcapRecorder()
{
    delete pcapWriter;
    for (auto helper : helpers)
        delete helper;
}

PcapRecorder::PcapRecorder() : SimpleModule()
{
}

bool PcapRecorder::shouldDissectProtocolDataUnit(const Protocol *protocol)
{
    return !contains(dumpProtocols, protocol);
}

void PcapRecorder::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    if (!contains(dumpProtocols, protocol)) {
        if (dumpProtocol == nullptr)
            frontOffset += chunk->getChunkLength();
        else
            backOffset += chunk->getChunkLength();
    }
    else
        dumpProtocol = protocol;
}

void PcapRecorder::initialize()
{
    verbose = par("verbose");
    recordEmptyPackets = par("recordEmptyPackets");
    enableConvertingPackets = par("enableConvertingPackets");
    snaplen = this->par("snaplen");
    dumpBadFrames = par("dumpBadFrames");
    signalList.clear();
    packetFilter.setExpression(par("packetFilter").objectValue());

    {
        cStringTokenizer signalTokenizer(par("sendingSignalNames"));

        while (signalTokenizer.hasMoreTokens())
            signalList[registerSignal(signalTokenizer.nextToken())] = DIRECTION_OUTBOUND;
    }

    {
        cStringTokenizer signalTokenizer(par("receivingSignalNames"));

        while (signalTokenizer.hasMoreTokens())
            signalList[registerSignal(signalTokenizer.nextToken())] = DIRECTION_INBOUND;
    }

    {
        cStringTokenizer protocolTokenizer(par("dumpProtocols"));

        while (protocolTokenizer.hasMoreTokens())
            dumpProtocols.push_back(Protocol::getProtocol(protocolTokenizer.nextToken()));
    }

    {
        cStringTokenizer protocolTokenizer(par("helpers"));

        while (protocolTokenizer.hasMoreTokens())
            helpers.push_back(check_and_cast<IHelper *>(createOne(protocolTokenizer.nextToken())));
    }

    const char *moduleNames = par("moduleNamePatterns");
    cStringTokenizer moduleTokenizer(moduleNames);

    while (moduleTokenizer.hasMoreTokens()) {
        bool found = false;
        std::string mname(moduleTokenizer.nextToken());
        bool isAllIndex = (mname.length() > 3) && mname.rfind("[*]") == mname.length() - 3;

        if (isAllIndex)
            mname.replace(mname.length() - 3, 3, "");

        if (mname[0] == '.') {
            for (auto& elem : signalList)
                getParentModule()->subscribe(elem.first, this);
            found = true;
        }
        else {
            for (cModule::SubmoduleIterator i(getParentModule()); !i.end(); i++) {
                cModule *submod = *i;
                if (0 == strcmp(isAllIndex ? submod->getName() : submod->getFullName(), mname.c_str())) {
                    found = true;

                    for (auto& elem : signalList) {
                        if (!submod->isSubscribed(elem.first, this)) {
                            submod->subscribe(elem.first, this);
                            EV_INFO << "Subscribing to " << submod->getFullPath() << ":" << getSignalName(elem.first) << EV_ENDL;
                        }
                    }
                }
            }
        }

        if (!found && !isAllIndex)
            EV_INFO << "The module " << mname << (isAllIndex ? "[*]" : "") << " not found" << EV_ENDL;
    }

    const char *file = par("pcapFile");
    const char *fileFormat = par("fileFormat");
    int timePrecision = par("timePrecision");
    if (!strcmp(fileFormat, "pcap"))
        pcapWriter = new PcapWriter();
    else if (!strcmp(fileFormat, "pcapng"))
        pcapWriter = new PcapngWriter();
    else
        throw cRuntimeError("Unknown fileFormat parameter");
    recordPcap = *file != '\0';
    if (recordPcap) {
        pcapWriter->open(file, snaplen, timePrecision);
        pcapWriter->setFlush(par("alwaysFlush"));
    }

    WATCH(numRecorded);
}

void PcapRecorder::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not handle messages");
}

std::string PcapRecorder::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return std::to_string(numRecorded);
        default:
            return SimpleModule::resolveDirective(directive);   
    }
}

void PcapRecorder::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (pcapWriter->isOpen()) {
        auto i = signalList.find(signalID);
        ASSERT(i != signalList.end());
        Direction direction = i->second;
        if (false)
            ;
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
        else if (auto signal = dynamic_cast<const physicallayer::Signal *>(obj))
            recordPacket(signal->getEncapsulatedPacket(), direction, source);
#endif
        else if (auto packet = dynamic_cast<cPacket *>(obj))
            recordPacket(packet, direction, source);
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
        else if (auto transmission = dynamic_cast<const physicallayer::ITransmission *>(obj))
            recordPacket(transmission->getPacket(), direction, source);
        else if (auto reception = dynamic_cast<const physicallayer::IReception *>(obj))
            recordPacket(reception->getTransmission()->getPacket(), direction, source);
#endif
    }
}

void PcapRecorder::writePacket(const Protocol *protocol, const Packet *packet, b frontOffset, b backOffset, Direction direction, NetworkInterface *networkInterface)
{
    auto pcapLinkType = protocolToLinkType(protocol);
    if (pcapLinkType == LINKTYPE_INVALID)
        throw cRuntimeError("Cannot determine the PCAP link type from protocol '%s'", protocol->getName());
    bool convertPacket = !matchesLinkType(pcapLinkType, protocol);
    if (convertPacket) {
        packet = tryConvertToLinkType(packet, frontOffset, backOffset, pcapLinkType, protocol);
        if (packet == nullptr)
            throw cRuntimeError("The protocol '%s' doesn't match PCAP link type %d", protocol->getName(), pcapLinkType);
        frontOffset = b(0);
        backOffset = b(0);
    }
    b recordedLength = packet->getDataLength() - frontOffset - backOffset;
    if (recordEmptyPackets || recordedLength != b(0)) {
        pcapWriter->writePacket(simTime(), packet, frontOffset, backOffset, direction, networkInterface, pcapLinkType);
        numRecorded++;
        emit(packetRecordedSignal, packet);
    }
    if (convertPacket)
        delete packet;
}

void PcapRecorder::recordPacket(const cPacket *cpacket, Direction direction, cComponent *source)
{
    if (auto packet = dynamic_cast<const Packet *>(cpacket)) {
        EV_INFO << "Recording packet" << EV_FIELD(source, source->getFullPath()) << EV_FIELD(direction, direction) << EV_FIELD(packet) << EV_ENDL;
        if (verbose)
            EV_DEBUG << "Dumping packet" << EV_FIELD(packet, packetPrinter.printPacketToString(const_cast<Packet *>(packet), "%i")) << EV_ENDL;
        if (recordPcap && packetFilter.matches(packet) && (dumpBadFrames || !packet->hasBitError())) {
            // get Direction
            if (direction == DIRECTION_UNDEFINED) {
                if (auto directionTag = packet->findTag<DirectionTag>())
                    direction = directionTag->getDirection();
            }

            // get NetworkInterface
            auto srcModule = check_and_cast<cModule *>(source);
            auto networkInterface = findContainingNicModule(srcModule);
            if (networkInterface == nullptr) {
                int ifaceId = -1;
                if (direction == DIRECTION_OUTBOUND) {
                    if (auto ifaceTag = packet->findTag<InterfaceReq>())
                        ifaceId = ifaceTag->getInterfaceId();
                }
                else if (direction == DIRECTION_INBOUND) {
                    if (auto ifaceTag = packet->findTag<InterfaceInd>())
                        ifaceId = ifaceTag->getInterfaceId();
                }
                if (ifaceId != -1) {
                    auto ift = check_and_cast_nullable<InterfaceTable *>(getContainingNode(srcModule)->getSubmodule("interfaceTable"));
                    networkInterface = ift->getInterfaceById(ifaceId);
                }
            }

            const auto& packetProtocolTag = packet->getTag<PacketProtocolTag>();
            auto protocol = packetProtocolTag->getProtocol();
            if (contains(dumpProtocols, protocol))
                writePacket(protocol, packet, packetProtocolTag->getFrontOffset(), packetProtocolTag->getBackOffset(), direction, networkInterface);
            else {
                frontOffset = b(0);
                backOffset = b(0);
                dumpProtocol = nullptr;
                Packet dissectedPacket(*packet);
                PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), *this);
                packetDissector.dissectPacket(&dissectedPacket);
                if (dumpProtocol != nullptr)
                    writePacket(dumpProtocol, packet, frontOffset, backOffset, direction, networkInterface);
            }
        }
    }
}

void PcapRecorder::finish()
{
    pcapWriter->close();
}

bool PcapRecorder::matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const
{
    if (protocol == nullptr)
        return false;
    else if (*protocol == Protocol::ethernetPhy)
        return pcapLinkType == LINKTYPE_ETHERNET_MPACKET;
    else if (*protocol == Protocol::ethernetMac)
        return pcapLinkType == LINKTYPE_ETHERNET;
    else if (*protocol == Protocol::ppp)
        return pcapLinkType == LINKTYPE_PPP_WITH_DIR;
    else if (*protocol == Protocol::ieee80211Mac)
        return pcapLinkType == LINKTYPE_IEEE802_11;
    else if (*protocol == Protocol::ipv4)
        return pcapLinkType == LINKTYPE_RAW || pcapLinkType == LINKTYPE_IPV4;
    else if (*protocol == Protocol::ipv6)
        return pcapLinkType == LINKTYPE_RAW || pcapLinkType == LINKTYPE_IPV6;
    else if (*protocol == Protocol::ieee802154)
        return pcapLinkType == LINKTYPE_IEEE802_15_4 || pcapLinkType == LINKTYPE_IEEE802_15_4_NOFCS;
    else {
        for (auto helper : helpers) {
            if (helper->matchesLinkType(pcapLinkType, protocol))
                return true;
        }
    }
    return false;
}

PcapLinkType PcapRecorder::protocolToLinkType(const Protocol *protocol) const
{
    if (*protocol == Protocol::ethernetPhy)
        return LINKTYPE_ETHERNET_MPACKET;
    else if (*protocol == Protocol::ethernetMac)
        return LINKTYPE_ETHERNET;
    else if (*protocol == Protocol::ppp)
        return LINKTYPE_PPP_WITH_DIR;
    else if (*protocol == Protocol::ieee80211Mac)
        return LINKTYPE_IEEE802_11;
    else if (*protocol == Protocol::ipv4 || *protocol == Protocol::ipv6)
        return LINKTYPE_RAW;
    else if (*protocol == Protocol::ieee802154)
        return LINKTYPE_IEEE802_15_4;
    else {
        for (auto helper : helpers) {
            auto lt = helper->protocolToLinkType(protocol);
            if (lt != LINKTYPE_INVALID)
                return lt;
        }
    }
    return LINKTYPE_INVALID;
}

Packet *PcapRecorder::tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset, PcapLinkType pcapLinkType, const Protocol *protocol) const
{
    if (enableConvertingPackets) {
        for (IHelper *helper : helpers) {
            if (auto newPacket = helper->tryConvertToLinkType(packet, frontOffset, backOffset, pcapLinkType, protocol))
                return newPacket;
        }
    }
    return nullptr;
}

} // namespace inet

