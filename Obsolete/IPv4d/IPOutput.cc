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


#include "IPOutput.h"
#include "IPDatagram.h"
#include "ARPPacket_m.h"


Define_Module(IPOutput);


void IPOutput::handleMessage(cMessage *msg)
{
    if (dynamic_cast<ARPPacket *>(msg))
    {
        // dispatch ARP packets to ARP
        send(msg, "queueOut");
        return;
    }

    IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);

    // hop counter check
    if (datagram->timeToLive() <= 0)
    {
        // drop datagram, destruction responsibility in ICMP
        ev << "datagram TTL reached zero, sending ICMP_TIME_EXCEEDED\n";
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_TIME_EXCEEDED, 0);
        return;
    }
    send(msg, "queueOut");
}


