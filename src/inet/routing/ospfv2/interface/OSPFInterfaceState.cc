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

#include <map>

#include "inet/routing/ospfv2/interface/OSPFInterfaceState.h"

#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/interface/OSPFInterface.h"
#include "inet/routing/ospfv2/interface/OSPFInterfaceStateBackup.h"
#include "inet/routing/ospfv2/interface/OSPFInterfaceStateDesignatedRouter.h"
#include "inet/routing/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

void InterfaceState::changeState(Interface *intf, InterfaceState *newState, InterfaceState *currentState)
{
    Interface::InterfaceStateType oldState = currentState->getState();
    Interface::InterfaceStateType nextState = newState->getState();
    Interface::OSPFInterfaceType intfType = intf->getType();
    bool shouldRebuildRoutingTable = false;

    intf->changeState(newState, currentState);

    if ((oldState == Interface::DOWN_STATE) ||
        (nextState == Interface::DOWN_STATE) ||
        (oldState == Interface::LOOPBACK_STATE) ||
        (nextState == Interface::LOOPBACK_STATE) ||
        (oldState == Interface::DESIGNATED_ROUTER_STATE) ||
        (nextState == Interface::DESIGNATED_ROUTER_STATE) ||
        ((intfType == Interface::POINTTOPOINT) &&
         ((oldState == Interface::POINTTOPOINT_STATE) ||
          (nextState == Interface::POINTTOPOINT_STATE))) ||
        (((intfType == Interface::BROADCAST) ||
          (intfType == Interface::NBMA)) &&
         ((oldState == Interface::WAITING_STATE) ||
          (nextState == Interface::WAITING_STATE))))
    {
        RouterLSA *routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());

        if (routerLSA != NULL) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsAge(MAX_AGE);
                intf->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            }
            else {
                RouterLSA *newLSA = intf->getArea()->originateRouterLSA();

                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
                delete newLSA;

                intf->getArea()->floodLSA(routerLSA);
            }
        }
        else {    // (lsa == NULL) -> This must be the first time any interface is up...
            RouterLSA *newLSA = intf->getArea()->originateRouterLSA();

            shouldRebuildRoutingTable |= intf->getArea()->installRouterLSA(newLSA);

            routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());

            intf->getArea()->setSPFTreeRoot(routerLSA);
            intf->getArea()->floodLSA(newLSA);
            delete newLSA;
        }
    }

    if (nextState == Interface::DESIGNATED_ROUTER_STATE) {
        NetworkLSA *newLSA = intf->getArea()->originateNetworkLSA(intf);
        if (newLSA != NULL) {
            shouldRebuildRoutingTable |= intf->getArea()->installNetworkLSA(newLSA);

            intf->getArea()->floodLSA(newLSA);
            delete newLSA;
        }
        else {    // no neighbors on the network -> old NetworkLSA must be flushed
            NetworkLSA *oldLSA = intf->getArea()->findNetworkLSA(intf->getAddressRange().address);

            if (oldLSA != NULL) {
                oldLSA->getHeader().setLsAge(MAX_AGE);
                intf->getArea()->floodLSA(oldLSA);
                oldLSA->incrementInstallTime();
            }
        }
    }

    if (oldState == Interface::DESIGNATED_ROUTER_STATE) {
        NetworkLSA *networkLSA = intf->getArea()->findNetworkLSA(intf->getAddressRange().address);

        if (networkLSA != NULL) {
            networkLSA->getHeader().setLsAge(MAX_AGE);
            intf->getArea()->floodLSA(networkLSA);
            networkLSA->incrementInstallTime();
        }
    }

    if (shouldRebuildRoutingTable) {
        intf->getArea()->getRouter()->rebuildRoutingTable();
    }
}

