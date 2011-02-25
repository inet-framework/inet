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

#include "OSPFInterfaceState.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"
#include "OSPFInterfaceStateDesignatedRouter.h"
#include "OSPFInterfaceStateNotDesignatedRouter.h"
#include "OSPFInterfaceStateBackup.h"
#include <map>

void OSPF::InterfaceState::ChangeState(OSPF::Interface* intf, OSPF::InterfaceState* newState, OSPF::InterfaceState* currentState)
{
    OSPF::Interface::InterfaceStateType oldState            = currentState->GetState();
    OSPF::Interface::InterfaceStateType nextState           = newState->GetState();
    OSPF::Interface::OSPFInterfaceType  intfType            = intf->GetType();
    bool                                rebuildRoutingTable = false;

    intf->ChangeState(newState, currentState);

    if ((oldState == OSPF::Interface::DownState) ||
        (nextState == OSPF::Interface::DownState) ||
        (oldState == OSPF::Interface::LoopbackState) ||
        (nextState == OSPF::Interface::LoopbackState) ||
        (oldState == OSPF::Interface::DesignatedRouterState) ||
        (nextState == OSPF::Interface::DesignatedRouterState) ||
        ((intfType == OSPF::Interface::PointToPoint) &&
         ((oldState == OSPF::Interface::PointToPointState) ||
          (nextState == OSPF::Interface::PointToPointState))) ||
        (((intfType == OSPF::Interface::Broadcast) ||
          (intfType == OSPF::Interface::NBMA)) &&
         ((oldState == OSPF::Interface::WaitingState) ||
          (nextState == OSPF::Interface::WaitingState))))
    {
        OSPF::RouterLSA* routerLSA = intf->GetArea()->FindRouterLSA(intf->GetArea()->GetRouter()->GetRouterID());

        if (routerLSA != NULL) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsAge(MAX_AGE);
                intf->GetArea()->FloodLSA(routerLSA);
                routerLSA->IncrementInstallTime();
            } else {
                OSPF::RouterLSA* newLSA = intf->GetArea()->OriginateRouterLSA();

                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                rebuildRoutingTable |= routerLSA->Update(newLSA);
                delete newLSA;

                intf->GetArea()->FloodLSA(routerLSA);
            }
        } else {  // (lsa == NULL) -> This must be the first time any interface is up...
            OSPF::RouterLSA* newLSA = intf->GetArea()->OriginateRouterLSA();

            rebuildRoutingTable |= intf->GetArea()->InstallRouterLSA(newLSA);

            routerLSA = intf->GetArea()->FindRouterLSA(intf->GetArea()->GetRouter()->GetRouterID());

            intf->GetArea()->SetSPFTreeRoot(routerLSA);
            intf->GetArea()->FloodLSA(newLSA);
            delete newLSA;
        }
    }

    if (nextState == OSPF::Interface::DesignatedRouterState) {
        OSPF::NetworkLSA* newLSA = intf->GetArea()->OriginateNetworkLSA(intf);
        if (newLSA != NULL) {
            rebuildRoutingTable |= intf->GetArea()->InstallNetworkLSA(newLSA);

            intf->GetArea()->FloodLSA(newLSA);
            delete newLSA;
        } else {    // no neighbors on the network -> old NetworkLSA must be flushed
            OSPF::NetworkLSA* oldLSA = intf->GetArea()->FindNetworkLSA(ULongFromIPv4Address(intf->GetAddressRange().address));

            if (oldLSA != NULL) {
                oldLSA->getHeader().setLsAge(MAX_AGE);
                intf->GetArea()->FloodLSA(oldLSA);
                oldLSA->IncrementInstallTime();
            }
        }
    }

    if (oldState == OSPF::Interface::DesignatedRouterState) {
        OSPF::NetworkLSA* networkLSA = intf->GetArea()->FindNetworkLSA(ULongFromIPv4Address(intf->GetAddressRange().address));

        if (networkLSA != NULL) {
            networkLSA->getHeader().setLsAge(MAX_AGE);
            intf->GetArea()->FloodLSA(networkLSA);
            networkLSA->IncrementInstallTime();
        }
    }

    if (rebuildRoutingTable) {
        intf->GetArea()->GetRouter()->RebuildRoutingTable();
    }
}

