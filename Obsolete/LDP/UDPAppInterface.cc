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
#include "UDPPacket.h"
#include "UDPControlInfo_m.h"
#include "IPControlInfo_m.h"
#include "IPAddress.h"
#include "IPAddressResolver.h"
#include "RoutingTableAccess.h"


class UDPAppInterface : public cSimpleModule
{
    Module_Class_Members(UDPAppInterface, cSimpleModule, 16384);

    virtual void activity();
    virtual void initialize();

  private:
    IPAddress local_addr;
};

Define_Module(UDPAppInterface);

void UDPAppInterface::initialize()
{
    local_addr = par("local_addr").stringValue();
    //local_addr = IPAddressResolver().getAddressFrom(RoutingTableAccess().get());
}

void UDPAppInterface::activity()
{
    // Wait for LDP signal (? --AV)
    cMessage *ldpSignal = receive();
    delete ldpSignal;

    // Send out a broadcast message
    cMessage *msg = new cMessage();
    msg->setLength(1);
    msg->addPar("content") = 1;
    msg->addPar("request") = true;

    UDPControlInfo *controlInfo = new UDPControlInfo();
    controlInfo->setSrcAddr(local_addr);
    controlInfo->setDestAddr(IPAddress("224.0.0.0"));
    controlInfo->setSrcPort(100);
    controlInfo->setDestPort(100);
    msg->setControlInfo(controlInfo);

    ev << "UDP_APP_INTERFACE DEBUG: Sending upd broadcast\n";
    send(msg, "to_udp_processing");

    // ??? --AV
    cMessage *forMe = receive();
    delete forMe;

    // pass up all further arriving packets
    while (true)
    {
        cMessage *msg1 = receive();
        IPAddress srcAddr = check_and_cast<UDPControlInfo *>(msg1->controlInfo())->getSrcAddr();
        ev << "UDP_APP_INTERFACE DEBUG: Message from " << srcAddr << "\n";
        send(msg1, "toAppl");
    }
}
