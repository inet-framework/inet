#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDROther.h"

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/OSPFv3Timers.h"

namespace inet{
void OSPFv3InterfaceStateDROther::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    /*
     * HELLO_TIMER - watch for changes in DR, BDR or priority!!
     * NEIGHBOR_CHANGE
     *  -bidirectional comm established
     *  -no longer bidirectional comm
     *  -one of the neighbors declared itself DR or BDR
     *  -one of the neighbors is no londer declaring itself DR or BDR
     *  -advertised priority of neighbot has changed
     * INTERFACE_DOWN or LOOPBACK_IND - DOWN or LOOPBACK
     */
    if (event == OSPFv3Interface::NEIGHBOR_CHANGE_EVENT) {
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
            EV_DEBUG << "Sending Hello to all in " << this->getInterfaceStateString() << "\n";
            Packet* hello = interface->prepareHello();
            interface->getArea()->getInstance()->getProcess()->sendPacket(hello, Ipv6Address::ALL_OSPF_ROUTERS_MCAST, interface->getIntName().c_str());
        }
        else {    // Interface::NBMA
            if (interface->getRouterPriority() > 0) {
                unsigned long neighborCount = interface->getNeighborCount();
                for (unsigned long i = 0; i < neighborCount; i++) {
                    OSPFv3Neighbor *neighbor = interface->getNeighbor(i);
                    if (neighbor->getNeighborPriority() > 0) {
                        Packet* hello = interface->prepareHello();
                        Ipv6Address dest = interface->getNeighbor(i)->getNeighborIP();
                        interface->getArea()->getInstance()->getProcess()->sendPacket(hello, dest, interface->getIntName().c_str());
                    }
                }
            }
            else {
                Packet* hello = interface->prepareHello();
                interface->getArea()->getInstance()->getProcess()->sendPacket(hello, interface->getDesignatedIP(), interface->getIntName().c_str());
                interface->getArea()->getInstance()->getProcess()->sendPacket(hello, interface->getBackupIP(), interface->getIntName().c_str());
            }
        }
        interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), interface->getHelloInterval());
    }
    if (event == OSPFv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT) {
        interface->sendDelayedAcknowledgements();
    }
//    if (event == OSPFv3Interface::NEIGHBOR_REVIVED_EVENT) {
//        changeState(interface, new OSPFv3InterfaceStateWaiting, this);
//        this->getArea()->getInstance()->getProcess()->setTimer(interface->getWaitTimer(), interface->getDeadInterval());
//    }
}//processEvent
}//namespace inet
