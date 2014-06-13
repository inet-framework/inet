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

#include "OSPFInterfaceState.h"

#include "OSPFArea.h"
#include "OSPFInterface.h"
#include "OSPFInterfaceStateBackup.h"
#include "OSPFInterfaceStateDesignatedRouter.h"
#include "OSPFInterfaceStateNotDesignatedRouter.h"
#include "OSPFRouter.h"


void OSPF::InterfaceState::changeState(OSPF::Interface* intf, OSPF::InterfaceState* newState, OSPF::InterfaceState* currentState)
{
    OSPF::Interface::InterfaceStateType oldState = currentState->getState();
    OSPF::Interface::InterfaceStateType nextState = newState->getState();
    OSPF::Interface::OSPFInterfaceType intfType = intf->getType();
    bool shouldRebuildRoutingTable = false;

    intf->changeState(newState, currentState);

    if ((oldState == OSPF::Interface::DOWN_STATE) ||
        (nextState == OSPF::Interface::DOWN_STATE) ||
        (oldState == OSPF::Interface::LOOPBACK_STATE) ||
        (nextState == OSPF::Interface::LOOPBACK_STATE) ||
        (oldState == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
        (nextState == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
        ((intfType == OSPF::Interface::POINTTOPOINT) &&
         ((oldState == OSPF::Interface::POINTTOPOINT_STATE) ||
          (nextState == OSPF::Interface::POINTTOPOINT_STATE))) ||
        (((intfType == OSPF::Interface::BROADCAST) ||
          (intfType == OSPF::Interface::NBMA)) &&
         ((oldState == OSPF::Interface::WAITING_STATE) ||
          (nextState == OSPF::Interface::WAITING_STATE))))
    {
        OSPF::RouterLSA* routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());

        if (routerLSA != NULL) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsAge(MAX_AGE);
                intf->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            } else {
                OSPF::RouterLSA* newLSA = intf->getArea()->originateRouterLSA();

                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
                delete newLSA;

                intf->getArea()->floodLSA(routerLSA);
            }
        } else {  // (lsa == NULL) -> This must be the first time any interface is up...
            OSPF::RouterLSA* newLSA = intf->getArea()->originateRouterLSA();

            shouldRebuildRoutingTable |= intf->getArea()->installRouterLSA(newLSA);

            routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());

            intf->getArea()->setSPFTreeRoot(routerLSA);
            intf->getArea()->floodLSA(newLSA);
            delete newLSA;
        }
    }

    if (nextState == OSPF::Interface::DESIGNATED_ROUTER_STATE) {
        OSPF::NetworkLSA* newLSA = intf->getArea()->originateNetworkLSA(intf);
        if (newLSA != NULL) {
            shouldRebuildRoutingTable |= intf->getArea()->installNetworkLSA(newLSA);

            intf->getArea()->floodLSA(newLSA);
            delete newLSA;
        } else {    // no neighbors on the network -> old NetworkLSA must be flushed
            OSPF::NetworkLSA* oldLSA = intf->getArea()->findNetworkLSA(intf->getAddressRange().address);

            if (oldLSA != NULL) {
                oldLSA->getHeader().setLsAge(MAX_AGE);
                intf->getArea()->floodLSA(oldLSA);
                oldLSA->incrementInstallTime();
            }
        }
    }

    if (oldState == OSPF::Interface::DESIGNATED_ROUTER_STATE) {
        OSPF::NetworkLSA* networkLSA = intf->getArea()->findNetworkLSA(intf->getAddressRange().address);

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

void OSPF::InterfaceState::calculateDesignatedRouter(OSPF::Interface* intf)
{
    OSPF::RouterID routerID = intf->parentArea->getRouter()->getRouterID();
    OSPF::DesignatedRouterID currentDesignatedRouter = intf->designatedRouter;
    OSPF::DesignatedRouterID currentBackupRouter = intf->backupDesignatedRouter;

    unsigned int neighborCount = intf->neighboringRouters.size();
    unsigned char repeatCount = 0;
    unsigned int i;

    OSPF::DesignatedRouterID declaredBackup;
    unsigned char declaredBackupPriority;
    OSPF::RouterID declaredBackupID;
    bool backupDeclared;

    OSPF::DesignatedRouterID declaredDesignatedRouter;
    unsigned char declaredDesignatedRouterPriority;
    OSPF::RouterID declaredDesignatedRouterID;
    bool designatedRouterDeclared;

    do {
        // calculating backup designated router
        declaredBackup = OSPF::NULL_DESIGNATEDROUTERID;
        declaredBackupPriority = 0;
        declaredBackupID = OSPF::NULL_ROUTERID;
        backupDeclared = false;

        OSPF::DesignatedRouterID highestRouter = OSPF::NULL_DESIGNATEDROUTERID;
        unsigned char highestPriority = 0;
        OSPF::RouterID highestID = OSPF::NULL_ROUTERID;

        for (i = 0; i < neighborCount; i++) {
            OSPF::Neighbor* neighbor = intf->neighboringRouters[i];
            unsigned char neighborPriority = neighbor->getPriority();

            if (neighbor->getState() < OSPF::Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            OSPF::RouterID neighborID = neighbor->getNeighborID();
            OSPF::DesignatedRouterID neighborsDesignatedRouter = neighbor->getDesignatedRouter();
            OSPF::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->getBackupDesignatedRouter();

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
                    } else {
                        declaredBackup = highestRouter;
                        declaredBackupPriority = highestPriority;
                        declaredBackupID = highestID;
                        backupDeclared = true;
                    }
                }
            }
        }

        // calculating backup designated router
        declaredDesignatedRouter = OSPF::NULL_DESIGNATEDROUTERID;
        declaredDesignatedRouterPriority = 0;
        declaredDesignatedRouterID = OSPF::NULL_ROUTERID;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            OSPF::Neighbor* neighbor = intf->neighboringRouters[i];
            unsigned char neighborPriority = neighbor->getPriority();

            if (neighbor->getState() < OSPF::Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            OSPF::RouterID neighborID = neighbor->getNeighborID();
            OSPF::DesignatedRouterID neighborsDesignatedRouter = neighbor->getDesignatedRouter();
            OSPF::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->getBackupDesignatedRouter();

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
                (declaredDesignatedRouter.routerID != OSPF::NULL_ROUTERID) &&
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
                (declaredBackup.routerID != OSPF::NULL_ROUTERID) &&
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
        } else {
            repeatCount += 2;
        }

    } while (repeatCount < 2);

    OSPF::RouterID routersOldDesignatedRouterID = intf->designatedRouter.routerID;
    OSPF::RouterID routersOldBackupID = intf->backupDesignatedRouter.routerID;

    intf->designatedRouter = declaredDesignatedRouter;
    intf->backupDesignatedRouter = declaredBackup;

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter = (routersOldDesignatedRouterID == routerID);
    bool wasOther = (intf->getState() == OSPF::Interface::NOT_DESIGNATED_ROUTER_STATE);
    bool wasWaiting = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter = (declaredBackup.routerID == routerID);
    bool isDesignatedRouter = (declaredDesignatedRouter.routerID == routerID);
    bool isOther = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isOther) {
            changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPF::InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasOther) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPF::InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPF::InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == OSPF::Interface::NBMA) &&
            ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
             (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->neighboringRouters[i]->getPriority() == 0) {
                intf->neighboringRouters[i]->processEvent(OSPF::Neighbor::START);
            }
        }
        if ((declaredDesignatedRouter.routerID != routersOldDesignatedRouterID) ||
            (declaredBackup.routerID != routersOldBackupID))
        {
            if (intf->neighboringRouters[i]->getState() >= OSPF::Neighbor::TWOWAY_STATE) {
                intf->neighboringRouters[i]->processEvent(OSPF::Neighbor::IS_ADJACENCY_OK);
            }
        }
    }
}
