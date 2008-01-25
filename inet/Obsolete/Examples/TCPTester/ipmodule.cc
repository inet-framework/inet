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

//
// V. Boehm, July 12 1999
//

#include "ip.h"
#include "tcp.h"
#include <omnetpp.h>

/**
 * Dummy implementation of IPv4
 */
class DummyIP : public cSimpleModule
{
  private:
    void fromTransProcessing(cMessage * segment);
    void fromDummyPPPProcessing(cMessage * datagram);
    IpHeader *newIpHeader(void);

  public:
      Module_Class_Members(DummyIP, cSimpleModule, 16384);
    virtual void activity();
};


Define_Module(DummyIP);


void DummyIP::activity()
{
    for (;;)
    {
        //arriving message
        cMessage *msg = receive();

        if (msg->arrivedOn("from_tcp"))
        {
            fromTransProcessing(msg);
        }
        else if (msg->arrivedOn("from_ppp"))
        {
            fromDummyPPPProcessing(msg);
        }
    }
}


void DummyIP::fromTransProcessing(cMessage * segment)
{
    // if packet arrives from transport layer, encapsulate
    // the packet and send it to the network layer

    ev << "IP received segment of " << segment->length() / 8 << " bytes from transport layer.\n";

    // create a new IP datagram
    cMessage *ip_datagram = new cMessage("IP_DATAGRAM", IP_DATAGRAM);

    // create a new IP header
    IpHeader *ip_header = newIpHeader();

    // fill in source and destination address
    ip_header->ip_src = segment->par("src_addr");
    ip_header->ip_dst = segment->par("dest_addr");

    // add IP header
    ip_datagram->addPar("ipheader") = (void *) ip_header;
    ip_datagram->par("ipheader").configPointer(NULL, NULL, sizeof(IpHeader));

    // set length of the IP header without options (in bits)
    ip_datagram->setLength(20 * 8);

    // encapsulate segment from transport layer
    ip_datagram->encapsulate(segment);

    // send datagram
    ev << "Sending IP datagram of " << ip_datagram->length() /
        8 << " bytes to destination IP address " << ip_header->ip_dst << ".\n";
    send(ip_datagram, "to_ppp");
};


void DummyIP::fromDummyPPPProcessing(cMessage * datagram)
{
    // if packet arrives from network layer, decapsulate the
    // IP datagram and send the segment to the transport layer

    ev << "IP received datagram of " << datagram->length() / 8 << " bytes from network layer.\n";

    //get the segment from the incoming IP datagram
    cMessage *segment = datagram->decapsulate();
    delete datagram;

    //send the segment to the transport layer
    ev << "Sending segment of " << segment->length() / 8 << " bytes to transport layer.\n";
    send(segment, "to_tcp");
}


//function to create a new IP header
IpHeader *DummyIP::newIpHeader()
{
    //create new Ip header
    IpHeader *ip_header = new IpHeader;

    //fill in header information
    // - 1 = information not yet available
    ip_header->ip_v = 4;
    ip_header->ip_hl = 5;       //no options included
    ip_header->ip_len = -1;
    ip_header->ip_id = -1;
    ip_header->df = IP_F_SET;   //no fragmentation implemented yet
    ip_header->mf = IP_F_NSET;  //no fragmentation implemented yet
    ip_header->ip_off = -1;
    ip_header->ip_ttl = -1;     //not implemented yet
    ip_header->ip_p = 6;        //only TCP included so far
    ip_header->ip_src = -1;
    ip_header->ip_dst = -1;

    return ip_header;
}
