
#include "inet/routing/ospfv3/Ospfv3Timers.h"
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStatePointToPoint.h"

namespace inet {
namespace ospfv3 {

void Ospfv3InterfaceStatePointToPoint::processEvent(Ospfv3Interface* interface, Ospfv3Interface::Ospfv3InterfaceEvent event)
{
    /*
     * HELLO_TIMER - no changes in DR or BDR because there isn't one
     * INTERFACE_DOWN or LOOPBACK_IND - DOWN or LOOPBACK
     */
    if (event == Ospfv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new Ospfv3InterfaceStateDown, this);
    }
    if (event == Ospfv3Interface::LOOP_IND_EVENT) {
        interface->reset();
        changeState(interface, new Ospfv3InterfaceStateLoopback, this);
    }
    if (event == Ospfv3Interface::HELLO_TIMER_EVENT) {
        if (interface->getType() == Ospfv3Interface::VIRTUAL_TYPE) {
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
    if (event == Ospfv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT) {
        interface->sendDelayedAcknowledgements();
    }
}//processEvent

} // namespace ospfv3
}//namespace inet

