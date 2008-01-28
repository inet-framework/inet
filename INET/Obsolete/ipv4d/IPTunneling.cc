//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga

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
#include <string.h>

#include "IPTunneling.h"
#include "IPControlInfo_m.h"


Define_Module(IPTunneling);


void IPTunneling::handleMessage(cMessage *msg)
{
    IPDatagram *dgram0 = (IPDatagram *)(msg->parList().get("datagram"));  // FIXME cPar!!!
    IPDatagram *datagram = new IPDatagram(*dgram0);
    IPAddress dest = msg->par("destination_address").stringValue();
    delete msg;

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setProtocol(IP_PROT_IP);
    controlInfo->setDestAddr(dest);
    datagram->setControlInfo(controlInfo);

    send(datagram, "sendOut");
}

