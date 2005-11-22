#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include "IPAddress.h"

class UDPSocket
{
	public:
		UDPSocket(int userId) {gateToUdp = NULL; this->userId = userId; }
	
		void bind(IPAddress addr, int port);
		void unbind();
		
		void setOutputIf(IPAddress outputIf) {this->outputIf = outputIf;}
		
		void sendToUDP(cMessage *msg);
		void send(cMessage *msg, IPAddress destAddr, int destPort);

		void setOutputGate(cGate *toUdp)  {gateToUdp = toUdp;}
		
	
	private:
		IPAddress localAddr;
		int localPort;
		
		IPAddress outputIf;
		
		cGate *gateToUdp;
		int userId;
		
};

#endif
