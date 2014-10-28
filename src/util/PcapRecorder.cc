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


#include "PcapRecorder.h"

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#endif

#ifdef WITH_IPv6
#include "IPv6Datagram.h"
#endif


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
    const char* file = par("pcapFile");
    snaplen = this->par("snaplen");
    dumpBadFrames = par("dumpBadFrames").boolValue();
    packetDumper.setVerbose(par("verbose").boolValue());
    packetDumper.setOutStream(EVSTREAM);
    signalList.clear();

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

    const char* moduleNames = par("moduleNamePatterns");
    cStringTokenizer moduleTokenizer(moduleNames);

    while (moduleTokenizer.hasMoreTokens())
    {
        bool found = false;
        std::string mname(moduleTokenizer.nextToken());
        bool isAllIndex = (mname.length() > 3) && mname.rfind("[*]") == mname.length() - 3;

        if (isAllIndex)
            mname.replace(mname.length() - 3, 3, "");

        for (cModule::SubmoduleIterator i(getParentModule()); !i.end(); i++)
        {
            cModule *submod = i();
            if (0 == strcmp(isAllIndex ? submod->getName() : submod->getFullName(), mname.c_str()))
            {
                found = true;

                for (SignalList::iterator s = signalList.begin(); s != signalList.end(); s++)
                {
                    if (!submod->isSubscribed(s->first, this))
                    {
                        submod->subscribe(s->first, this);
                        EV << "PcapRecorder " << getFullPath() << " subscribed to "
                           << submod->getFullPath() << ":" << getSignalName(s->first) << endl;
                    }
                }
            }
        }

        if (!found)
        {
            EV << "The module " << mname << (isAllIndex ? "[*]" : "")
                    << " not found for PcapRecorder " << getFullPath() << endl;
        }
    }

    if (*file)
        pcapDumper.openPcap(file, snaplen);
}

void PcapRecorder::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not handle messages");
}

void PcapRecorder::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    cPacket *packet = dynamic_cast<cPacket *>(obj);

    if (packet)
    {
        SignalList::const_iterator i = signalList.find(signalID);
        bool l2r = (i != signalList.end()) ? i->second : true;
        recordPacket(packet, l2r);
    }
}

void PcapRecorder::recordPacket(cPacket *msg, bool l2r)
{
    if (!ev.isDisabled())
    {
        EV << "PcapRecorder::recordPacket(" << msg->getFullPath() << ", " << l2r << ")\n";
        packetDumper.dumpPacket(l2r, msg);
    }

#if defined(WITH_IPv4) || defined(WITH_IPv6)
    if (!pcapDumper.isOpen())
        return;

    bool hasBitError = false;

#ifdef WITH_IPv4
    IPv4Datagram *ip4Packet = NULL;
#endif
#ifdef WITH_IPv6
    IPv6Datagram *ip6Packet = NULL;
#endif
    while (msg)
    {
        if (msg->hasBitError())
            hasBitError = true;
#ifdef WITH_IPv4
        if (NULL != (ip4Packet = dynamic_cast<IPv4Datagram *>(msg))) {
            break;
        }
#endif
#ifdef WITH_IPv6
        if (NULL != (ip6Packet = dynamic_cast<IPv6Datagram *>(msg))) {
            break;
        }
#endif

        msg = msg->getEncapsulatedPacket();
    }
#endif
#ifdef WITH_IPv4
    if (ip4Packet && (dumpBadFrames || !hasBitError))
    {
        const simtime_t stime = simulation.getSimTime();
        pcapDumper.writeFrame(stime, ip4Packet);
    }
#endif
#ifdef WITH_IPv6
    if (ip6Packet && (dumpBadFrames || !hasBitError))
    {
        const simtime_t stime = simulation.getSimTime();
        pcapDumper.writeIPv6Frame(stime, ip6Packet);
    }
#endif
}

void PcapRecorder::finish()
{
     packetDumper.dump("", "pcapRecorder finished");
     pcapDumper.closePcap();
}

