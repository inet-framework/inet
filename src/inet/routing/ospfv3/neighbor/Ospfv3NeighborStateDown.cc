/*
 * HELLO_RECEIVED - restart Inactivity timer, transition to INIT
 * START - go to ATTEMPT, start Inactivity timer
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 */

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateAttempt.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateInit.h"

namespace inet {
namespace ospfv3 {

void Ospfv3NeighborStateDown::processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event)
{
    /*
      Send an Hello Packet to the neighbor (this neighbor
      is always associated with an NBMA network) and start
      the Inactivity Timer for the neighbor.  The timer's
      later firing would indicate that communication with
      the neighbor was not attained.
    */
    if (event == Ospfv3Neighbor::START) {
        int hopLimit = (neighbor->getInterface()->getType() == Ospfv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getPollTimer());
        Packet* hello = neighbor->getInterface()->prepareHello();
//        Ospfv3HelloPacket* hello = neighbor->getInterface()->prepareHello();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(hello, neighbor->getNeighborIP(), neighbor->getInterface()->getIntName().c_str(), hopLimit);
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getNeighborDeadInterval());
        this->changeState(neighbor, new Ospfv3NeighborStateAttempt, this);
    }

    /*
      Start the Inactivity Timer for the neighbor.  The
      timer's later firing would indicate that the
      neighbor is dead.
    */
    if (event == Ospfv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getPollTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getNeighborDeadInterval());
        EV_DEBUG << "HELLO_RECEIVED, number of neighbors: " << neighbor->getInterface()->getNeighborCount() << endl;
        changeState(neighbor, new Ospfv3NeighborStateInit, this);
    }
    if (event == Ospfv3Neighbor::POLL_TIMER) {
        int hopLimit = (neighbor->getInterface()->getType() == Ospfv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
        Packet* hello = neighbor->getInterface()->prepareHello();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(hello, neighbor->getNeighborIP(), neighbor->getInterface()->getIntName().c_str(), hopLimit);
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getPollTimer(), neighbor->getNeighborDeadInterval());
    }
}

} // namespace ospfv3
}//namespace inet