void OSPF::InterfaceState::CalculateDesignatedRouter(OSPF::Interface* intf)
{
    OSPF::RouterID           routerID                = intf->parentArea->GetRouter()->GetRouterID();
    OSPF::DesignatedRouterID currentDesignatedRouter = intf->designatedRouter;
    OSPF::DesignatedRouterID currentBackupRouter     = intf->backupDesignatedRouter;

    unsigned int             neighborCount           = intf->neighboringRouters.size();
    unsigned char            repeatCount             = 0;
    unsigned int             i;

    OSPF::DesignatedRouterID declaredBackup;
    unsigned char            declaredBackupPriority;
    OSPF::RouterID           declaredBackupID;
    bool                     backupDeclared;

    OSPF::DesignatedRouterID declaredDesignatedRouter;
    unsigned char            declaredDesignatedRouterPriority;
    OSPF::RouterID           declaredDesignatedRouterID;
    bool                     designatedRouterDeclared;

    do {
        // calculating backup designated router
        declaredBackup = OSPF::NullDesignatedRouterID;
        declaredBackupPriority = 0;
        declaredBackupID = OSPF::NullRouterID;
        backupDeclared = false;

        OSPF::DesignatedRouterID highestRouter                 = OSPF::NullDesignatedRouterID;
        unsigned char            highestPriority               = 0;
        OSPF::RouterID           highestID                     = OSPF::NullRouterID;

        for (i = 0; i < neighborCount; i++) {
            OSPF::Neighbor* neighbor         = intf->neighboringRouters[i];
            unsigned char   neighborPriority = neighbor->GetPriority();

            if (neighbor->GetState() < OSPF::Neighbor::TwoWayState) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            OSPF::RouterID           neighborID                      = neighbor->GetNeighborID();
            OSPF::DesignatedRouterID neighborsDesignatedRouter       = neighbor->GetDesignatedRouter();
            OSPF::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->GetBackupDesignatedRouter();

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
                        highestRouter.ipInterfaceAddress = neighbor->GetAddress();
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
        declaredDesignatedRouter = OSPF::NullDesignatedRouterID;
        declaredDesignatedRouterPriority = 0;
        declaredDesignatedRouterID = OSPF::NullRouterID;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            OSPF::Neighbor* neighbor         = intf->neighboringRouters[i];
            unsigned char   neighborPriority = neighbor->GetPriority();

            if (neighbor->GetState() < OSPF::Neighbor::TwoWayState) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            OSPF::RouterID           neighborID                      = neighbor->GetNeighborID();
            OSPF::DesignatedRouterID neighborsDesignatedRouter       = neighbor->GetDesignatedRouter();
            OSPF::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->GetBackupDesignatedRouter();

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
        //FIXME  suggest parentheses around && within ||
        if (
            (
                (declaredDesignatedRouter.routerID != OSPF::NullRouterID) &&
                (
                    (currentDesignatedRouter.routerID == routerID) &&
                    (declaredDesignatedRouter.routerID != routerID)
                ) ||
                (
                    (currentDesignatedRouter.routerID != routerID) &&
                    (declaredDesignatedRouter.routerID == routerID)
                )
            ) ||
            (
                (declaredBackup.routerID != OSPF::NullRouterID) &&
                (
                    (currentBackupRouter.routerID == routerID) &&
                    (declaredBackup.routerID != routerID)
                ) ||
                (
                    (currentBackupRouter.routerID != routerID) &&
                    (declaredBackup.routerID == routerID)
                )
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
    OSPF::RouterID routersOldBackupID           = intf->backupDesignatedRouter.routerID;

    intf->designatedRouter = declaredDesignatedRouter;
    intf->backupDesignatedRouter = declaredBackup;

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter       = (routersOldDesignatedRouterID == routerID);
    bool wasOther                  = (intf->GetState() == OSPF::Interface::NotDesignatedRouterState);
    bool wasWaiting                = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter  = (declaredBackup.routerID == routerID);
    bool isDesignatedRouter        = (declaredDesignatedRouter.routerID == routerID);
    bool isOther                   = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            ChangeState(intf, new OSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isOther) {
            ChangeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            ChangeState(intf, new OSPF::InterfaceStateBackup, this);
        }
        if (isOther) {
            ChangeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasOther) {
        if (isDesignatedRouter) {
            ChangeState(intf, new OSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            ChangeState(intf, new OSPF::InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            ChangeState(intf, new OSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            ChangeState(intf, new OSPF::InterfaceStateBackup, this);
        }
        if (isOther) {
            ChangeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == OSPF::Interface::NBMA) &&
            ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
             (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->neighboringRouters[i]->GetPriority() == 0) {
                intf->neighboringRouters[i]->ProcessEvent(OSPF::Neighbor::Start);
            }
        }
        if ((declaredDesignatedRouter.routerID != routersOldDesignatedRouterID) ||
            (declaredBackup.routerID != routersOldBackupID))
        {
            if (intf->neighboringRouters[i]->GetState() >= OSPF::Neighbor::TwoWayState) {
                intf->neighboringRouters[i]->ProcessEvent(OSPF::Neighbor::IsAdjacencyOK);
            }
        }
    }
}
