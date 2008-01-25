#ifndef __HELLOHANDLER_HPP__
#define __HELLOHANDLER_HPP__

#include "IMessageHandler.h"

namespace OSPF {

class HelloHandler : public IMessageHandler {
public:
    HelloHandler (Router* containingRouter);

    void    ProcessPacket (OSPFPacket* packet, Interface* intf, Neighbor* unused = NULL);
};

} // namespace OSPF

#endif // __HELLOHANDLER_HPP__

