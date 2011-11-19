//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
//               2011 Zoltan Bojthe
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


#include "TCPDump.h"

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#endif

//----

Define_Module(TCPDump);

TCPDump::TCPDump() : cSimpleModule(), tcpdump()
{
}

TCPDump::~TCPDump()
{
}

void TCPDump::initialize()
{
    const char* file = this->par("dumpFile");

    dumpBadFrames = par("dumpBadFrames").boolValue();
    dropBadFrames = par("dropBadFrames").boolValue();

    snaplen = this->par("snaplen");
    tcpdump.setVerbose(par("verbose").boolValue());
    tcpdump.setOutStream(ev.getOStream());

    if (*file)
        pcapDump.openPcap(file, snaplen);
}

void TCPDump::handleMessage(cMessage *msg)
{
    if (!ev.isDisabled() && msg->isPacket())
    {
        bool l2r = msg->arrivedOn("hlIn");
        tcpdump.dumpPacket(l2r, PK(msg));
    }

#ifdef WITH_IPv4
    if (pcapDump.isOpen() && dynamic_cast<IPv4Datagram *>(msg)
            && (dumpBadFrames || !PK(msg)->hasBitError()))
    {
        const simtime_t stime = simulation.getSimTime();
        IPv4Datagram *ipPacket = check_and_cast<IPv4Datagram *>(msg);
        pcapDump.writeFrame(stime, ipPacket);
    }
#endif

    if (PK(msg)->hasBitError() && dropBadFrames)
    {
        delete msg;
        return;
    }

    // forward
    int32 index = msg->getArrivalGate()->getIndex();
    int32 id;

    if (msg->getArrivalGate()->isName("ifIn"))
        id = findGate("hlOut", index);
    else
        id = findGate("ifOut", index);

    send(msg, id);
}

void TCPDump::finish()
{
    tcpdump.dump("", "tcpdump finished");
    pcapDump.closePcap();
}

