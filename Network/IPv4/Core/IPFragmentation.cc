// $Header$
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

/* -------------------------------------------------
	file: IPFragmentation.cc
	Purpose: Implementation for IPFragmentation
	------
	Responsibilities:
	receive valid IP datagram from Routing or Multicast
	Fragment datagram if size > MTU [output port]
	send fragments to IPOutput[output port]

	comment: par("numOfPorts") not used at all

	author: Jochen Reber
	------------------------------------------------- */

#include "IPFragmentation.h"

Define_Module( IPFragmentation );

/* ICMP type 2, code 4: fragmentation needed, but
		don't-fragment bit set */
const int ICMP_FRAGMENTATION_ERROR_CODE = 4;

/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */

void IPFragmentation::initialize()
{
	RoutingTableAccess::initialize();

	numOfPorts = par("numOfPorts");
	delay = par("procdelay");
}

void IPFragmentation::activity()
{
    IPDatagram *datagram;
	int mtu, headerLength, payload;
	int noOfFragments;
	int i;

    while(true)
    {
        datagram = (IPDatagram *)receive();

//		mtu = rt->getInterfaceByIndex(datagram->outputPort()).mtu;
		mtu = rt->getInterfaceByIndex(datagram->outputPort())->mtu;

		/*
		ev << "totalLength / MTU: " << datagram->totalLength() << " / "
		<< mtu << "\n";
		*/

		// check if datagram does not require fragmentation
		if (datagram->totalLength() <= mtu)
		{
			datagram->addPar("finalFragment").setBoolValue(true);
			wait(delay);
			sendDatagramToOutput(datagram);
			continue;
		}

		headerLength = datagram->headerLength();
		payload = datagram->totalLength() - headerLength;

		noOfFragments=
			int(ceil((float(payload)/mtu) /
			(1-float(headerLength)/mtu) ) );
		/*
		ev << "No of Fragments: " << noOfFragments << "\n";
		*/

		/* if "don't Fragment"-bit is set, throw
			datagram away and send ICMP error message */
		if (datagram->dontFragment() && noOfFragments > 1)
		{
			sendErrorMessage (datagram,
				ICMP_DESTINATION_UNREACHABLE,
				ICMP_FRAGMENTATION_ERROR_CODE);
			continue;
		}

		for (i=0; i<noOfFragments; i++)
		{

			IPDatagram *fragment = (IPDatagram *) datagram->dup();
			// fragment = new IPDatagram;
			// fragment->operator=(*datagram);

			/* total_length equal to mtu, except for
				last fragment */

			/* "more Fragments"-bit is unchanged
				in the last fragment, otherwise
				true */
			if (i != noOfFragments-1)
			{
				// claimKernelAgain();
				fragment->setMoreFragments (true);
				fragment->setTotalLength(mtu);
			} else
			{
				// size of last fragment
				fragment->setTotalLength
					(datagram->totalLength() -
					 (noOfFragments-1) *
					 (mtu - datagram->headerLength()));
			}
			fragment->setFragmentOffset(
					i*(mtu - datagram->headerLength()) );


			wait(delay);
			sendDatagramToOutput(fragment);
		} // end for to noOfFragments
		delete( datagram );

	} // end while
}



/*  ----------------------------------------------------------
        Private Functions
    ----------------------------------------------------------  */

//  send error message to ICMP Module
void IPFragmentation::sendErrorMessage(IPDatagram *datagram,
        ICMPType type, ICMPCode code)
{
	// format in ICMP.h
    cMessage *icmpNotification = new cMessage();

    datagram->setName("datagram");
    icmpNotification->addPar("ICMPType") = (long)type;
    icmpNotification->addPar("ICMPCode") = code;
    icmpNotification->parList().add(datagram);

    send(icmpNotification, "errorOut");
}


void IPFragmentation::sendDatagramToOutput(IPDatagram *datagram)
{
	int outputPort;

	outputPort = datagram->outputPort();
	if (outputPort > numOfPorts -1)
	{
		ev << "Error in IPFragmentation: "
			<< "illegal output port: " << outputPort << "\n";
		delete( datagram );
		return;
	}

	send(datagram, "outputOut", outputPort);
}


