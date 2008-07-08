#ifndef __INET_LINKSTATEACKNOWLEDGEMENTHANDLER_H
#define __INET_LINKSTATEACKNOWLEDGEMENTHANDLER_H

#include "IMessageHandler.h"

namespace OSPF {

class LinkStateAcknowledgementHandler : public IMessageHandler {
public:
    LinkStateAcknowledgementHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace OSPF

#endif // __INET_LINKSTATEACKNOWLEDGEMENTHANDLER_H

