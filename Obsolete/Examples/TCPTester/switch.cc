//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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
// V. Boehm, June 20 1999
//


#include "nw.h"
#include "ip.h"
#include "tcp.h"
#include <omnetpp.h>


/**
 * For testing the TCP FSM
 */
class Switch:public cSimpleModule
{
    Module_Class_Members(Switch, cSimpleModule, 16384);
    virtual void activity();
};

Define_Module(Switch);

void Switch::activity()
{
    double pk_delay = 1.0 / (double) par("pk_rate");
    for (;;)
    {
        //receive frame, implicit queuing
        cMessage *rframe = receive();

        if (((simTime() <= 1.19) || (simTime() > 1.201))
            && ((simTime() <= 6.49) || (simTime() > 6.6)))
        {
            // get the datagram from the incoming frame/packet
            cMessage *datagram = rframe->decapsulate();

            // get length of frame after decapsulation
            int nw_length = rframe->length() / 8;

            delete rframe;

            // get IP header information about the IP destination address
            IpHeader *ip_header = (IpHeader *) (datagram->par("ipheader").pointerValue());
            int dest = ip_header->ip_dst;
            //delete ip_header;

            // create new frame to send to destination
            cMessage *sframe = new cMessage("NW_FRAME", NW_FRAME);

            // set length
            sframe->setLength(nw_length * 8);

            // encapsulate datagram
            sframe->encapsulate(datagram);

            // processing delay
            wait(pk_delay);

            ev << "Relaying frame to " << dest << " (destination IP address).\n";
            send(sframe, "out", dest);
        }
        else
        {
            ev << "Deleting frame\n";
            delete rframe;
        }
    };
};
