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
#include "TxNotifDetails.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Module(NAMTraceWriter);


void NAMTraceWriter::initialize(int stage)
{
    if (stage==0)
    {
        // subscribe to the interesting notifications
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_NODE_FAILURE);
        nb->subscribe(this, NF_NODE_RECOVERY);
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
    else if (stage==1)
    {
        // fill in peerNamIds in InterfaceEntries

        // write "node" entry to the trace
        recordNodeEvent("UP", "circle");

        // write "link" entries
        InterfaceTable *ift = InterfaceTableAccess().get();
        for (int i=0; i<ift->numInterfaces(); i++)
        {
            InterfaceEntry *ie = ift->interfaceAt(i);
            if (!ie->isLoopback())
                recordLinkEvent(ie, "UP");
        }
    }
}

NAMTraceWriter::~NAMTraceWriter()
{
}

void NAMTraceWriter::receiveChangeNotification(int category, cPolymorphic *details)
{
    if (category==NF_PP_TX_BEGIN || category==NF_PP_TX_END || category==NF_L2_Q_DROP)
    {
        TxNotifDetails *d = check_and_cast<TxNotifDetails *>(details);
        int peernamid = d->interfaceEntry()->peerNamId();
        cMessage *msg = d->message();

        switch(category)
        {
            case NF_PP_TX_BEGIN: recordPacketEvent('h', peernamid, msg); break;
            case NF_PP_TX_END:   recordPacketEvent('e', peernamid, msg); break;
            case NF_L2_Q_DROP:   recordPacketEvent('d', peernamid, msg); break;
        }
    }
    else
    {
        switch(category)
        {
            case NF_NODE_FAILURE: break; // TODO
            case NF_NODE_RECOVERY: break; // TODO
        }
    }
}

void NAMTraceWriter::recordNodeEvent(char *state, char *shape)
{
    std::ostream& out = nt->out();
    out << "n -t ";
    if (simTime() == 0.0)
        out << "*";
    else
        out << simTime();
    out << " -s " << namid << " -a " << namid << " -S " << state << " -v " << shape << endl;
}

void NAMTraceWriter::recordLinkEvent(InterfaceEntry *ie, char *state)
{
    std::ostream& out = nt->out();
    int peernamid = ie->peerNamId();

    double delay = 1e-3; // FIXME!!!!!!

    // link entry (to be registered ON ONE END ONLY!)
    if (namid < peernamid)
        out << "l -t * -s " << namid << " -d " << peernamid
            << " -S " << state << " -r " << (int)ie->datarate() << " -D " << delay << endl;

    // queue entry
    out << "q -t * -s " << namid << " -d " << peernamid << " -a 0 " << endl;
}

void NAMTraceWriter::recordPacketEvent(const char event, int peernamid, cMessage *msg)
{
    std::ostream& out = nt->out();

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

