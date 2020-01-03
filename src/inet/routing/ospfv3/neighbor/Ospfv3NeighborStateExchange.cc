/*
 * EXCHANGE_DONE - new state FULL if the neighbor LSR is empty.
 *               - new state LOADING if the LSR is not empty
 *
 * ADJACENCY_OK? - of yes, stay, if not, it should be destroyed and the new state should be 2WAY
 * SEQUENCE_NUMBER_MISMATCH - new state EXSTART
 * BAD_LS_REQ - new state EXSTART
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 * 1WAY_RECEIVED - new state INIT
 * 2WAY_RECEIVED - no change
 */

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState2Way.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateExStart.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateExchange.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateFull.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateInit.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateLoading.h"

namespace inet {
namespace ospfv3 {

void Ospfv3NeighborStateExchange::processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event)
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
        if (!neighbor->needAdjacency()) {
            neighbor->reset();
            changeState(neighbor, new Ospfv3NeighborState2Way, this);
        }
    }
    if ((event == Ospfv3Neighbor::SEQUENCE_NUMBER_MISMATCH) || (event == Ospfv3Neighbor::BAD_LINK_STATE_REQUEST)) {
        neighbor->reset();
        neighbor->incrementDDSequenceNumber();
        neighbor->sendDDPacket(true);
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
        changeState(neighbor, new Ospfv3NeighborStateExStart, this);
    }
    if (event == Ospfv3Neighbor::EXCHANGE_DONE) {
        EV_DEBUG << "Ospfv3Neighbor::EXCHANGE_DONE caught in ExchangeState\n";
        if (!neighbor->isLinkStateRequestListEmpty()) {
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getDeadInterval());
            neighbor->clearRequestRetransmissionTimer();
            changeState(neighbor, new Ospfv3NeighborStateFull, this);
        }
        else {
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
            changeState(neighbor, new Ospfv3NeighborStateLoading, this);
        }
    }
    if (event == Ospfv3Neighbor::UPDATE_RETRANSMISSION_TIMER) {
        EV_DEBUG << "Ospfv3Neighbor::UPDATE_RETRANSMISSION_TIMER caught in ExchangeState\n";
        neighbor->retransmitUpdatePacket();                             //  ZAKOMENTOVANE PRED MIGRACIOU .  PRECO ?
        neighbor->startUpdateRetransmissionTimer();
    }
    if (event == Ospfv3Neighbor::REQUEST_RETRANSMISSION_TIMER) {
        EV_DEBUG << "Ospfv3Neighbor::REQUEST_RETRANSMISSION_TIMER caught in ExchangeState\n";
        neighbor->sendLinkStateRequestPacket();                             //  ZAKOMENTOVANE PRED MIGRACIOU .  PRECO ?
        neighbor->startRequestRetransmissionTimer();
    }
}

} // namespace ospfv3
}//namespace inet

