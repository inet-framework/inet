//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/interface/Ospfv2InterfaceState.h"

#include <map>

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateBackup.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateDesignatedRouter.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateNotDesignatedRouter.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

void Ospfv2InterfaceState::changeState(Ospfv2Interface *intf, Ospfv2InterfaceState *newState, Ospfv2InterfaceState *currentState)
{
    Ospfv2Interface::Ospfv2InterfaceStateType oldState = currentState->getState();
    Ospfv2Interface::Ospfv2InterfaceStateType nextState = newState->getState();
    Ospfv2Interface::Ospfv2InterfaceType intfType = intf->getType();
    bool shouldRebuildRoutingTable = false;

    intf->changeState(newState, currentState);

    if ((oldState == Ospfv2Interface::DOWN_STATE) ||
        (nextState == Ospfv2Interface::DOWN_STATE) ||
        (oldState == Ospfv2Interface::LOOPBACK_STATE) ||
        (nextState == Ospfv2Interface::LOOPBACK_STATE) ||
        (oldState == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
        (nextState == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
        ((intfType == Ospfv2Interface::POINTTOPOINT) &&
         ((oldState == Ospfv2Interface::POINTTOPOINT_STATE) ||
          (nextState == Ospfv2Interface::POINTTOPOINT_STATE))) ||
        (((intfType == Ospfv2Interface::BROADCAST) ||
          (intfType == Ospfv2Interface::NBMA)) &&
         ((oldState == Ospfv2Interface::WAITING_STATE) ||
          (nextState == Ospfv2Interface::WAITING_STATE))))
    {
        RouterLsa *routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());

        if (routerLSA != nullptr) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
                intf->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            }
            else {
                RouterLsa *newLSA = intf->getArea()->originateRouterLSA();

                newLSA->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
                delete newLSA;

                intf->getArea()->floodLSA(routerLSA);
            }
        }
        else { // (lsa == nullptr) -> This must be the first time any interface is up...
            RouterLsa *newLSA = intf->getArea()->originateRouterLSA();

            shouldRebuildRoutingTable |= intf->getArea()->installRouterLSA(newLSA);

            routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());

            intf->getArea()->setSPFTreeRoot(routerLSA);
            intf->getArea()->floodLSA(newLSA);
            delete newLSA;
        }
    }

    if (nextState == Ospfv2Interface::DESIGNATED_ROUTER_STATE) {
        NetworkLsa *newLSA = intf->getArea()->originateNetworkLSA(intf);
        if (newLSA != nullptr) {
            shouldRebuildRoutingTable |= intf->getArea()->installNetworkLSA(newLSA);

            intf->getArea()->floodLSA(newLSA);
            delete newLSA;
        }
        else { // no neighbors on the network -> old NetworkLsa must be flushed
            NetworkLsa *oldLSA = intf->getArea()->findNetworkLSA(intf->getAddressRange().address);

            if (oldLSA != nullptr) {
                oldLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
                intf->getArea()->floodLSA(oldLSA);
                oldLSA->incrementInstallTime();
            }
        }
    }

    if (oldState == Ospfv2Interface::DESIGNATED_ROUTER_STATE) {
        NetworkLsa *networkLSA = intf->getArea()->findNetworkLSA(intf->getAddressRange().address);

        if (networkLSA != nullptr) {
            networkLSA->getHeaderForUpdate().setLsAge(MAX_AGE);
            intf->getArea()->floodLSA(networkLSA);
            networkLSA->incrementInstallTime();
        }
    }

    if (shouldRebuildRoutingTable) {
        intf->getArea()->getRouter()->rebuildRoutingTable();
    }
}

