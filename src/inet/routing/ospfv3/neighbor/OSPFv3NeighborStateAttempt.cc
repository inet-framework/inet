/*
 * HELLO_RECEIVED - restart inactivity timer
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 */
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateAttempt.h"

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateInit.h"


namespace inet{
void OSPFv3NeighborStateAttempt::processEvent(OSPFv3Neighbor *neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event)
{
    if ((event == OSPFv3Neighbor::KILL_NEIGHBOR) || (event == OSPFv3Neighbor::LINK_DOWN)) {
        neighbor->reset();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new OSPFv3NeighborStateDown, this);
    }
    if (event == OSPFv3Neighbor::INACTIVITY_TIMER) {
        neighbor->reset();
        if (neighbor->getInterface()->getType() == OSPFv3Interface::NBMA_TYPE)
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());

        changeState(neighbor, new OSPFv3NeighborStateDown, this);
    }
    if (event == OSPFv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getNeighborDeadInterval());
        changeState(neighbor, new OSPFv3NeighborStateInit, this);
    }
}
}//namespace inet
