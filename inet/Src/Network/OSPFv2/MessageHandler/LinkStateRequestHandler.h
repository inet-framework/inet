#ifndef __LINKSTATEREQUESTHANDLER_HPP__
#define __LINKSTATEREQUESTHANDLER_HPP__

#include "IMessageHandler.h"

namespace OSPF {

class LinkStateRequestHandler : public IMessageHandler {
public:
    LinkStateRequestHandler (Router* containingRouter);

    void    ProcessPacket (OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace OSPF

#endif // __LINKSTATEREQUESTHANDLER_HPP__

