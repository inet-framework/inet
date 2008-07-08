#ifndef __INET_LINKSTATEREQUESTHANDLER_H
#define __INET_LINKSTATEREQUESTHANDLER_H

#include "IMessageHandler.h"

namespace OSPF {

class LinkStateRequestHandler : public IMessageHandler {
public:
    LinkStateRequestHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace OSPF

#endif // __INET_LINKSTATEREQUESTHANDLER_H

