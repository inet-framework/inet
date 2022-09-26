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
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/common/packet/recorder/PcapWriter.h"
#include "inet/common/packet/recorder/PcapngWriter.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"

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

PcapRecorder::PcapRecorder() : cSimpleModule()
{
}

void PcapRecorder::initialize()
{
    verbose = par("verbose");
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
    if (!strcmp(fileFormat, "pcap"))
        pcapWriter = new PcapWriter();
    else if (!strcmp(fileFormat, "pcapng"))
        pcapWriter = new PcapngWriter();
    else
        throw cRuntimeError("Unknown fileFormat parameter");
    recordPcap = *file != '\0';
    if (recordPcap) {
        pcapWriter->open(file, snaplen);
        pcapWriter->setFlush(par("alwaysFlush"));
    }

    WATCH(numRecorded);
}

void PcapRecorder::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not handle messages");
}

void PcapRecorder::refreshDisplay() const
{
    updateDisplayString();
}

std::string PcapRecorder::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return std::to_string(numRecorded);
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void PcapRecorder::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

void PcapRecorder::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (pcapWriter->isOpen()) {
        cPacket *packet = dynamic_cast<cPacket *>(obj);

        if (packet) {
            auto i = signalList.find(signalID);
            Direction direction = (i != signalList.end()) ? i->second : DIRECTION_UNDEFINED;
            recordPacket(packet, direction, source);
        }
    }
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
            if (packetProtocolTag->getFrontOffset() == b(0) && packetProtocolTag->getBackOffset() == b(0) && contains(dumpProtocols, protocol)) {
                auto pcapLinkType = protocolToLinkType(protocol);
                if (pcapLinkType == LINKTYPE_INVALID)
                    throw cRuntimeError("Cannot determine the PCAP link type from protocol '%s'", protocol->getName());

                if (matchesLinkType(pcapLinkType, protocol)) {
                    pcapWriter->writePacket(simTime(), packet, direction, networkInterface, pcapLinkType);
                    numRecorded++;
                    emit(packetRecordedSignal, packet);
                }
                else {
                    if (auto convertedPacket = tryConvertToLinkType(packet, pcapLinkType, protocol)) {
                        pcapWriter->writePacket(simTime(), convertedPacket, direction, networkInterface, pcapLinkType);
                        numRecorded++;
                        emit(packetRecordedSignal, packet);
                        delete convertedPacket;
                    }
                    else
                        throw cRuntimeError("The protocol '%s' doesn't match PCAP link type %d", protocol->getName(), pcapLinkType);
                }
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

Packet *PcapRecorder::tryConvertToLinkType(const Packet *packet, PcapLinkType pcapLinkType, const Protocol *protocol) const
{
    for (IHelper *helper : helpers) {
        if (auto newPacket = helper->tryConvertToLinkType(packet, pcapLinkType, protocol))
            return newPacket;
    }
    return nullptr;
}

} // namespace inet

