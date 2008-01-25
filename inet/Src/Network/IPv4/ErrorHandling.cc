//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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


//  Cleanup and rewrite: Andras Varga, 2004

#include <omnetpp.h>
#include "ErrorHandling.h"
#include "IPControlInfo.h"
#include "IPDatagram.h"
#include "ICMPMessage.h"


Define_Module(ErrorHandling);

void ErrorHandling::initialize()
{
    numReceived = 0;
    WATCH(numReceived);
}

void ErrorHandling::handleMessage(cMessage *msg)
{
    numReceived++;

    ICMPMessage *icmpMsg = check_and_cast<ICMPMessage *>(msg);
    IPDatagram *d = check_and_cast<IPDatagram *>(icmpMsg->encapsulatedMsg());

    EV << "Error Handler: ICMP message received:\n";
    EV << " Type: " << (int)icmpMsg->getType()
       << " Code: " << (int)icmpMsg->getCode()
       << " Bytelength: " << d->byteLength()
       << " Src: " << d->srcAddress()
       << " Dest: " << d->destAddress()
       << " Time: " << simTime()
       << "\n";

    delete icmpMsg;

    char buf[80];
    sprintf(buf, "errors: %ld", numReceived);
    displayString().setTagArg("t",0,buf);
}

