#ifndef __MESSAGEHANDLER_HPP__
#define __MESSAGEHANDLER_HPP__

#include "IMessageHandler.h"
#include "HelloHandler.h"
#include "DatabaseDescriptionHandler.h"
#include "LinkStateRequestHandler.h"
#include "LinkStateUpdateHandler.h"
#include "LinkStateAcknowledgementHandler.h"
#include "OSPFTimer_m.h"
#include "IPControlInfo.h"
#include "OSPFInterface.h"
//#include "OSPFNeighbor.h"

namespace OSPF {

class MessageHandler : public IMessageHandler {
private:
    cSimpleModule*                  ospfModule;

    HelloHandler                    helloHandler;
    DatabaseDescriptionHandler      ddHandler;
    LinkStateRequestHandler         lsRequestHandler;
    LinkStateUpdateHandler          lsUpdateHandler;
    LinkStateAcknowledgementHandler lsAckHandler;

public:
    MessageHandler  (Router* containingRouter, cSimpleModule* containingModule);

    void    MessageReceived (cMessage* message);
    void    HandleTimer     (OSPFTimer* timer);

    void    ProcessPacket   (OSPFPacket* packet, Interface* unused1 = NULL, Neighbor* unused2 = NULL);

    void    SendPacket      (OSPFPacket* packet, IPv4Address destination, int outputIfIndex, short ttl = 1);
    void    ClearTimer      (OSPFTimer* timer);
    void    StartTimer      (OSPFTimer* timer, simtime_t delay);

    void    PrintEvent                          (const char* eventString, const Interface* onInterface = NULL, const Neighbor* forNeighbor = NULL) const;
    void    PrintHelloPacket                    (const OSPFHelloPacket* helloPacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintDatabaseDescriptionPacket      (const OSPFDatabaseDescriptionPacket* ddPacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintLinkStateRequestPacket         (const OSPFLinkStateRequestPacket* requestPacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintLinkStateUpdatePacket          (const OSPFLinkStateUpdatePacket* updatePacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintLinkStateAcknowledgementPacket (const OSPFLinkStateAcknowledgementPacket* ackPacket, IPv4Address destination, int outputIfIndex) const;

    // Authentication not implemented
    bool    AuthenticatePacket  (OSPFPacket* packet)    { return true; }
};

} // namespace OSPF

#endif // __MESSAGEHANDLER_HPP__

