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

#include <omnetpp.h>
#include "TransportPacket.h"
#include "IPInterfacePacket.h"

#include "ip_address.h"

//#include <stdio.h>
//#include <string.h>

class Tcp2Ip: public cSimpleModule {
//  private:
//	void setStringAddress(char *saddr, int addr);

  public: 
	Module_Class_Members(Tcp2Ip, cSimpleModule, 0);
	virtual void initialize() { }
	virtual void handleMessage(cMessage *msg);
};


Define_Module(Tcp2Ip);


void Tcp2Ip::handleMessage(cMessage *msg)
{
//	IPAddrChar src_addr, dest_addr;

	// put the message into a TransportPacket instance
	TransportPacket *tpacket = new TransportPacket(*msg);

	// set kind of the transport packet 
	// this comes from tcpmodule and could be ACK_SEG, DATA etc.
	tpacket->setMsgKind(msg->kind());

	// set source and destination port
	tpacket->setSourcePort(
		msg->hasPar("src_port") ? (int)msg->par("src_port") : 255);
	tpacket->setDestinationPort(
		msg->hasPar("dest_port") ? (int)msg->par("dest_port") : 255);

//	//put the addresses into a string format
//	setStringAddress(src_addr, 
//					 msg->hasPar("src_addr") ? (int)msg->par("src_addr") : 255);
//	setStringAddress(dest_addr, 
//					 msg->hasPar("dest_addr") ? (int)msg->par("dest_addr") : 255);

	// encapsulate tpacket into an IPInterfacePacket
	IPInterfacePacket *ipintpacket = new IPInterfacePacket();
	ipintpacket->encapsulate(tpacket);
	ipintpacket->setDestAddr(IPAddress((int)msg->par("dest_addr")).getString());
	ipintpacket->setSrcAddr(IPAddress((int)msg->par("src_addr")).getString());
	ipintpacket->setProtocol(IP_PROT_TCP);

	// we don't set other values now
        delete msg;

	// send ipintpacket out to IP
	send(ipintpacket, "out");
}


/* Old way the coding was done
void Tcp2Ip::setStringAddress(char *saddr, int addr)
{
	char stringdom[]="10.0.0.";
	char buf[3];

	// no negative addr's allowed:
	// converted to 255
	if (addr < 0) 
		addr = 255;

	// put the integer addr into buffer
	sprintf(buf,"%i",addr);
	
	// copy stringhost to buf vector
	strcpy(saddr, stringdom);
	strcat(saddr, buf);
}
*/