void InterfaceState::calculateDesignatedRouter(Interface *intf)
{
    RouterID routerID = intf->parentArea->getRouter()->getRouterID();
    DesignatedRouterID currentDesignatedRouter = intf->designatedRouter;
    DesignatedRouterID currentBackupRouter = intf->backupDesignatedRouter;

    unsigned int neighborCount = intf->neighboringRouters.size();
    unsigned char repeatCount = 0;
    unsigned int i;

    DesignatedRouterID declaredBackup;
    unsigned char declaredBackupPriority;
    RouterID declaredBackupID;
    bool backupDeclared;

    DesignatedRouterID declaredDesignatedRouter;
    unsigned char declaredDesignatedRouterPriority;
    RouterID declaredDesignatedRouterID;
    bool designatedRouterDeclared;

    do {
        // calculating backup designated router
        declaredBackup = NULL_DESIGNATEDROUTERID;
        declaredBackupPriority = 0;
        declaredBackupID = NULL_ROUTERID;
        backupDeclared = false;

        DesignatedRouterID highestRouter = NULL_DESIGNATEDROUTERID;
        unsigned char highestPriority = 0;
        RouterID highestID = NULL_ROUTERID;

        for (i = 0; i < neighborCount; i++) {
            Neighbor *neighbor = intf->neighboringRouters[i];
            unsigned char neighborPriority = neighbor->getPriority();

            if (neighbor->getState() < Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            RouterID neighborID = neighbor->getNeighborID();
            DesignatedRouterID neighborsDesignatedRouter = neighbor->getDesignatedRouter();
            DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->getBackupDesignatedRouter();

            if (neighborsDesignatedRouter.routerID != neighborID) {
                if (neighborsBackupDesignatedRouter.routerID == neighborID) {
                    if ((neighborPriority > declaredBackupPriority) ||
                        ((neighborPriority == declaredBackupPriority) &&
                         (neighborID > declaredBackupID)))
                    {
                        declaredBackup = neighborsBackupDesignatedRouter;
                        declaredBackupPriority = neighborPriority;
                        declaredBackupID = neighborID;
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((neighborPriority > highestPriority) ||
                        ((neighborPriority == highestPriority) &&
                         (neighborID > highestID)))
                    {
                        highestRouter.routerID = neighborID;
                        highestRouter.ipInterfaceAddress = neighbor->getAddress();
                        highestPriority = neighborPriority;
                        highestID = neighborID;
                    }
                }
            }
        }
        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter.routerID != routerID) {
                if (currentBackupRouter.routerID == routerID) {
                    if ((intf->routerPriority > declaredBackupPriority) ||
                        ((intf->routerPriority == declaredBackupPriority) &&
                         (routerID > declaredBackupID)))
                    {
                        declaredBackup.routerID = routerID;
                        declaredBackup.ipInterfaceAddress = intf->interfaceAddressRange.address;
                        declaredBackupPriority = intf->routerPriority;
                        declaredBackupID = routerID;
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((intf->routerPriority > highestPriority) ||
                        ((intf->routerPriority == highestPriority) &&
                         (routerID > highestID)))
                    {
                        declaredBackup.routerID = routerID;
                        declaredBackup.ipInterfaceAddress = intf->interfaceAddressRange.address;
                        declaredBackupPriority = intf->routerPriority;
                        declaredBackupID = routerID;
                        backupDeclared = true;
                    }
                    else {
                        declaredBackup = highestRouter;
                        declaredBackupPriority = highestPriority;
                        declaredBackupID = highestID;
                        backupDeclared = true;
                    }
                }
            }
        }

        // calculating backup designated router
        declaredDesignatedRouter = NULL_DESIGNATEDROUTERID;
        declaredDesignatedRouterPriority = 0;
        declaredDesignatedRouterID = NULL_ROUTERID;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            Neighbor *neighbor = intf->neighboringRouters[i];
            unsigned char neighborPriority = neighbor->getPriority();

            if (neighbor->getState() < Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            RouterID neighborID = neighbor->getNeighborID();
            DesignatedRouterID neighborsDesignatedRouter = neighbor->getDesignatedRouter();
            DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->getBackupDesignatedRouter();

            if (neighborsDesignatedRouter.routerID == neighborID) {
                if ((neighborPriority > declaredDesignatedRouterPriority) ||
                    ((neighborPriority == declaredDesignatedRouterPriority) &&
                     (neighborID > declaredDesignatedRouterID)))
                {
                    declaredDesignatedRouter = neighborsDesignatedRouter;
                    declaredDesignatedRouterPriority = neighborPriority;
                    declaredDesignatedRouterID = neighborID;
                    designatedRouterDeclared = true;
                }
            }
        }
        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter.routerID == routerID) {
                if ((intf->routerPriority > declaredDesignatedRouterPriority) ||
                    ((intf->routerPriority == declaredDesignatedRouterPriority) &&
                     (routerID > declaredDesignatedRouterID)))
                {
                    declaredDesignatedRouter.routerID = routerID;
                    declaredDesignatedRouter.ipInterfaceAddress = intf->interfaceAddressRange.address;
                    declaredDesignatedRouterPriority = intf->routerPriority;
                    declaredDesignatedRouterID = routerID;
                    designatedRouterDeclared = true;
                }
            }
        }
        if (!designatedRouterDeclared) {
            declaredDesignatedRouter = declaredBackup;
            declaredDesignatedRouterPriority = declaredBackupPriority;
            declaredDesignatedRouterID = declaredBackupID;
            designatedRouterDeclared = true;
        }

        // if the router is any kind of DR or is no longer one of them, then repeat
        if (
            (
                (declaredDesignatedRouter.routerID != NULL_ROUTERID) &&
                ((
                     (currentDesignatedRouter.routerID == routerID) &&
                     (declaredDesignatedRouter.routerID != routerID)
                     ) ||
                 (
                     (currentDesignatedRouter.routerID != routerID) &&
                     (declaredDesignatedRouter.routerID == routerID)
                 ))
            ) ||
            (
                (declaredBackup.routerID != NULL_ROUTERID) &&
                ((
                     (currentBackupRouter.routerID == routerID) &&
                     (declaredBackup.routerID != routerID)
                     ) ||
                 (
                     (currentBackupRouter.routerID != routerID) &&
                     (declaredBackup.routerID == routerID)
                 ))
            )
            )
        {
            currentDesignatedRouter = declaredDesignatedRouter;
            currentBackupRouter = declaredBackup;
            repeatCount++;
        }
        else {
            repeatCount += 2;
        }
    } while (repeatCount < 2);

    RouterID routersOldDesignatedRouterID = intf->designatedRouter.routerID;
    RouterID routersOldBackupID = intf->backupDesignatedRouter.routerID;

    intf->designatedRouter = declaredDesignatedRouter;
    intf->backupDesignatedRouter = declaredBackup;

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter = (routersOldDesignatedRouterID == routerID);
    bool wasOther = (intf->getState() == Interface::NOT_DESIGNATED_ROUTER_STATE);
    bool wasWaiting = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter = (declaredBackup.routerID == routerID);
    bool isDesignatedRouter = (declaredDesignatedRouter.routerID == routerID);
    bool isOther = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            changeState(intf, new InterfaceStateDesignatedRouter, this);
        }
        if (isOther) {
            changeState(intf, new InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            changeState(intf, new InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasOther) {
        if (isDesignatedRouter) {
            changeState(intf, new InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            changeState(intf, new InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new InterfaceStateNotDesignatedRouter, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == Interface::NBMA) &&
            ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
             (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->neighboringRouters[i]->getPriority() == 0) {
                intf->neighboringRouters[i]->processEvent(Neighbor::START);
            }
        }
        if ((declaredDesignatedRouter.routerID != routersOldDesignatedRouterID) ||
            (declaredBackup.routerID != routersOldBackupID))
        {
            if (intf->neighboringRouters[i]->getState() >= Neighbor::TWOWAY_STATE) {
                intf->neighboringRouters[i]->processEvent(Neighbor::IS_ADJACENCY_OK);
            }
        }
    }
}

} // namespace ospf

} // namespace inet

