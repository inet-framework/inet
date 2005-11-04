//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "NAMTrace.h"
#include "NAMTraceWriter.h"
#include "NotificationBoard.h"



Define_Module(NAMTraceWriter);


void NAMTraceWriter::initialize(int stage)
{
    // all initialization is done in the first stage
    if (stage!=0)
        return;

    // subscribe to the interesting notifications
    NotificationBoard *nb = NotificationBoardAccess().get();
    nb->subscribe(this, NF_HOST_FAILURE);
    nb->subscribe(this, NF_HOST_RECOVERY);
    nb->subscribe(this, NF_PP_TX_BEGIN);
    nb->subscribe(this, NF_PP_TX_END);
    nb->subscribe(this, NF_L2_Q_DROP);

    // get pointer to the NAMTrace module
    nt = check_and_cast<NAMTrace*>(simulation.moduleByPath("nam"));

    // register given namid, or allocate one (if -1 was configured)
    int namid0 = par("namid");
    cModule *node = parentModule();  // the host or router
    namid = nt->assignNamId(node, namid0);
    if (namid0==-1)
        par("namid") = namid;
}

NAMTraceWriter::~NAMTraceWriter()
{
}

void NAMTraceWriter::receiveChangeNotification(int category, cPolymorphic *details)
{
/* FIXME where to get peernamid? from InterfaceEntry?
    switch(category)
    {
        case NF_HOST_FAILURE: break;
        case NF_HOST_RECOVERY: break;
        case NF_PP_TX_BEGIN: recordPacketEvent('h', peer, msg); break;
        case NF_PP_TX_END: recordPacketEvent('e', peer, msg); break;
        case NF_L2_Q_DROP: recordPacketEvent('d', peer, msg); break;
    }
*/
}

#if 0
void NAMTraceWriter::traceInit()
{
    // node entry
    recordNodeEvent("UP", "circle");

    // link entry
    // LINKS HAVE TO BE REGISTERED ONLY ON ONE END!!!
    *nams << "l -t * -s " << namid << " -d " << peernamid << " -S UP -r " <<
            (int)bandwidth << " -D " << delay << endl;

    // queue entry
    *nams << "q -t * -s " << namid << " -d " << peernamid << " -a 0 " << endl;
}

void NAMTraceWriter::recordNodeEvent(char *state, char *shape)
{
    std::ostream& out = nt->log();
    *nams << "n -t ";
    if (simTime() == 0.0)
        *nams << "*";
    else
        *nams << simTime();
    *nams << " -s " << namid << " -a " << namid << " -S " << state << " -v " << shape << endl;
}

void NAMTraceWriter::recordLinkEvent(InterfaceEntry *ie)
{
    std::ostream& out = nt->log();
}
#endif

void NAMTraceWriter::recordPacketEvent(const char event, cModule *peer, cMessage *msg)
{
    std::ostream& out = nt->log();
    int peernamid = nt->getNamId(peer);

    int size = msg->byteLength();

    out << event << " -t " << simTime() << " -s " << namid << " -d " << peernamid << " -e " << size;

    cMessage *em = msg;
    while (em)
    {
        if (em->hasPar("color"))
        {
            out << " -a " << em->par("color").longValue();
            break;
        }
        em = em->encapsulatedMsg();
    }

    out << endl;
}