void Ospfv2InterfaceState::calculateDesignatedRouter(Ospfv2Interface *intf)
{
    RouterId routerID = intf->parentArea->getRouter()->getRouterID();
    DesignatedRouterId currentDesignatedRouter = intf->designatedRouter;
    DesignatedRouterId currentBackupRouter = intf->backupDesignatedRouter;

    unsigned int neighborCount = intf->neighboringRouters.size();
    unsigned char repeatCount = 0;
    unsigned int i;

    // BDR
    DesignatedRouterId declaredBackup;
    unsigned char declaredBackupPriority;
    RouterId declaredBackupID;
    bool backupDeclared;

    // DR
    DesignatedRouterId declaredDesignatedRouter;
    unsigned char declaredDesignatedRouterPriority;
    RouterId declaredDesignatedRouterID;
    bool designatedRouterDeclared;

    do {
        // calculating backup designated router
        declaredBackup = NULL_DESIGNATEDROUTERID;
        declaredBackupPriority = 0;
        declaredBackupID = NULL_ROUTERID;
        backupDeclared = false;

        DesignatedRouterId highestRouter = NULL_DESIGNATEDROUTERID;
        unsigned char highestPriority = 0;
        RouterId highestID = NULL_ROUTERID;

        for (i = 0; i < neighborCount; i++) {
            Neighbor *neighbor = intf->neighboringRouters[i];
            unsigned char neighborPriority = neighbor->getPriority();

            if (neighbor->getState() < Neighbor::TWOWAY_STATE || neighborPriority == 0)
                continue;

            RouterId neighborID = neighbor->getNeighborID();
            DesignatedRouterId neighborsDesignatedRouter = neighbor->getDesignatedRouter();
            DesignatedRouterId neighborsBackupDesignatedRouter = neighbor->getBackupDesignatedRouter();

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

        // calculating designated router
        declaredDesignatedRouter = NULL_DESIGNATEDROUTERID;
        declaredDesignatedRouterPriority = 0;
        declaredDesignatedRouterID = NULL_ROUTERID;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            Neighbor *neighbor = intf->neighboringRouters[i];
            unsigned char neighborPriority = neighbor->getPriority();

            if (neighbor->getState() < Neighbor::TWOWAY_STATE || neighborPriority == 0)
                continue;

            RouterId neighborID = neighbor->getNeighborID();
            DesignatedRouterId neighborsDesignatedRouter = neighbor->getDesignatedRouter();
            DesignatedRouterId neighborsBackupDesignatedRouter = neighbor->getBackupDesignatedRouter();

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

    printElectionResult(intf, declaredDesignatedRouter, declaredBackup);

    RouterId routersOldDesignatedRouterID = intf->designatedRouter.routerID;
    RouterId routersOldBackupID = intf->backupDesignatedRouter.routerID;

    intf->designatedRouter = declaredDesignatedRouter;
    intf->backupDesignatedRouter = declaredBackup;

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter = (routersOldDesignatedRouterID == routerID);
    bool wasOther = (intf->getState() == Ospfv2Interface::NOT_DESIGNATED_ROUTER_STATE);
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
        if ((intf->interfaceType == Ospfv2Interface::NBMA) &&
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

void Ospfv2InterfaceState::printElectionResult(const Ospfv2Interface *onInterface, DesignatedRouterId DR, DesignatedRouterId BDR)
{
    EV_DETAIL << "DR/BDR election is done for interface[" << static_cast<short>(onInterface->getIfIndex()) << "] ";
    switch (onInterface->getType()) {
        case Ospfv2Interface::BROADCAST:
            EV_DETAIL << "(Broadcast)";
            break;

        case Ospfv2Interface::NBMA:
            EV_DETAIL << "(NBMA)";
            break;

        default:
            EV_DETAIL << "(Unknown)";
            break;
    }
    EV_DETAIL << " (state: " << onInterface->getStateString(onInterface->getState()) << "):";
    EV_DETAIL << "\n";
    EV_DETAIL << "   DR id: " << DR.routerID.str(false) << ", DR interface: " << DR.ipInterfaceAddress.str(false);
    EV_DETAIL << "\n";
    EV_DETAIL << "   BDR id: " << BDR.routerID.str(false) << ", BDR interface: " << BDR.ipInterfaceAddress.str(false);
    EV_DETAIL << "\n";
}

} // namespace ospfv2

} // namespace inet

