//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MESSAGEHANDLER_H
#define __INET_MESSAGEHANDLER_H

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

    void    MessageReceived(cMessage* message);
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
    void    PrintLinkStateAcknowledgementPacket(const OSPFLinkStateAcknowledgementPacket* ackPacket, IPv4Address destination, int outputIfIndex) const;

    // Authentication not implemented
    bool    AuthenticatePacket  (OSPFPacket* packet)    { return true; }
};

} // namespace OSPF

#endif // __INET_MESSAGEHANDLER_H

