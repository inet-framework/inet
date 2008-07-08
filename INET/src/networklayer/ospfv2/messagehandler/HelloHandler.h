#ifndef __INET_HELLOHANDLER_H
#define __INET_HELLOHANDLER_H

#include "IMessageHandler.h"

namespace OSPF {

class HelloHandler : public IMessageHandler {
public:
    HelloHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* unused = NULL);
};

} // namespace OSPF

#endif // __INET_HELLOHANDLER_H

