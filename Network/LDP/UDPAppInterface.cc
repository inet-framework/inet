/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#include <omnetpp.h>
#include "TransportInterfacePacket.h"
#include "UDPPacket.h"
#include "IPInterfacePacket.h"
#include "IPAddress.h"

class UDPAppInterface:public cSimpleModule
{
    Module_Class_Members(UDPAppInterface, cSimpleModule, 16384);

    virtual void activity();
    virtual void initialize();

  private:
      bool debug;
    const char *local_addr;
};

Define_Module(UDPAppInterface);

void UDPAppInterface::initialize()
{
    local_addr = par("local_addr").stringValue();

}

void UDPAppInterface::activity()
{

// FIXME THIS WOULDN'T WORK BECAUSE UDP NOW USES UDPInterfacePacket not cPars!!!

    cMessage *msg = receive();

    // delete ldpSignal;

    // Send out a broadcast message

    // cMessage *msg = new cMessage();

    msg->setLength(1);
    msg->addPar("content") = 1;
    msg->addPar("request") = true;

    IPAddress *address = new IPAddress("224.0.0.0");

    msg->addPar("dest_addr") = (address->str().c_str());
    msg->addPar("src_port") = 100;
    msg->addPar("dest_port") = 100;
    msg->addPar("src_addr") = local_addr;


    ev << "UDP_APP_INTERFACE DEBUG: Sending upd broadcast\n";
    send(msg, "to_udp_processing");

    cMessage *forMe = receive();
    delete forMe;

    while (true)
    {
        cMessage *msg1 = receive();

        ev << "UDP_APP_INTERFACE DEBUG: " <<
            "Message from " << (msg1->par("src_addr").stringValue()) << "\n";

        send(msg1, "toAppl");
    }

}
