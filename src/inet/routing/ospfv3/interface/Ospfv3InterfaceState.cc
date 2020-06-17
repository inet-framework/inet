
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateBackup.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDr.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDrOther.h"

namespace inet {
namespace ospfv3 {

void Ospfv3InterfaceState::changeState(Ospfv3Interface *interface, Ospfv3InterfaceState *newState, Ospfv3InterfaceState *currentState)
{
    Ospfv3Interface::Ospfv3InterfaceFaState oldState = currentState->getState();
    Ospfv3Interface::Ospfv3InterfaceFaState nextState = newState->getState();
    Ospfv3Interface::Ospfv3InterfaceType intfType = interface->getType();
    bool shouldRebuildRoutingTable = false;

    EV_DEBUG << "CHANGE STATE: " << interface->getArea()->getInstance()->getProcess()->getRouterID() << ", area " << interface->getArea()->getAreaID() << ", intf " << interface->getIntName() << ", curr st: " << currentState->getInterfaceStateString() << ", new st: " << newState->getInterfaceStateString() << "\n";

    interface->changeState(currentState, newState);

    //change of state -> new router LSA needs to be generated

    if ((oldState == Ospfv3Interface::INTERFACE_STATE_DOWN) ||
         (nextState == Ospfv3Interface::INTERFACE_STATE_DOWN) ||
         (oldState == Ospfv3Interface::INTERFACE_STATE_LOOPBACK) ||
         (nextState == Ospfv3Interface::INTERFACE_STATE_LOOPBACK) ||
         (oldState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED) ||
         (nextState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED) ||

         ((intfType == Ospfv3Interface::POINTTOMULTIPOINT_TYPE) && ((oldState == Ospfv3Interface::INTERFACE_STATE_POINTTOPOINT) ||
           (nextState == Ospfv3Interface::INTERFACE_STATE_POINTTOPOINT))) ||

         (((intfType == Ospfv3Interface::BROADCAST_TYPE) ||
           (intfType == Ospfv3Interface::NBMA_TYPE)) && (/*(oldState == Ospfv3Interface::INTERFACE_STATE_WAITING) ||*/
           (nextState == Ospfv3Interface::INTERFACE_STATE_WAITING))))
    {
        LSAKeyType lsaKey;
        lsaKey.LSType = Ospfv3LsaFunctionCode::ROUTER_LSA;
        lsaKey.advertisingRouter = interface->getArea()->getInstance()->getProcess()->getRouterID();
        lsaKey.linkStateID = interface->getArea()->getInstance()->getProcess()->getRouterID();

        RouterLSA *routerLSA = interface->getArea()->getRouterLSAbyKey(lsaKey);
        if (routerLSA != nullptr)
        {
            long sequenceNumber = routerLSA->getHeader().getLsaSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER)
            {
                routerLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                interface->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            }
            else
            {
                RouterLSA *newLSA = interface->getArea()->originateRouterLSA();

                newLSA->getHeaderForUpdate().setLsaSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= interface->getArea()->installRouterLSA(newLSA);
                routerLSA = interface->getArea()->findRouterLSA(interface->getArea()->getInstance()->getProcess()->getRouterID());
                interface->getArea()->setSpfTreeRoot(routerLSA);
                interface->getArea()->floodLSA(newLSA);
                delete newLSA;
            }
        }
        else // (lsa == nullptr) -> This must be the first time any interface is up...
        {
            RouterLSA *newLSA = interface->getArea()->originateRouterLSA();

            shouldRebuildRoutingTable |= interface->getArea()->installRouterLSA(newLSA);
            if (shouldRebuildRoutingTable)
            {
                routerLSA = interface->getArea()->findRouterLSA(interface->getArea()->getInstance()->getProcess()->getRouterID());
                interface->getArea()->setSpfTreeRoot(routerLSA);
                interface->getArea()->floodLSA(newLSA);
            }
            delete newLSA;
        }
    }
    if (oldState == Ospfv3Interface::INTERFACE_STATE_WAITING)
    {
        RouterLSA* routerLSA;
        int routerLSAcount = interface->getArea()->getRouterLSACount();
        int i = 0;
        while (i < routerLSAcount)
        {
            routerLSA = interface->getArea()->getRouterLSA(i);
            const Ospfv3LsaHeader &header = routerLSA->getHeader();
            if (header.getAdvertisingRouter() == interface->getArea()->getInstance()->getProcess()->getRouterID() &&
                    interface->getType() == Ospfv3Interface::BROADCAST_TYPE) {
                EV_DEBUG << "Changing state -> deleting the old router LSA\n";
                interface->getArea()->deleteRouterLSA(i);
            }

            if (interface->getArea()->getRouterLSACount() == routerLSAcount)
                i++;
            else //some routerLSA was deleted
            {
                routerLSAcount = interface->getArea()->getRouterLSACount();
                i = 0;
            }
        }

        EV_DEBUG << "Changing state -> new Router LSA\n";
        RouterLSA* newLSA = interface->getArea()->originateRouterLSA();
        if (newLSA != nullptr)
        {
            if (interface->getArea()->installRouterLSA(newLSA))
            {
                routerLSA = interface->getArea()->findRouterLSA(interface->getArea()->getInstance()->getProcess()->getRouterID());
                interface->getArea()->setSpfTreeRoot(routerLSA);
                interface->getArea()->floodLSA(newLSA);
            }
            delete newLSA;
        }
        if (interface->getType() == Ospfv3Interface::POINTTOPOINT_TYPE || (interface->getArea()->hasAnyPassiveInterface()))
        {
            Ospfv3IntraAreaPrefixLsa* prefLSA  = interface->getArea()->originateIntraAreaPrefixLSA();
            if (prefLSA != nullptr)
            {
                if (interface->getArea()->installIntraAreaPrefixLSA(prefLSA))
                {
                    interface->getArea()->floodLSA(prefLSA);
                }
                delete prefLSA;
            }
        }

        if (nextState == Ospfv3Interface::INTERFACE_STATE_BACKUP ||
           nextState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED ||
           nextState == Ospfv3Interface::INTERFACE_STATE_DROTHER) {
            interface->setTransitNetInt(true);//this interface is in broadcast network

            // if router has any interface as Passive, create separate Intra-Area-Prefix-LSA for these interfaces
            if (interface->getArea()->hasAnyPassiveInterface()) {
                Ospfv3IntraAreaPrefixLsa* prefLSA  = interface->getArea()->originateIntraAreaPrefixLSA();
                if (prefLSA != nullptr) {
                    if (interface->getArea()->installIntraAreaPrefixLSA(prefLSA))
                        interface->getArea()->floodLSA(prefLSA);
                    delete prefLSA;
                }
            }
        }
        shouldRebuildRoutingTable = true;
    }

