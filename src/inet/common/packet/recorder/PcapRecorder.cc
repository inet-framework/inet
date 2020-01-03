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

#include "inet/common/packet/recorder/PcapRecorder.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/common/stlutils.h"

namespace inet {

//----

Define_Module(PcapRecorder);

simsignal_t PcapRecorder::packetRecordedSignal = registerSignal("packetRecorded");

PcapRecorder::~PcapRecorder()
{
    for (auto helper : helpers)
        delete helper;
}

PcapRecorder::PcapRecorder() : cSimpleModule(), pcapWriter()
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
            signalList[registerSignal(signalTokenizer.nextToken())] = true;
    }

    {
        cStringTokenizer signalTokenizer(par("receivingSignalNames"));

        while (signalTokenizer.hasMoreTokens())
            signalList[registerSignal(signalTokenizer.nextToken())] = false;
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

        if (!found) {
            EV << "The module " << mname << (isAllIndex ? "[*]" : "")
               << " not found for PcapRecorder " << getFullPath() << endl;
        }
    }

    pcapLinkType = (PcapLinkType)par("pcapLinkType").intValue();
    const char *file = par("pcapFile");
    recordPcap = *file != '\0';
    if (recordPcap && pcapLinkType != LINKTYPE_INVALID) {
        pcapWriter.openPcap(file, snaplen, pcapLinkType);
        pcapWriter.setFlushParameter(par("alwaysFlush").boolValue());
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
        bool l2r = (i != signalList.end()) ? i->second : true;
        recordPacket(packet, l2r);
    }
}

void PcapRecorder::recordPacket(const cPacket *msg, bool l2r)
{
    EV << "PcapRecorder::recordPacket(" << msg->getFullPath() << ", " << l2r << ")\n";
    packetDumper.dumpPacket(l2r, msg);

    if (!recordPcap)
        return;

    auto packet = dynamic_cast<const Packet *>(msg);

    if (packet && packetFilter.matches(packet) && (dumpBadFrames || !packet->hasBitError())) {
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (contains(dumpProtocols, protocol)) {
            if (pcapLinkType == LINKTYPE_INVALID) {
                ASSERT(!pcapWriter.isOpen());
                pcapLinkType = protocolToLinkType(protocol);
                if (pcapLinkType == LINKTYPE_INVALID)
                    throw cRuntimeError("Cannot determine the PCAP link type from protocol '%s', specify it in the pcapLinkType parameter", protocol->getName());
                pcapWriter.openPcap(par("pcapFile"), snaplen, pcapLinkType);
                pcapWriter.setFlushParameter(par("alwaysFlush").boolValue());
            }
            if (!matchesLinkType(protocol)) {
                auto convertedPacket = tryConvertToLinkType(packet, protocol);
                if (convertedPacket) {
                    pcapWriter.writePacket(simTime(), convertedPacket);
                    numRecorded++;
                    emit(packetRecordedSignal, packet);
                    delete convertedPacket;
                    return;
                }
                throw cRuntimeError("The protocol '%s' doesn't match PCAP link type %d", protocol->getName(), pcapLinkType);
            }
            pcapWriter.writePacket(simTime(), packet);
            numRecorded++;
            emit(packetRecordedSignal, packet);
        }
    }
}

void PcapRecorder::finish()
{
    packetDumper.dump("", "pcapRecorder finished");
    pcapWriter.closePcap();
}

bool PcapRecorder::matchesLinkType(const Protocol *protocol) const
{
    if (protocol == nullptr)
        return false;
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
    if (*protocol == Protocol::ethernetMac)
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

Packet *PcapRecorder::tryConvertToLinkType(const Packet* packet, const Protocol *protocol) const
{
    for (IHelper *helper: helpers) {
        if (auto newPacket = helper->tryConvertToLinkType(packet, pcapLinkType, protocol))
            return newPacket;
    }
    return nullptr;
}

} // namespace inet

