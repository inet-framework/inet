//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/DirectionTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/common/StringFormat.h"
#include "inet/common/packet/recorder/PcapngWriter.h"
#include "inet/common/packet/recorder/PcapRecorder.h"
#include "inet/common/packet/recorder/PcapWriter.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

//----

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
    snaplen = this->par("snaplen");
    dumpBadFrames = par("dumpBadFrames");
    packetDumper.setVerbose(par("verbose"));
    packetDumper.setOutStream(EVSTREAM);
    signalList.clear();
    packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));

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
            for (auto & elem : signalList)
                getParentModule()->subscribe(elem.first, this);
            found = true;
        }
        else {
            for (cModule::SubmoduleIterator i(getParentModule()); !i.end(); i++) {
                cModule *submod = *i;
                if (0 == strcmp(isAllIndex ? submod->getName() : submod->getFullName(), mname.c_str())) {
                    found = true;

                    for (auto & elem : signalList) {
                        if (!submod->isSubscribed(elem.first, this)) {
                            submod->subscribe(elem.first, this);
                            EV << "PcapRecorder " << getFullPath() << " subscribed to "
                               << submod->getFullPath() << ":" << getSignalName(elem.first) << endl;
                        }
                    }
                }
            }
        }

        if (!found && !isAllIndex) {
            EV << "The module " << mname << (isAllIndex ? "[*]" : "")
               << " not found for PcapRecorder " << getFullPath() << endl;
        }
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

void PcapRecorder::updateDisplayString() const
{
    auto text = StringFormat::formatString(par("displayStringTextFormat"), [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'n':
                result = std::to_string(numRecorded);
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

void PcapRecorder::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    cPacket *packet = dynamic_cast<cPacket *>(obj);

    if (packet) {
        SignalList::const_iterator i = signalList.find(signalID);
        Direction direction = (i != signalList.end()) ? i->second : DIRECTION_UNDEFINED;
        recordPacket(packet, direction, source);
    }
}

void PcapRecorder::recordPacket(const cPacket *msg, Direction direction, cComponent *source)
{
    EV_DEBUG << "PcapRecorder::recordPacket(" << msg->getFullPath() << ", " << direction << ")\n";
    packetDumper.dumpPacket(direction == DIRECTION_OUTBOUND, msg);

    if (!recordPcap)
        return;

    auto packet = dynamic_cast<const Packet *>(msg);

    if (packet && packetFilter.matches(packet) && (dumpBadFrames || !packet->hasBitError())) {
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();

        // get Direction
        if (direction == DIRECTION_UNDEFINED) {
            if (auto directionTag = packet->findTag<DirectionTag>())
                direction = directionTag->getDirection();
        }

        // get InterfaceEntry
        auto srcModule = check_and_cast<cModule *>(source);
        auto interfaceEntry = findContainingNicModule(srcModule);
        if (interfaceEntry == nullptr) {
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
                interfaceEntry = ift->getInterfaceById(ifaceId);
            }
        }

        if (contains(dumpProtocols, protocol)) {
            auto pcapLinkType = protocolToLinkType(protocol);
            if (pcapLinkType == LINKTYPE_INVALID)
                throw cRuntimeError("Cannot determine the PCAP link type from protocol '%s'", protocol->getName());

            if (!pcapWriter->isOpen()) {
                pcapWriter->open(par("pcapFile"), snaplen);
                pcapWriter->setFlush(par("alwaysFlush"));
            }

            if (matchesLinkType(pcapLinkType, protocol)) {
                pcapWriter->writePacket(simTime(), packet, direction, interfaceEntry, pcapLinkType);
                numRecorded++;
                emit(packetRecordedSignal, packet);
            }
            else {
                if (auto convertedPacket = tryConvertToLinkType(packet, pcapLinkType, protocol)) {
                    pcapWriter->writePacket(simTime(), convertedPacket, direction, interfaceEntry, pcapLinkType);
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

void PcapRecorder::finish()
{
    packetDumper.dump("", "pcapRecorder finished");
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
        for (auto helper: helpers) {
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
    else if (*protocol == Protocol::ipv4)
        return LINKTYPE_RAW;
    else if (*protocol == Protocol::ipv6)
        return LINKTYPE_RAW;
    else if (*protocol == Protocol::ieee802154)
        return LINKTYPE_IEEE802_15_4;
    else {
        for (auto helper: helpers) {
            auto lt = helper->protocolToLinkType(protocol);
            if (lt != LINKTYPE_INVALID)
                return lt;
        }
    }
    return LINKTYPE_INVALID;
}

Packet *PcapRecorder::tryConvertToLinkType(const Packet* packet, PcapLinkType pcapLinkType, const Protocol *protocol) const
{
    for (IHelper *helper: helpers) {
        if (auto newPacket = helper->tryConvertToLinkType(packet, pcapLinkType, protocol))
            return newPacket;
    }
    return nullptr;
}

} // namespace inet

