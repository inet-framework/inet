#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStatePointToPoint.h"

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/OSPFv3Timers.h"

namespace inet{
void OSPFv3InterfaceStatePointToPoint::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    /*
     * HELLO_TIMER - no changes in DR or BDR because there isn't one
     * INTERFACE_DOWN or LOOPBACK_IND - DOWN or LOOPBACK
     */
    if (event == OSPFv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateDown, this);
    }
    if (event == OSPFv3Interface::LOOP_IND_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateLoopback, this);
    }
    if (event == OSPFv3Interface::HELLO_TIMER_EVENT) {
        if (interface->getType() == OSPFv3Interface::VIRTUAL_TYPE) {
            if (interface->getNeighborCount() > 0) {
                Packet* hello = interface->prepareHello();
                Ipv6Address dest = interface->getNeighbor(0)->getNeighborIP();
                interface->getArea()->getInstance()->getProcess()->sendPacket(hello, dest, interface->getIntName().c_str(), VIRTUAL_LINK_TTL);
            }
        }
        else {
            Packet* hello = interface->prepareHello();
            interface->getArea()->getInstance()->getProcess()->sendPacket(hello, Ipv6Address::ALL_OSPF_ROUTERS_MCAST, interface->getIntName().c_str());
        }
        interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), interface->getHelloInterval());
    }
    if (event == OSPFv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT) {
        interface->sendDelayedAcknowledgements();
    }
}//processEvent
}//namespace inet
