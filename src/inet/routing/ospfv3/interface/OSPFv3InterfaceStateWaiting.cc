#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateWaiting.h"

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/OSPFv3Timers.h"

namespace inet{
void OSPFv3InterfaceStateWaiting::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    /*
     * HELLO_TIMER - for the time in wait timer it monitors hello packets
     * WAIT_TIMER - election - it goes either into DR, BDR or DROther
     * BACKUP_SEEN - end of waiting - either some router says it is the BDR or DR and the BDR is missing
     *              -either way the state is over
     * INTERFACE_DOWN or LOOPBACK_IND - transit to DOWN
     *
     */
    if ((event == OSPFv3Interface::BACKUP_SEEN_EVENT) ||
            (event == OSPFv3Interface::WAIT_TIMER_EVENT))
    {
        calculateDesignatedRouter(interface);
    }
    if (event == OSPFv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateDown, this);
    }
    if (event == OSPFv3Interface::LOOP_IND_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateLoopback, this);
    }
    if (event == OSPFv3Interface::HELLO_TIMER_EVENT) {
        if (interface->getType() == OSPFv3Interface::BROADCAST_TYPE) {
            Packet* hello = interface->prepareHello();
            interface->getArea()->getInstance()->getProcess()->sendPacket(hello, Ipv6Address::ALL_OSPF_ROUTERS_MCAST, interface->getIntName().c_str());
        }
        else {    // Interface::NBMA
            unsigned long neighborCount = interface->getNeighborCount();
            int hopLimit = (interface->getType() == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
            for (unsigned long i = 0; i < neighborCount; i++) {
                OSPFv3Neighbor *neighbor = interface->getNeighbor(i);
                if (neighbor->getNeighborPriority() > 0) {
                    Packet* hello = interface->prepareHello();
                    Ipv6Address dest = neighbor->getNeighborIP();
                    interface->getArea()->getInstance()->getProcess()->sendPacket(hello, dest, interface->getIntName().c_str(), hopLimit);
                }
            }
        }
        interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), interface->getHelloInterval());
    }
    if (event == OSPFv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT) {
        interface->sendDelayedAcknowledgements();
    }
}//processEvent


}//namespace inet
