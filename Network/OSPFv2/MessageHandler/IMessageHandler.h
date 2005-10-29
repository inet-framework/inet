#ifndef __IMESSAGEHANDLER_HPP__
#define __IMESSAGEHANDLER_HPP__

#include "OSPFPacket_m.h"

namespace OSPF {

class Router;
class Interface;
class Neighbor;

class IMessageHandler
{
protected:
    Router* router;

public:
    IMessageHandler (Router* containingRouter)  { router = containingRouter; }
    virtual ~IMessageHandler () {}

    virtual void    ProcessPacket (OSPFPacket*, Interface* intf, Neighbor* neighbor) = 0;
};

} // namespace OSPF

#endif // __IMESSAGEHANDLER_HPP__
