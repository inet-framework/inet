
#include <omnetpp.h>
#include "TransportInterfacePacket.h"
#include "UDPPacket.h"
#include "IPInterfacePacket.h"

class UDPModule: public cSimpleModule
{
  Module_Class_Members(UDPModule, cSimpleModule, 0);
  virtual void initialize();
  virtual void handleMessage(cMessage* msg);

private:
  bool debug;
};

Define_Module(UDPModule);

void UDPModule::initialize()
{
  debug        = par("debug");
}

void UDPModule::handleMessage(cMessage* msg)
{
  if (debug)
    ev << "Received msg " << msg->name() << " ";

  if (strcmp(msg->arrivalGate()->name(), "from_ip") == 0)
    {
      if (debug)
    ev << "from IP layer" << endl;

      IPInterfacePacket* ippack = (IPInterfacePacket*) msg;
      UDPPacket* udppack = (UDPPacket*) ippack->decapsulate();
      TransportInterfacePacket* tpack = new TransportInterfacePacket;
      tpack->setSourcePort(udppack->sourcePort());
      tpack->setSourceAddress(ippack->srcAddr().getString());
      tpack->setDestinationPort(udppack->destinationPort());
      tpack->setDestinationAddress(ippack->destAddr().getString());
      tpack->encapsulate(udppack->decapsulate());

      send(tpack, "to_socket");
    }
  else // received from application layer
    {
      if (debug)
    ev << "from Socket layer" << endl;

      TransportInterfacePacket* tpack = (TransportInterfacePacket*) msg;
      IPInterfacePacket* ippack = new IPInterfacePacket();
      UDPPacket * udppack = new UDPPacket();

      udppack->setSourcePort(tpack->sourcePort());
      udppack->setDestinationPort(tpack->destinationPort());
      if (tpack->encapsulatedMsg())
        udppack->encapsulate(tpack->decapsulate());
      ippack->setDestAddr(tpack->destinationAddress());
      ippack->setSrcAddr(tpack->sourceAddress());
      ippack->encapsulate(udppack);

      ippack->setProtocol(IP_PROT_UDP);

      send(ippack, "to_ip");
    }

  delete msg;
}
