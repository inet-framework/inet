//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState.h"

#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"

namespace inet {
namespace ospfv3 {

void Ospfv3NeighborState::changeState(Ospfv3Neighbor *neighbor, Ospfv3NeighborState *newState, Ospfv3NeighborState *currentState)
{
    Ospfv3Neighbor::Ospfv3NeighborStateType oldState = currentState->getState();
    Ospfv3Neighbor::Ospfv3NeighborStateType nextState = newState->getState();
    bool shouldRebuildRoutingTable = false;

    EV_DEBUG << "Changing neighbor state from " << currentState->getNeighborStateString() << " to " << newState->getNeighborStateString() << "\n";

    neighbor->changeState(newState, currentState);

    if ((oldState == Ospfv3Neighbor::FULL_STATE) || (nextState == Ospfv3Neighbor::FULL_STATE)) {
        Ipv4Address routerID = neighbor->getInterface()->getArea()->getInstance()->getProcess()->getRouterID();
        RouterLSA *routerLSA = neighbor->getInterface()->getArea()->findRouterLSA(routerID);

        if (routerLSA != nullptr) {
            long sequenceNumber = routerLSA->getHeader().getLsaSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            }
            else {
                RouterLSA *newLSA = neighbor->getInterface()->getArea()->originateRouterLSA();

                newLSA->getHeaderForUpdate().setLsaSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= neighbor->getInterface()->getArea()->updateRouterLSA(routerLSA, newLSA);
                if (shouldRebuildRoutingTable)
                    neighbor->getInterface()->getArea()->setSpfTreeRoot(routerLSA);
                delete newLSA;

                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
            }
        }

        if (neighbor->getInterface()->getState() == Ospfv3Interface::INTERFACE_STATE_DESIGNATED) {
            NetworkLSA *networkLSA = neighbor->getInterface()->getArea()->findNetworkLSAByLSID(
                    Ipv4Address(neighbor->getInterface()->getInterfaceId()));

            if (networkLSA != nullptr) {
                long sequenceNumber = networkLSA->getHeader().getLsaSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    networkLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                    networkLSA->incrementInstallTime();
                }
                else {
                    NetworkLSA *newLSA = neighbor->getInterface()->getArea()->originateNetworkLSA(neighbor->getInterface());

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsaSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= neighbor->getInterface()->getArea()->updateNetworkLSA(networkLSA, newLSA);
                        delete newLSA;
                    }
                    else { // no neighbors on the network -> old NetworkLSA must be flushed
                        networkLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                        networkLSA->incrementInstallTime();
                    }
                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                }
            }
        }
    }

    Ospfv3Area *thisArea = neighbor->getInterface()->getArea();
    if (nextState == Ospfv3Neighbor::DOWN_STATE) { // this neigbor was shuted down
        // invalidate all LSA type 3, which I know from this neighbor
        // set MAX_AGE
        if (thisArea->getInstance()->getAreaCount() > 1) { // this is ABR
            for (int ar = 0; ar < thisArea->getInstance()->getAreaCount(); ar++) {
                Ospfv3Area *area = thisArea->getInstance()->getArea(ar);
                if (area->getAreaID() == thisArea->getAreaID()) // skip my Area
                    continue;

                // in all other Areas invalidate all Inter-Area-Prefix LSAs with same prefix IP as all
                // known Intra-Area-Prefix LSAs from this neighbor
                for (int i = 0; i < thisArea->getIntraAreaPrefixLSACount(); i++) {
                    IntraAreaPrefixLSA *iapLSA = thisArea->getIntraAreaPrefixLSA(i);
                    if (neighbor->getNeighborID() == iapLSA->getHeader().getAdvertisingRouter()) {
                        for (size_t k = 0; k < iapLSA->getPrefixesArraySize(); k++) {
                            // go through all Inter-Area-Prefix LSA of other Area
                            for (int j = 0; j < area->getInterAreaPrefixLSACount(); j++) {
                                InterAreaPrefixLSA *interLSA = area->getInterAreaPrefixLSA(j);
                                if ((interLSA->getHeader().getAdvertisingRouter() == thisArea->getInstance()->getProcess()->getRouterID()) &&
                                    (interLSA->getPrefix().addressPrefix == iapLSA->getPrefixes(k).addressPrefix) &&
                                    (interLSA->getPrefix().prefixLen == iapLSA->getPrefixes(k).prefixLen))
                                {
                                    interLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                                    area->floodLSA(interLSA);
                                }
                            }
                        }
                        // invalidate INTRA LSA too
                        iapLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
//                      neighbor->getInterface()->getArea()->floodLSA(iapLSA);
                    }
                }
            }
        }
        else { // invalidate only INTRA LSA
            for (int i = 0; i < thisArea->getIntraAreaPrefixLSACount(); i++) {
                IntraAreaPrefixLSA *iapLSA = thisArea->getIntraAreaPrefixLSA(i);
                if (neighbor->getNeighborID() == iapLSA->getHeader().getAdvertisingRouter()) {
                    // invalidate INTRA LSA too
                    iapLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                    thisArea->floodLSA(iapLSA);
                }
            }
        }
    }

    if (shouldRebuildRoutingTable) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->rebuildRoutingTable();
    }
}

} // namespace ospfv3
} // namespace inet

