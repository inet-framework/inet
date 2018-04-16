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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/recorder/PcapRecorder.h"

namespace inet {

//----

Define_Module(PcapRecorder);

PcapRecorder::~PcapRecorder()
{
}

PcapRecorder::PcapRecorder() : cSimpleModule(), pcapDumper()
{
}

void PcapRecorder::initialize()
{
    const char *file = par("pcapFile");
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

    if (*file) {
        pcapDumper.openPcap(file, snaplen, par("pcapNetwork"));
        pcapDumper.setFlushParameter(par("alwaysFlush").boolValue());
    }
}

void PcapRecorder::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not handle messages");
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

void PcapRecorder::recordPacket(cPacket *msg, bool l2r)
{
    EV << "PcapRecorder::recordPacket(" << msg->getFullPath() << ", " << l2r << ")\n";
    packetDumper.dumpPacket(l2r, msg);

    if (!pcapDumper.isOpen())
        return;

    auto packet = dynamic_cast<Packet *>(msg);

    if (packet && packetFilter.matches(packet) && (dumpBadFrames || !packet->hasBitError())) {
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        for (auto dumpProtocol : dumpProtocols) {
            if (protocol == dumpProtocol) {
                pcapDumper.writePacket(simTime(), packet);
                break;
            }
        }
    }
}

void PcapRecorder::finish()
{
    packetDumper.dump("", "pcapRecorder finished");
    pcapDumper.closePcap();
}

} // namespace inet

