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


#include "OSPFNeighborState.h"

#include "OSPFArea.h"
#include "OSPFInterface.h"
#include "OSPFRouter.h"


void OSPF::NeighborState::changeState(OSPF::Neighbor* neighbor, OSPF::NeighborState* newState, OSPF::NeighborState* currentState)
{

    OSPF::Neighbor::NeighborStateType oldState = currentState->getState();
    OSPF::Neighbor::NeighborStateType nextState = newState->getState();
    bool shouldRebuildRoutingTable = false;

    neighbor->changeState(newState, currentState);

    if ((oldState == OSPF::Neighbor::FULL_STATE) || (nextState == OSPF::Neighbor::FULL_STATE)) {
        OSPF::RouterID routerID = neighbor->getInterface()->getArea()->getRouter()->getRouterID();
        OSPF::RouterLSA* routerLSA = neighbor->getInterface()->getArea()->findRouterLSA(routerID);

        if (routerLSA != NULL) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsAge(MAX_AGE);
                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            } else {
                OSPF::RouterLSA* newLSA = neighbor->getInterface()->getArea()->originateRouterLSA();

                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
                delete newLSA;

                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
            }
        }

        if (neighbor->getInterface()->getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) {
            OSPF::NetworkLSA* networkLSA = neighbor->getInterface()->getArea()->findNetworkLSA(neighbor->getInterface()->getAddressRange().address);

            if (networkLSA != NULL) {
                long sequenceNumber = networkLSA->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    networkLSA->getHeader().setLsAge(MAX_AGE);
                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                    networkLSA->incrementInstallTime();
                } else {
                    OSPF::NetworkLSA* newLSA = neighbor->getInterface()->getArea()->originateNetworkLSA(neighbor->getInterface());

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= networkLSA->update(newLSA);
                        delete newLSA;
                    } else {    // no neighbors on the network -> old NetworkLSA must be flushed
                        networkLSA->getHeader().setLsAge(MAX_AGE);
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
