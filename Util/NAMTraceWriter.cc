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
#include "NAMTraceWriter.h"
#include "NotificationBoard.h"



Define_Module(NAMTraceWriter);


void NAMTraceWriter::initialize(int stage)
{
    // all initialization is done in the first stage
    if (stage!=0)
        return;

    namid = par("namid");

    NotificationBoard *nb = NotificationBoardAccess().get();
    nb->subscribe(this, NF_HOST_FAILURE);
    nb->subscribe(this, NF_HOST_RECOVERY);
    nb->subscribe(this, NF_PP_TX_BEGIN);
    nb->subscribe(this, NF_PP_TX_END);
    nb->subscribe(this, NF_PP_RX_END);
    nb->subscribe(this, NF_L2_Q_DROP);
}

NAMTraceWriter::~NAMTraceWriter()
{
}

void NAMTraceWriter::receiveChangeNotification(int category, cPolymorphic *details)
{
}

#if 0
void NAMTraceWriter::traceInit()
{
    // node entry
    *nams << "n -t ";
    if (simTime() == 0.0)
        *nams << "*";
    else
        *nams << simTime();
    *nams << " -s " << namid << " -a " << namid << " -S UP -v circle" << endl;

    // link entry
    *nams << "l -t * -s " << namid << " -d " << peerid << " -S UP -r " <<
            (int)bandwidth << " -D " << delay << endl;

    // queue entry
    *nams << "q -t * -s " << namid << " -d " << peerid << " -a 0 " << endl;
    *nams << "q -t * -s " << peerid << " -d " << namid << " -a 0 " << endl;
}

void NAMTraceWriter::tracePacket(const char event, cMessage *msg)
{
    if(!nams)
        return;

    int size = msg->length() / 8;

    *nams << event << " -t " << simTime() << " -s " << namid << " -d " <<
            peerid << " -e " << size;

    cMessage *em = msg;
    while(em)
    {
        if(em->hasPar("color"))
        {
            *nams << " -a " << em->par("color").longValue();
            break;
        }
        em = em->encapsulatedMsg();
    }

    *nams << endl;
}
#endif

