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

#include "IPFragmentation.h"
#include "IPControlInfo_m.h"
#include "InterfaceTableAccess.h"


Define_Module( IPFragmentation );

// ICMP type 2, code 4: fragmentation needed, but don't-fragment bit set
const int ICMP_FRAGMENTATION_ERROR_CODE = 4;


void IPFragmentation::initialize()
{
    ift = InterfaceTableAccess().get();
}

// FIXME performance model is not good here! probably we should wait #fragments times delay
void IPFragmentation::handleMessage(cMessage *msg)
{
    IPDatagram *datagram  = check_and_cast<IPDatagram *>(msg);
    IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision *>(msg->controlInfo());
    int outputPort = controlInfo->outputPort();
    IPAddress nextHopAddr = controlInfo->nextHopAddr();

    int mtu = ift->interfaceByPortNo(outputPort)->mtu();

    // check if datagram does not require fragmentation
    if (datagram->length()/8 <= mtu)
    {
        send(datagram, "outputOut");
        return;
    }

    int headerLength = datagram->headerLength();
    int payload = datagram->length()/8 - headerLength;

    int noOfFragments =
        int(ceil((float(payload)/mtu) /
        (1-float(headerLength)/mtu) ) ); // FIXME ???

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (datagram->dontFragment() && noOfFragments>1)
    {
        ev << "datagram larger than MTU and don't fragment bit set, sending ICMP_DESTINATION_UNREACHABLE\n";
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE,
                                                     ICMP_FRAGMENTATION_ERROR_CODE);
        return;
    }

    // create and send fragments
    ev << "Breaking datagram into " << noOfFragments << " fragments\n";
    // FIXME revise this!
    for (int i=0; i<noOfFragments; i++)
    {
        // FIXME is it ok that full encapsulated packet travels in every datagram fragment?
        // should better travel in the last fragment only. Cf. with reassembly code!
        IPDatagram *fragment = (IPDatagram *) datagram->dup();

        // total_length equal to mtu, except for last fragment;
        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if (i != noOfFragments-1)
        {
            fragment->setMoreFragments(true);
            fragment->setLength(8*mtu);
        }
        else
        {
            // size of last fragment
            int bytes = datagram->length()/8 - (noOfFragments-1) * (mtu - datagram->headerLength());
            fragment->setLength(8*bytes);
        }
        fragment->setFragmentOffset( i*(mtu - datagram->headerLength()) );

        sendDatagramToOutput(fragment, outputPort, nextHopAddr);
    }

    delete datagram;
}


void IPFragmentation::sendDatagramToOutput(IPDatagram *datagram, int outputPort, IPAddress nextHopAddr)
{
    // attach next hop address if needed
    IPRoutingDecision *routingDecision = new IPRoutingDecision();
    routingDecision->setNextHopAddr(nextHopAddr);
    routingDecision->setOutputPort(outputPort);
    datagram->setControlInfo(routingDecision);

    send(datagram, "outputOut");
}


