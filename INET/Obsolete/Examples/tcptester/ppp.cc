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
// V. Boehm, July 12 1999
//

#include "nw.h"
#include "ip.h"
#include "tcp.h"
#include <omnetpp.h>


/**
 * Implementation of a "dummy" L2 protocol
 */
class DummyPPP : public cSimpleModule
{
  private:
    void fromSwitch(cMessage * frame);
    void fromIp(cMessage * datagram, int nw_length);

  public:
      Module_Class_Members(DummyPPP, cSimpleModule, 16384);
    virtual void activity();
};


Define_Module(DummyPPP);


void DummyPPP::activity()
{
    //module parameter
    int nw_length = par("nw_length");

    for (;;)
    {
        //arriving datagram or frame/packet
        cMessage *msg = receive();

        if (msg->arrivedOn("from_ip"))
        {
            fromIp(msg, nw_length);
        }
        else if (msg->arrivedOn("from_compound"))
        {
            fromSwitch(msg);
        }
    }
}


void DummyPPP::fromIp(cMessage * datagram, int nw_length)
{
    ev << "Communication network layer received datagram of " << datagram->length() /
        8 << " bytes from IP layer.\n";

    //create a new network layer frame/packet
    cMessage *nw_frame = new cMessage("NW_FRAME", NW_FRAME);

    //set length
    nw_frame->setLength(nw_length * 8);

    //encapsulate datagram from IP layer
    nw_frame->encapsulate(datagram);

    //send frame/packet
    ev << "Sending network frame  of " << nw_frame->length() / 8 << " bytes to switch.\n";
    send(nw_frame, "to_compound");
}


void DummyPPP::fromSwitch(cMessage * frame)
{
    ev << "Network layer received frame of " << frame->length() / 8 << " bytes from switch.\n";

    //get the datagram from the incoming frame/packet
    cMessage *datagram = frame->decapsulate();
    delete frame;

    //send the datagram to the IP layer
    ev << "Sending datagram of " << datagram->length() / 8 << " bytes to IP layer.\n";
    send(datagram, "to_ip");
}

