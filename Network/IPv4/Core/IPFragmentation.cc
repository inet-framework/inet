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


#include "IPFragmentation.h"

Define_Module( IPFragmentation );

// ICMP type 2, code 4: fragmentation needed, but don't-fragment bit set
const int ICMP_FRAGMENTATION_ERROR_CODE = 4;


void IPFragmentation::initialize()
{
    QueueBase::initialize();
    numOfPorts = par("numOfPorts");  // FIXME is this the same as gateSize("outputOut")?
}

// FIXME performance model is not good here!!! we should wait #fragments times delay!!!
void IPFragmentation::endService(cMessage *msg)
{
    IPDatagram *datagram  = check_and_cast<IPDatagram *>(msg);

    RoutingTable *rt = routingTableAccess.get();
    int mtu = rt->getInterfaceByIndex(datagram->outputPort())->mtu;

    // check if datagram does not require fragmentation
    if (datagram->length()/8 <= mtu)
    {
        datagram->addPar("finalFragment").setBoolValue(true);
        sendDatagramToOutput(datagram);
        return;
    }

    int headerLength = datagram->headerLength();
    int payload = datagram->length()/8 - headerLength;

    int noOfFragments =
        int(ceil((float(payload)/mtu) /
        (1-float(headerLength)/mtu) ) ); // FIXME ???

    // ev << "No of Fragments: " << noOfFragments << endl;

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (datagram->dontFragment() && noOfFragments>1)
    {
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE,
                                                     ICMP_FRAGMENTATION_ERROR_CODE);
        return;
    }

    // create and send fragments
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

        sendDatagramToOutput(fragment);
    }

    delete datagram;
}



//----------------------------------------------------------
// Private Functions
//----------------------------------------------------------

void IPFragmentation::sendDatagramToOutput(IPDatagram *datagram)
{
    int outputPort = datagram->outputPort();
    if (outputPort >= numOfPorts)
    {
        ev << "Error in IPFragmentation: illegal output port " << outputPort << endl;
        delete datagram;
        return;
    }

    send(datagram, "outputOut", outputPort);
}


