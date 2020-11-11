/*
 * NEGOTIATION_DONE - new state EXCHANGE
 * ADJACENCY_OK? - of yes, stay, if not, it should be destroyed and the new state should be 2WAY
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 * 1WAY_RECEIVED - new state INIT
 * 2WAY_RECEIVED - no change
 */
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateExStart.h"

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState2Way.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateExchange.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateInit.h"

namespace inet {
namespace ospfv3 {

void Ospfv3NeighborStateExStart::processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event)
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
    if (event == Ospfv3Neighbor::ONEWAY_RECEIVED) {
        neighbor->reset();
        changeState(neighbor, new Ospfv3NeighborStateInit, this);
    }
    if (event == Ospfv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getInterface()->getDeadInterval());
    }
    if (event == Ospfv3Neighbor::IS_ADJACENCY_OK) {
        EV_DEBUG << "Ospfv3Neighbor::IS_ADJACENCY_OK caught in ExStartState for neighbor " << neighbor->getNeighborID() << "\n";
        if (!neighbor->needAdjacency()) {
            EV_DEBUG << "The adjacency is needed for neighbor " << neighbor->getNeighborID() << "\n";
            neighbor->reset();
            changeState(neighbor, new Ospfv3NeighborState2Way, this);
        }
    }
    if (event == Ospfv3Neighbor::DD_RETRANSMISSION_TIMER) {
        EV_DEBUG << "Ospfv3Neighbor::DD_RETRANSMISSION_TIMER caught in ExStartState\n";
        neighbor->retransmitDatabaseDescriptionPacket();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
    }
    if (event == Ospfv3Neighbor::NEGOTIATION_DONE) {
        EV_DEBUG << "Ospfv3Neighbor::NEGOTIATION_DONE caught in ExStartState\n";
        neighbor->createDatabaseSummary();
        EV_DEBUG << "SummaryListCount " << neighbor->getDatabaseSummaryListCount() << endl;
        neighbor->sendDDPacket();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getDDRetransmissionTimer());
        changeState(neighbor, new Ospfv3NeighborStateExchange, this);
    }
}

} // namespace ospfv3
} // namespace inet

