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

/*
  file: RoutingCore.cc
  Purpose: Implementation of static Routing, based on Routing file

  Responsibilities: 
  Receive correct IP datagram
  send datagram with Multicast addr. to Multicast module
  if source routing option is on, use next source addr. as dest. addr.
  map IP address on output port, use static routing table
  if destination address is not in routing table, 
  throw datagram away and notify ICMP
  process record route and timestamp options, if applicable
  send to local Deliver if dest. addr. = 127.0.0.1 
  or dest. addr. = NetworkCardAddr.[]
  otherwise, send to Fragmentation module

  comments: 
  IP options should be handled here, but are currently not
  implemented (20.5.00)

  author: Jochen Reber
  date: 20.5.00, 30.5.00
*/

#include <stdlib.h>
#include <omnetpp.h>

#include "hook_types.h"
#include "RoutingCore.h"
#include "RoutingTable.h"

const int MAX_FILENAME_LENGTH = 30;

Define_Module( RoutingCore );

/* 	----------------------------------------------------------
        Public Functions
	----------------------------------------------------------	*/

void RoutingCore::initialize()
{
  RoutingTableAccess::initialize();
  IPForward = par("IPForward").boolValue();
  delay = par("procdelay");
  hasHook = (findGate("netfilterOut") != -1);
}

void RoutingCore::activity()
{
  cMessage *dfmsg;
  cMessage *message;
  IPAddrChar destAddress;
  IPDatagram *datagram;
  int outputPort;

  while(true)
    {
      message = receive();

      /* 	IP datagrams are treated differently
                than ICMP messages 
      */
      if (message->kind() == MK_PACKET) // IP datagram
        {
          datagram = (IPDatagram *)message; 

          // pass Datagram through netfilter
          if (hasHook) 
            {
              send(datagram, "netfilterOut");
              dfmsg = receiveNewOn("netfilterIn");
              if (dfmsg->kind() == DISCARD_PACKET)
            	{
                  delete dfmsg;
                  releaseKernel();
                  continue;
            	}
              datagram = (IPDatagram *)dfmsg;
            }

          // wait for duration of processing delay
          wait(delay);

          // FIXME add option handling code here!
			
          strcpy(destAddress, datagram->destAddress());
// BCH Andras -- code from UTS MPLS model
          ev << "Packet destination address is: " << destAddress << "\n";
// ECH

          // 	multicast check
          if (rt->isMulticastAddr(destAddress))
            {
              send(datagram, "multicastOut");
              continue;
            } 
	
          // check for local delivery
          if (rt->localDeliver(destAddress))
            {
              send(datagram, "localOut");
              continue;
            }
	
          /* delete datagram and continue,
             if datagram arrived from input gate 
             and IP_FORWARD is off */
          if (datagram->inputPort() != -1 && IPForward == false)
            {
              delete(datagram);
              releaseKernel();
              continue;
            }

          /* 	error handling: destination address does not exist in
                routing table:
                notify ICMP, throw packet away and continue
          */
          // use of outputPort to check existance saves computation time
          outputPort = rt->outputPortNo(destAddress);

          /* debug message for routing check
             ev << "*" << fullPath() 
             << " routing: " << destAddress 
             << " -> " << outputPort
             << "\n";
          */

          if (outputPort == -1)
            {
              sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
              continue;
            }
	
          // default: send datagram to fragmentation
          datagram->setOutputPort(outputPort);
          send(datagram, "fragmentationOut");
	
        } else // ICMP message
          {
            /* ICMP messages currently not handled
               possibly no need */

            releaseKernel();
            delete(message);
	
          } // end else 
    } // end while

}

/* 	----------------------------------------------------------
        Private Functions
	----------------------------------------------------------	*/

// send error message to ICMP Module
void RoutingCore::sendErrorMessage(IPDatagram *datagram, 
                                   ICMPType type, ICMPCode code)
{
  // format of message: see ICMP.h

  cMessage *icmpNotification = new cMessage();

  datagram->setName("datagram");
  icmpNotification->addPar("ICMPType") = (long)type;
  icmpNotification->addPar("ICMPCode") = code;
  icmpNotification->parList().add(datagram);

  send(icmpNotification, "errorOut");
}

