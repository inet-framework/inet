//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/routing/ospfv2/neighbor/OspfNeighborState.h"

#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/interface/OspfInterface.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {

namespace ospf {

void NeighborState::changeState(Neighbor *neighbor, NeighborState *newState, NeighborState *currentState)
{
    Neighbor::NeighborStateType oldState = currentState->getState();
    Neighbor::NeighborStateType nextState = newState->getState();
    bool shouldRebuildRoutingTable = false;

    neighbor->changeState(newState, currentState);

    if ((oldState == Neighbor::FULL_STATE) || (nextState == Neighbor::FULL_STATE)) {
        RouterId routerID = neighbor->getInterface()->getArea()->getRouter()->getRouterID();
        RouterLsa *routerLSA = neighbor->getInterface()->getArea()->findRouterLSA(routerID);

        if (routerLSA != nullptr) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            }
            else {
                RouterLsa *newLSA = neighbor->getInterface()->getArea()->originateRouterLSA();

                newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
                delete newLSA;

                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
            }
        }

        if (neighbor->getInterface()->getState() == Interface::DESIGNATED_ROUTER_STATE) {
            NetworkLsa *networkLSA = neighbor->getInterface()->getArea()->findNetworkLSA(neighbor->getInterface()->getAddressRange().address);

            if (networkLSA != nullptr) {
                long sequenceNumber = networkLSA->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    networkLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                    networkLSA->incrementInstallTime();
                }
                else {
                    NetworkLsa *newLSA = neighbor->getInterface()->getArea()->originateNetworkLSA(neighbor->getInterface());

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= networkLSA->update(newLSA);
                        delete newLSA;
                    }
                    else {    // no neighbors on the network -> old NetworkLsa must be flushed
                        networkLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
                        networkLSA->incrementInstallTime();
                    }

                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                }
            }
        }
    }

    if (shouldRebuildRoutingTable) {
        neighbor->getInterface()->getArea()->getRouter()->rebuildRoutingTable();
    }
}

} // namespace ospf

} // namespace inet