    if (nextState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED) {
        NetworkLSA* newLSA = interface->getArea()->originateNetworkLSA(interface);
        if (newLSA != nullptr) {
            shouldRebuildRoutingTable |= interface->getArea()->installNetworkLSA(newLSA);
            if (shouldRebuildRoutingTable) {
                Ospfv3IntraAreaPrefixLsa* prefLSA = interface->getArea()->originateNetIntraAreaPrefixLSA(newLSA, interface, false);
                if (prefLSA != nullptr) {
                    interface->getArea()->installIntraAreaPrefixLSA(prefLSA);
                    InterfaceEntry* ie = interface->containingProcess->ift->getInterfaceById(interface->getInterfaceId());
                    auto& ipv6int = ie->getProtocolDataForUpdate<Ipv6InterfaceData>();
                    ipv6int->joinMulticastGroup(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);

                    interface->getArea()->floodLSA(newLSA);
                    interface->getArea()->floodLSA(prefLSA);
                }
                delete prefLSA;
            }
            delete newLSA;
        }
        else {
            NetworkLSA *networkLSA = interface->getArea()->findNetworkLSAByLSID(Ipv4Address(interface->getInterfaceId()));
            if (networkLSA != nullptr) {
                networkLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                interface->getArea()->floodLSA(networkLSA);
                networkLSA->incrementInstallTime();
            }
            // here I am in state when interface comes to DR state but there is no neighbor on this iface.
            // so new LSA type 9 need to be created
//            if (interface->getArea()->getInstance()->getAreaCount() > 1) { //this is ABR
//                interface->getArea()->originateInterAreaPrefixLSA(prefLSA, interface->getArea(), false);
//            }
        }
        Ospfv3IntraAreaPrefixLsa* prefLSA = interface->getArea()->originateIntraAreaPrefixLSA();
        if (prefLSA != nullptr) {
            shouldRebuildRoutingTable |= interface->getArea()->installIntraAreaPrefixLSA(prefLSA);
            interface->getArea()->floodLSA(prefLSA);
            delete prefLSA;
        }
    }

