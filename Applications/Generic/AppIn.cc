//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#include <omnetpp.h>
#include "AppIn.h"
#include "IPControlInfo_m.h"
#include "IPDatagram.h"


Define_Module( AppIn );

void AppIn::initialize()
{
}

void AppIn::activity()
{
    while(true)
    {
        cMessage *msg = receive();
        processMessage(msg);
    }
}

// private function
void AppIn::processMessage(cMessage *msg)
{
    simtime_t arrivalTime = msg->arrivalTime();
    int content = msg->hasPar("content") ? (int)msg->par("content") : (int)-1;
    int length = msg->length();
    IPControlInfo *controlInfo = check_and_cast<IPControlInfo *>(msg->removeControlInfo());

    // print out Packet info
    ev  << "AppIn: Packet received:\n"
        << " Cont: " << content
        << " Bitlen: " << length
        << "   Arrival Time: " << arrivalTime
        << " Simtime: " << simTime()
        << "\nSrc: " << controlInfo->srcAddr()
        << " Dest: " << controlInfo->destAddr() << "\n\n";

    delete controlInfo;
    delete msg;
}

