#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateWaiting.h"

#include "inet/routing/ospfv3/Ospfv3Timers.h"
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateLoopback.h"

namespace inet {
namespace ospfv3 {

void Ospfv3InterfaceStateWaiting::processEvent(Ospfv3Interface *interface, Ospfv3Interface::Ospfv3InterfaceEvent event)
{
    /*
     * HELLO_TIMER - for the time in wait timer it monitors hello packets
     * WAIT_TIMER - election - it goes either into DR, BDR or DROther
     * BACKUP_SEEN - end of waiting - either some router says it is the BDR or DR and the BDR is missing
     *              -either way the state is over
     * INTERFACE_DOWN or LOOPBACK_IND - transit to DOWN
     *
     */
    if ((event == Ospfv3Interface::BACKUP_SEEN_EVENT) ||
        (event == Ospfv3Interface::WAIT_TIMER_EVENT))
    {
        calculateDesignatedRouter(interface);
    }
    if (event == Ospfv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new Ospfv3InterfaceStateDown, this);
    }
    if (event == Ospfv3Interface::LOOP_IND_EVENT) {
        interface->reset();
        changeState(interface, new Ospfv3InterfaceStateLoopback, this);
    }
    if (event == Ospfv3Interface::HELLO_TIMER_EVENT) {
        if (interface->getType() == Ospfv3Interface::BROADCAST_TYPE) {
            Packet *hello = interface->prepareHello();
            interface->getArea()->getInstance()->getProcess()->sendPacket(hello, Ipv6Address::ALL_OSPF_ROUTERS_MCAST, interface->getIntName().c_str());
        }
        else { // Interface::NBMA
            unsigned long neighborCount = interface->getNeighborCount();
            int hopLimit = (interface->getType() == Ospfv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
            for (unsigned long i = 0; i < neighborCount; i++) {
                Ospfv3Neighbor *neighbor = interface->getNeighbor(i);
                if (neighbor->getNeighborPriority() > 0) {
                    Packet *hello = interface->prepareHello();
                    Ipv6Address dest = neighbor->getNeighborIP();
                    interface->getArea()->getInstance()->getProcess()->sendPacket(hello, dest, interface->getIntName().c_str(), hopLimit);
                }
            }
        }
        interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), interface->getHelloInterval());
    }
    if (event == Ospfv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT) {
        interface->sendDelayedAcknowledgements();
    }
} // processEvent

} // namespace ospfv3
} // namespace inet

