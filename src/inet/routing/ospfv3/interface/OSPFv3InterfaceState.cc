#include "inet/routing/ospfv3/interface/OSPFv3InterfaceState.h"

#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateBackup.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDR.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDROther.h"

namespace inet{
void OSPFv3InterfaceState::changeState(OSPFv3Interface *interface, OSPFv3InterfaceState *newState, OSPFv3InterfaceState *currentState)
{
    OSPFv3Interface::OSPFv3InterfaceFAState oldState = currentState->getState();
    OSPFv3Interface::OSPFv3InterfaceFAState nextState = newState->getState();
    OSPFv3Interface::OSPFv3InterfaceType intfType = interface->getType();
    bool shouldRebuildRoutingTable = false;

    std::cout << "CHANGE STATE: " << interface->getArea()->getInstance()->getProcess()->getRouterID() << ", area " << interface->getArea()->getAreaID() << ", intf " << interface->getIntName() << ", curr st: " << currentState->getInterfaceStateString()<< ", new st: " << newState->getInterfaceStateString() << "\n";
    EV_DEBUG << "CHANGE STATE: " << interface->getArea()->getInstance()->getProcess()->getRouterID() << ", area " << interface->getArea()->getAreaID() << ", intf " << interface->getIntName() << ", curr st: " << currentState->getInterfaceStateString() << ", new st: " << newState->getInterfaceStateString() << "\n";



    interface->changeState(currentState, newState);

    //change of state -> new router LSA needs to be generated

    if ((oldState == OSPFv3Interface::INTERFACE_STATE_DOWN) ||
         (nextState == OSPFv3Interface::INTERFACE_STATE_DOWN) ||
         (oldState == OSPFv3Interface::INTERFACE_STATE_LOOPBACK) ||
         (nextState == OSPFv3Interface::INTERFACE_STATE_LOOPBACK) ||
         (oldState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
         (nextState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||

         ((intfType == OSPFv3Interface::POINTTOMULTIPOINT_TYPE) && ((oldState == OSPFv3Interface::INTERFACE_STATE_POINTTOPOINT) ||
           (nextState == OSPFv3Interface::INTERFACE_STATE_POINTTOPOINT))) ||

         (((intfType == OSPFv3Interface::BROADCAST_TYPE) ||
           (intfType == OSPFv3Interface::NBMA_TYPE)) && (/*(oldState == OSPFv3Interface::INTERFACE_STATE_WAITING) ||*/
           (nextState == OSPFv3Interface::INTERFACE_STATE_WAITING))))
    {

        LSAKeyType lsaKey;
        lsaKey.LSType = OSPFv3LSAFunctionCode::ROUTER_LSA;
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
                interface->getArea()->setSpfTreeRoot(routerLSA);
                interface->getArea()->floodLSA(newLSA);
            }
            delete newLSA;

        }
    }
    if (oldState == OSPFv3Interface::INTERFACE_STATE_WAITING)
    {
        RouterLSA* routerLSA;
        int routerLSAcount = interface->getArea()->getRouterLSACount();
        int i = 0;
        while (i < routerLSAcount)
        {
            routerLSA = interface->getArea()->getRouterLSA(i);
            const OSPFv3LSAHeader &header = routerLSA->getHeader();
            if (header.getAdvertisingRouter() == interface->getArea()->getInstance()->getProcess()->getRouterID() &&
                    interface->getType() == OSPFv3Interface::BROADCAST_TYPE) {
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
        RouterLSA* routerLsa = interface->getArea()->originateRouterLSA();
        if (routerLsa != nullptr)
        {
            if (interface->getArea()->installRouterLSA(routerLsa))
            {
                interface->getArea()->setSpfTreeRoot(routerLsa);
                interface->getArea()->floodLSA(routerLsa);

            }
        }
        if (interface->getType() == OSPFv3Interface::POINTTOPOINT_TYPE || (interface->getArea()->hasAnyPassiveInterface()))
        {
            OSPFv3IntraAreaPrefixLSA* prefLSA  = interface->getArea()->originateIntraAreaPrefixLSA();
            if (prefLSA != nullptr)
                if (interface->getArea()->installIntraAreaPrefixLSA(prefLSA))
                    interface->getArea()->floodLSA(prefLSA);

        }

        if(nextState == OSPFv3Interface::INTERFACE_STATE_BACKUP ||
           nextState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED ||
           nextState == OSPFv3Interface::INTERFACE_STATE_DROTHER) {
            interface->setTransitNetInt(true);//this interface is in broadcast network

            // if router has any interface as Passive, create separate Intra-Area-Prefix-LSA for these interfaces
            if (interface->getArea()->hasAnyPassiveInterface())
            {
                OSPFv3IntraAreaPrefixLSA* prefLSA  = interface->getArea()->originateIntraAreaPrefixLSA();
                if (prefLSA != nullptr)
                    if (interface->getArea()->installIntraAreaPrefixLSA(prefLSA))
                        interface->getArea()->floodLSA(prefLSA);
            }
        }
        shouldRebuildRoutingTable = true;
    }

    if (nextState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) {
        NetworkLSA* newLSA = interface->getArea()->originateNetworkLSA(interface);
        if (newLSA != nullptr) {
            shouldRebuildRoutingTable |= interface->getArea()->installNetworkLSA(newLSA);
            if (shouldRebuildRoutingTable)
            {
                OSPFv3IntraAreaPrefixLSA* prefLSA = interface->getArea()->originateNetIntraAreaPrefixLSA(newLSA, interface, true);
                if (prefLSA != nullptr)
                {
                    interface->getArea()->installIntraAreaPrefixLSA(prefLSA);
                    InterfaceEntry* ie = interface->containingProcess->ift->getInterfaceById(interface->getInterfaceId());
                    Ipv6InterfaceData *ipv6int = ie->ipv6Data();
                    ipv6int->joinMulticastGroup(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
//                    ipv6int->assignAddress(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, false, 0, 0);

                    interface->getArea()->floodLSA(newLSA);
                    interface->getArea()->floodLSA(prefLSA);
                }
            }
        }
        else
        {
            NetworkLSA *networkLSA = interface->getArea()->findNetworkLSAByLSID(Ipv4Address(interface->getInterfaceId()));
            if (networkLSA != nullptr)
            {
                networkLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                interface->getArea()->floodLSA(networkLSA);
                networkLSA->incrementInstallTime();
            }
            // here I am in state when interface comes to DR state but there is no neighbor on this iface.
            // so new LSA type 9 need to be created
//            if (interface->getArea()->getInstance()->getAreaCount() > 1) //this is ABR
//            {
//                interface->getArea()->originateInterAreaPrefixLSA(prefLSA, interface->getArea(), false);
//            }
        }
        OSPFv3IntraAreaPrefixLSA* prefLSA = interface->getArea()->originateIntraAreaPrefixLSA();
        if (prefLSA != nullptr)
        {
            shouldRebuildRoutingTable |= interface->getArea()->installIntraAreaPrefixLSA(prefLSA);
            interface->getArea()->floodLSA(prefLSA);
        }
    }

    if (nextState ==  OSPFv3Interface::INTERFACE_STATE_BACKUP)
    {
        InterfaceEntry* ie = interface->containingProcess->ift->getInterfaceById(interface->getInterfaceId());
        Ipv6InterfaceData *ipv6int = ie->ipv6Data();
        ipv6int->joinMulticastGroup(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
    }

    if ((oldState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED )&& (nextState != OSPFv3Interface::INTERFACE_STATE_DESIGNATED)) {
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

    if ((oldState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED || oldState == OSPFv3Interface::INTERFACE_STATE_BACKUP) &&
        (nextState != OSPFv3Interface::INTERFACE_STATE_DESIGNATED || nextState != OSPFv3Interface::INTERFACE_STATE_BACKUP))
    {
        InterfaceEntry* ie = interface->containingProcess->ift->getInterfaceById(interface->getInterfaceId());
        Ipv6InterfaceData *ipv6int = ie->ipv6Data();
        ipv6int->leaveMulticastGroup(Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
    }
}

void OSPFv3InterfaceState::calculateDesignatedRouter(OSPFv3Interface *intf){
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

            OSPFv3Neighbor *neighbor = intf->getNeighbor(i);
            int neighborPriority = neighbor->getNeighborPriority();

            if (neighbor->getState() < OSPFv3Neighbor::TWOWAY_STATE) {
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
            OSPFv3Neighbor *neighbor = intf->getNeighbor(i);
            unsigned short neighborPriority = neighbor->getNeighborPriority();

            if (neighbor->getState() < OSPFv3Neighbor::TWOWAY_STATE) {
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
    for (int g = 0 ; g <  intf->getNeighborCount(); g++)
    {
        EV_DEBUG << "@" << g <<  "  - " <<  intf->getNeighbor(g)->getNeighborID() << " / " << intf->getNeighbor(g)->getNeighborInterfaceID()  << "\n";
    }

    OSPFv3Neighbor* DRneighbor = intf->getNeighborById(declaredDesignatedRouterID);
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
    bool wasOther = (intf->getState() == OSPFv3Interface::INTERFACE_STATE_DROTHER);
    bool wasWaiting = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter = (declaredBackupID == routerID);
    bool isDesignatedRouter = (declaredDesignatedRouterID == routerID);
    bool isOther = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }
        if (isOther) {
            changeState(intf, new OSPFv3InterfaceStateDROther, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new OSPFv3InterfaceStateDROther, this);
        }
        if (isDesignatedRouter) { // for case that link between routers was disconnected and Router become DR again
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }

    }
    if (wasOther) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new OSPFv3InterfaceStateDROther, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == OSPFv3Interface::NBMA_TYPE) &&
                ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
                        (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->getNeighbor(i)->getNeighborPriority() == 0) {
                intf->getNeighbor(i)->processEvent(OSPFv3Neighbor::START);
            }
        }
        if ((declaredDesignatedRouterID != routersOldDesignatedRouterID) ||
                (declaredBackupID != routersOldBackupID))
        {
            if (intf->getNeighbor(i)->getState() >= OSPFv3Neighbor::TWOWAY_STATE) {
                intf->getNeighbor(i)->processEvent(OSPFv3Neighbor::IS_ADJACENCY_OK);
            }
        }
    }

    if(intf->getDesignatedID() != intf->getArea()->getInstance()->getProcess()->getRouterID()) {
        OSPFv3Neighbor* designated = intf->getNeighborById(intf->getDesignatedID());
        if(designated != nullptr)
            intf->setDesignatedIP(designated->getNeighborIP());
    }
    else
        intf->setDesignatedIP(intf->getInterfaceLLIP());

    if(intf->getBackupID() != intf->getArea()->getInstance()->getProcess()->getRouterID()) {
        OSPFv3Neighbor* backup = intf->getNeighborById(intf->getBackupID());
        if(backup != nullptr)
            intf->setBackupIP(backup->getNeighborIP());
    }
    else
        intf->setBackupIP(intf->getInterfaceLLIP());
}

}//namespace inet
