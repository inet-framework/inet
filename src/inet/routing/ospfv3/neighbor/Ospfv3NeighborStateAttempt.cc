/*
 * HELLO_RECEIVED - restart inactivity timer
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 */
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateAttempt.h"

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateInit.h"

namespace inet {
namespace ospfv3 {

void Ospfv3NeighborStateAttempt::processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event)
{
    if ((event == Ospfv3Neighbor::KILL_NEIGHBOR) || (event == Ospfv3Neighbor::LINK_DOWN)) {
        neighbor->reset();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new Ospfv3NeighborStateDown, this);
    }
    if (event == Ospfv3Neighbor::INACTIVITY_TIMER) {
        neighbor->reset();
        if (neighbor->getInterface()->getType() == Ospfv3Interface::NBMA_TYPE)
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());

        changeState(neighbor, new Ospfv3NeighborStateDown, this);
    }
    if (event == Ospfv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getNeighborDeadInterval());
        changeState(neighbor, new Ospfv3NeighborStateInit, this);
    }
}

} // namespace ospfv3
} // namespace inet

