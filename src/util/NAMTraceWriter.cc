//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "NAMTrace.h"
#include "NAMTraceWriter.h"
#include "NotificationBoard.h"
#include "TxNotifDetails.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Module(NAMTraceWriter);


void NAMTraceWriter::initialize(int stage)
{
    if (stage==1)  // let NAMTrace module initialize in stage 0
    {
        // get pointer to the NAMTrace module
        cModule *namMod = simulation.getModuleByPath("nam");
        if (!namMod)
        {
            nt = NULL;
            EV << "NAMTraceWriter: nam module not found, no trace will be written\n";
            return;
        }

        // store ptr to namtrace module
        nt = check_and_cast<NAMTrace*>(namMod);

        // register given namid; -1 means autoconfigure
        int namid0 = par("namid");
        cModule *node = getParentModule();  // the host or router
        namid = nt->assignNamId(node, namid0);
        if (namid0==-1)
            par("namid") = namid;  // let parameter reflect autoconfigured namid

        // write "node" entry to the trace
        if (nt->isEnabled())
            recordNodeEvent("UP", "circle");

        // subscribe to the interesting notifications
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_NODE_FAILURE);
        nb->subscribe(this, NF_NODE_RECOVERY);
        nb->subscribe(this, NF_PP_TX_BEGIN);
        nb->subscribe(this, NF_PP_RX_END);
        nb->subscribe(this, NF_L2_Q_DROP);
    }
    else if (stage==2 && nt!=NULL && nt->isEnabled())
    {
        // write "link" entries
        IInterfaceTable *ift = InterfaceTableAccess().get();
        cModule *node = getParentModule();  // the host or router
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            // skip loopback interfaces
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isLoopback()) continue;
            if (!ie->isPointToPoint()) continue; // consider pont-to-point links only

            // fill in peerNamIds in InterfaceEntries
            cGate *outgate = node->gate(ie->getNodeOutputGateId());
            if (!outgate || !outgate->getNextGate()) continue;
            cModule *peernode = outgate->getNextGate()->getOwnerModule(); // FIXME not entirely correct: what if a subnet is "boxed"?
            cModule *peerwriter = peernode->getSubmodule("namTrace");
            if (!peerwriter) error("module %s doesn't have a submodule named namTrace", peernode->getFullPath().c_str());
            int peernamid = peerwriter->par("namid");
            ie->setPeerNamId(peernamid);

            // find delay
            simtime_t delay = 0;
            cDatarateChannel *chan = dynamic_cast<cDatarateChannel*>(outgate->getChannel());
            if (chan) delay = chan->getDelay();

            // write link entry into trace
            recordLinkEvent(peernamid, ie->getDatarate(), delay, "UP");
        }
    }
}

NAMTraceWriter::~NAMTraceWriter()
{
/*FIXME this will crash if the "nt" module gets cleaned up sooner than this one
    if (nt && nt->isEnabled())
    {
        recordNodeEvent("DOWN", "circle");
    }
*/
}


void NAMTraceWriter::receiveChangeNotification(int category, const cPolymorphic *details)
{
    // don't do anything if global NAMTrace module doesn't exist or does not have a file open
    if (!nt || !nt->isEnabled())
        return;

    printNotificationBanner(category, details);

    // process notification
    if (category==NF_PP_TX_BEGIN || category==NF_PP_RX_END || category==NF_L2_Q_DROP)
    {
        TxNotifDetails *d = check_and_cast<TxNotifDetails *>(details);
        int peernamid = d->getInterfaceEntry()->getPeerNamId();
        cPacket *msg = d->getPacket();

        switch(category)
        {
            case NF_PP_TX_BEGIN: recordPacketEvent('h', peernamid, msg); break;
            case NF_PP_RX_END:   recordPacketEvent('r', peernamid, msg); break;
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

void NAMTraceWriter::recordNodeEvent(const char *state, const char *shape)
{
    ASSERT(nt && nt->isEnabled());
    std::ostream& out = nt->out();
    out << "n -t ";
    if (simTime() == 0.0)
        out << "*";
    else
        out << simTime();
    out << " -s " << namid << " -a " << namid << " -S " << state << " -v " << shape << endl;
}

void NAMTraceWriter::recordLinkEvent(int peernamid, double datarate, simtime_t delay, const char *state)
{
    ASSERT(nt && nt->isEnabled());
    std::ostream& out = nt->out();

    // link entry (to be registered ON ONE END ONLY! This also means that
    // ns2 thinks that datarate and delay must be the same in both directions)
    if (namid < peernamid)
        out << "l -t * -s " << namid << " -d " << peernamid
            << " -S " << state << " -r " << (int)datarate << " -D " << delay << endl;

    // queue entry
    out << "q -t * -s " << namid << " -d " << peernamid << " -a 0 " << endl;
}

void NAMTraceWriter::recordPacketEvent(char event, int peernamid, cPacket *msg)
{
    ASSERT(nt && nt->isEnabled());
    std::ostream& out = nt->out();

    int size = msg->getByteLength();
    int color = 0;
    for (cPacket *em = msg; em; em = em->getEncapsulatedPacket())
        if (em->hasPar("color"))
            {color = em->par("color").longValue(); break;}

    out << event << " -t " << simTime();
    if (event=='h')
        out << " -s " << namid << " -d " << peernamid;
    else
        out << " -s " << peernamid << " -d " << namid;

    out << " -e " << size << " -a " << color << endl;
}

