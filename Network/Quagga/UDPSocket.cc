
#include "UDPSocket.h"
#include "UDPPacket.h" // should be included in UDP.h
#include "UDP.h"


void UDPSocket::bind(IPAddress addr, int port)
{
	localAddr = addr;
	localPort = port;
	
	UDPControlInfo *udpControlInfo = new UDPControlInfo();
	udpControlInfo->setSrcPort(localPort);
	udpControlInfo->setSrcAddr(localAddr);
	udpControlInfo->setUserId(userId);

	cMessage *msg = new cMessage("udp-bind");
	msg->setKind(UDP_C_BIND);
	msg->setControlInfo(udpControlInfo);
	
	sendToUDP(msg);
}

void UDPSocket::unbind()
{
	UDPControlInfo *udpControlInfo = new UDPControlInfo();
	udpControlInfo->setSrcPort(localPort);
	udpControlInfo->setSrcAddr(localAddr);
	udpControlInfo->setUserId(userId);

	cMessage *msg = new cMessage("udp-unbind");
	msg->setKind(UDP_C_UNBIND);
	msg->setControlInfo(udpControlInfo);
	
	sendToUDP(msg);
}

void UDPSocket::send(cMessage *msg, IPAddress destAddr, int destPort)
{
	UDPControlInfo *udpControlInfo = new UDPControlInfo();
	udpControlInfo->setSrcPort(localPort);
	udpControlInfo->setSrcAddr(localAddr);
	udpControlInfo->setDestPort(destPort);
	udpControlInfo->setDestAddr(destAddr);
	udpControlInfo->setOutputIf(outputIf);
	
    msg->setKind(UDP_C_DATA);
	msg->setControlInfo(udpControlInfo);
    sendToUDP(msg);
}

void UDPSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        opp_error("UDPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToUdp->ownerModule())->send(msg, gateToUdp);
}