    if (nextState ==  Ospfv3Interface::INTERFACE_STATE_BACKUP) {
        InterfaceEntry* ie = interface->containingProcess->ift->getInterfaceById(interface->getInterfaceId());
        auto& ipv6int = ie->getProtocolDataForUpdate<Ipv6InterfaceData>();
        ipv6int->joinMulticastGroup(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
    }

    if ((oldState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED )&& (nextState != Ospfv3Interface::INTERFACE_STATE_DESIGNATED)) {
        NetworkLSA *networkLSA = interface->getArea()->findNetworkLSA(interface->getInterfaceId(), interface->getArea()->getInstance()->getProcess()->getRouterID());
//
        if (networkLSA != nullptr) {
            networkLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
            interface->getArea()->floodLSA(networkLSA);
            networkLSA->incrementInstallTime();
        }
    }

    if (shouldRebuildRoutingTable) {
        interface->getArea()->getInstance()->getProcess()->rebuildRoutingTable();
    }

    if ((oldState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED || oldState == Ospfv3Interface::INTERFACE_STATE_BACKUP) &&
        (nextState != Ospfv3Interface::INTERFACE_STATE_DESIGNATED && nextState != Ospfv3Interface::INTERFACE_STATE_BACKUP))
    {
        InterfaceEntry* ie = interface->containingProcess->ift->getInterfaceById(interface->getInterfaceId());
        auto& ipv6int = ie->findProtocolDataForUpdate<Ipv6InterfaceData>();
        ipv6int->leaveMulticastGroup(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
    }
}

void Ospfv3InterfaceState::calculateDesignatedRouter(Ospfv3Interface *intf) {
    Ipv4Address routerID = intf->getArea()->getInstance()->getProcess()->getRouterID();
    Ipv4Address currentDesignatedRouter = intf->getDesignatedID();
    Ipv4Address currentBackupRouter = intf->getBackupID();
    EV_DEBUG << "Calculating the designated router, currentDesignated:" << currentDesignatedRouter << ", current backup: " << currentBackupRouter << "\n";

    unsigned int neighborCount = intf->getNeighborCount();
    unsigned char repeatCount = 0;
    unsigned int i;

    Ipv6Address declaredBackupIP;
    int declaredBackupPriority;
    Ipv4Address declaredBackupID;
    bool backupDeclared;

    Ipv6Address declaredDesignatedRouterIP;
    int declaredDesignatedRouterPriority;
    Ipv4Address declaredDesignatedRouterID;
    bool designatedRouterDeclared;

    do {
        // calculating backup designated router
        declaredBackupIP = Ipv6Address::UNSPECIFIED_ADDRESS;
        declaredBackupPriority = 0;
        declaredBackupID = NULL_IPV4ADDRESS;
        backupDeclared = false;

        Ipv4Address highestRouter = NULL_IPV4ADDRESS;
        L3Address highestRouterIP = Ipv6Address::UNSPECIFIED_ADDRESS;
        int highestPriority = 0;

        for (i = 0; i < neighborCount; i++) {
            Ospfv3Neighbor *neighbor = intf->getNeighbor(i);
            int neighborPriority = neighbor->getNeighborPriority();

            if (neighbor->getState() < Ospfv3Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            Ipv4Address neighborID = neighbor->getNeighborID();
            Ipv4Address neighborsDesignatedRouterID = neighbor->getNeighborsDR();
            Ipv4Address neighborsBackupDesignatedRouterID = neighbor->getNeighborsBackup();

//            EV_DEBUG << "Neighbors DR: " << neighborsDesignatedRouterID << ", neighbors backup: " << neighborsBackupDesignatedRouterID << "\n";
            if (neighborsDesignatedRouterID != neighborID) {
//                EV_DEBUG << "Router " << routerID << " trying backup on neighbor " << neighborID << "\n";
                if (neighborsBackupDesignatedRouterID == neighborID) {
                    if ((neighborPriority > declaredBackupPriority) ||
                            ((neighborPriority == declaredBackupPriority) &&
                                    (neighborID > declaredBackupID)))
                    {
                        declaredBackupID = neighborsBackupDesignatedRouterID;
                        declaredBackupPriority = neighborPriority;
                        declaredBackupIP = neighbor->getNeighborIP();
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((neighborPriority > highestPriority) ||
                            ((neighborPriority == highestPriority) &&
                                    (neighborID > highestRouter)))
                    {
                        highestRouter = neighborID;
                        highestRouterIP = neighbor->getNeighborIP();
                        highestPriority = neighborPriority;
                    }
                }
            }
//            EV_DEBUG << "Router " << routerID << " declared backup is " << declaredBackupID << "\n";
        }

        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter != routerID) {
                if (currentBackupRouter == routerID) {
                    if ((intf->routerPriority > declaredBackupPriority) ||
                            ((intf->routerPriority == declaredBackupPriority) &&
                                    (routerID > declaredBackupID)))
                    {
                        declaredBackupID = routerID;
                        declaredBackupIP = intf->getInterfaceLLIP();
                        declaredBackupPriority = intf->getRouterPriority();
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((intf->routerPriority > highestPriority) ||
                            ((intf->routerPriority == highestPriority) &&
                                    (routerID > highestRouter)))
                    {
                        declaredBackupID = routerID;
                        declaredBackupIP = intf->getInterfaceLLIP();
                        declaredBackupPriority = intf->getRouterPriority();
                        backupDeclared = true;
                    }
                    else {
                        declaredBackupID = highestRouter;
                        declaredBackupPriority = highestPriority;
                        backupDeclared = true;
                    }
                }
            }
        }
//        EV_DEBUG << "Router " << routerID << " declared backup after backup round is " << declaredBackupID << "\n";
        // calculating designated router
        declaredDesignatedRouterID = NULL_IPV4ADDRESS;
        declaredDesignatedRouterPriority = 0;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            Ospfv3Neighbor *neighbor = intf->getNeighbor(i);
            unsigned short neighborPriority = neighbor->getNeighborPriority();

            if (neighbor->getState() < Ospfv3Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            Ipv4Address neighborID = neighbor->getNeighborID();
            Ipv4Address neighborsDesignatedRouterID = neighbor->getNeighborsDR();
            Ipv4Address neighborsBackupDesignatedRouterID = neighbor->getNeighborsBackup();

            if (neighborsDesignatedRouterID == neighborID) {
                if ((neighborPriority > declaredDesignatedRouterPriority) ||
                        ((neighborPriority == declaredDesignatedRouterPriority) &&
                                (neighborID > declaredDesignatedRouterID)))
                {
//                    declaredDesignatedRouterID = neighborsDesignatedRouterID;
                    declaredDesignatedRouterPriority = neighborPriority;
                    declaredDesignatedRouterID = neighborID;
                    designatedRouterDeclared = true;
                }
            }
        }
//        EV_DEBUG << "Router " << routerID << " declared DR after neighbors is " << declaredDesignatedRouterID << "\n";
        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter == routerID) {
                if ((intf->routerPriority > declaredDesignatedRouterPriority) ||
                        ((intf->routerPriority == declaredDesignatedRouterPriority) &&
                                (routerID > declaredDesignatedRouterID)))
                {
                    declaredDesignatedRouterID = routerID;
                    declaredDesignatedRouterIP = intf->getInterfaceLLIP();
                    declaredDesignatedRouterPriority = intf->getRouterPriority();
                    designatedRouterDeclared = true;
                }
            }
        }
        if (!designatedRouterDeclared) {
            declaredDesignatedRouterID = declaredBackupID;
            declaredDesignatedRouterPriority = declaredBackupPriority;
            designatedRouterDeclared = true;
        }
//        EV_DEBUG << "Router " << routerID << " declared DR after a round is " << declaredDesignatedRouterID << "\n";
        // if the router is any kind of DR or is no longer one of them, then repeat
        if (
                (
                        (declaredDesignatedRouterID != NULL_IPV4ADDRESS) &&
                        ((      //if this router is not the DR but it was before
                                (currentDesignatedRouter == routerID) &&
                                (declaredDesignatedRouterID != routerID)
                        ) ||
                                (//if this router was not the DR but now it is
                                        (currentDesignatedRouter != routerID) &&
                                        (declaredDesignatedRouterID == routerID)
                                ))
                ) ||
                (
                        (declaredBackupID != NULL_IPV4ADDRESS) &&
                        ((
                                (currentBackupRouter == routerID) &&
                                (declaredBackupID != routerID)
                        ) ||
                                (
                                        (currentBackupRouter != routerID) &&
                                        (declaredBackupID == routerID)
                                ))
                )
        )
        {
            currentDesignatedRouter = declaredDesignatedRouterID;
            currentBackupRouter = declaredBackupID;
            repeatCount++;
        }
        else {
            repeatCount += 2;
        }
    } while (repeatCount < 2);

    Ipv4Address routersOldDesignatedRouterID = intf->getDesignatedID();
    Ipv4Address routersOldBackupID = intf->getBackupID();

    intf->setDesignatedID(declaredDesignatedRouterID);
    EV_DEBUG <<  "declaredDesignatedRouterID = " << declaredDesignatedRouterID << "\n";
    EV_DEBUG <<  "declaredBackupID = " << declaredBackupID << "\n";
    for (int g = 0 ; g <  intf->getNeighborCount(); g++) {
        EV_DEBUG << "@" << g <<  "  - " <<  intf->getNeighbor(g)->getNeighborID() << " / " << intf->getNeighbor(g)->getNeighborInterfaceID()  << "\n";
    }

    Ospfv3Neighbor* DRneighbor = intf->getNeighborById(declaredDesignatedRouterID);
    if (DRneighbor == nullptr) {
        EV_DEBUG << "DRneighbor == nullptr\n";
        intf->setDesignatedIntID(intf->getInterfaceId());
    }
    else {
//        intf->setDesignatedIntID(DRneighbor->getInterface()->getInterfaceId());
        intf->setDesignatedIntID(DRneighbor->getNeighborInterfaceID());
    }
    intf->setBackupID(declaredBackupID);

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter = (routersOldDesignatedRouterID == routerID);
    bool wasOther = (intf->getState() == Ospfv3Interface::INTERFACE_STATE_DROTHER);
    bool wasWaiting = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter = (declaredBackupID == routerID);
    bool isDesignatedRouter = (declaredDesignatedRouterID == routerID);
    bool isOther = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            changeState(intf, new Ospfv3InterfaceStateDr, this);
        }
        if (isOther) {
            changeState(intf, new Ospfv3InterfaceStateDrOther, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            changeState(intf, new Ospfv3InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new Ospfv3InterfaceStateDrOther, this);
        }
        if (isDesignatedRouter) { // for case that link between routers was disconnected and Router become DR again
            changeState(intf, new Ospfv3InterfaceStateDr, this);
        }

    }
    if (wasOther) {
        if (isDesignatedRouter) {
            changeState(intf, new Ospfv3InterfaceStateDr, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new Ospfv3InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            changeState(intf, new Ospfv3InterfaceStateDr, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new Ospfv3InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new Ospfv3InterfaceStateDrOther, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == Ospfv3Interface::NBMA_TYPE) &&
                ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
                        (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->getNeighbor(i)->getNeighborPriority() == 0) {
                intf->getNeighbor(i)->processEvent(Ospfv3Neighbor::START);
            }
        }
        if ((declaredDesignatedRouterID != routersOldDesignatedRouterID) ||
                (declaredBackupID != routersOldBackupID))
        {
            if (intf->getNeighbor(i)->getState() >= Ospfv3Neighbor::TWOWAY_STATE) {
                intf->getNeighbor(i)->processEvent(Ospfv3Neighbor::IS_ADJACENCY_OK);
            }
        }
    }

    if (intf->getDesignatedID() != intf->getArea()->getInstance()->getProcess()->getRouterID()) {
        Ospfv3Neighbor* designated = intf->getNeighborById(intf->getDesignatedID());
        if (designated != nullptr)
            intf->setDesignatedIP(designated->getNeighborIP());
    }
    else
        intf->setDesignatedIP(intf->getInterfaceLLIP());

    if (intf->getBackupID() != intf->getArea()->getInstance()->getProcess()->getRouterID()) {
        Ospfv3Neighbor* backup = intf->getNeighborById(intf->getBackupID());
        if (backup != nullptr)
            intf->setBackupIP(backup->getNeighborIP());
    }
    else
        intf->setBackupIP(intf->getInterfaceLLIP());
}

} // namespace ospfv3
}//namespace inet

