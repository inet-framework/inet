#include "inet/routing/ospfv3/process/OSPFv3Area.h"

#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

namespace inet{

OSPFv3Area::OSPFv3Area(Ipv4Address areaID, OSPFv3Instance* parent, OSPFv3AreaType type)
{
    this->areaID=areaID;
    this->containingInstance=parent;
    this->externalRoutingCapability = true;
    this->areaType = type;
    this->spfTreeRoot = nullptr;

    WATCH_PTRVECTOR(this->interfaceList);
}

OSPFv3Area::~OSPFv3Area()
{
}

void OSPFv3Area::init()
{
    if (this->getInstance()->getAddressFamily() == IPV6INSTANCE) // if this instance is a part of IPv6 AF process, set v6 to true, otherwise, set v6 to false
        v6 = true;
    else
        v6 = false;

    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
    {
        (*it)->setInterfaceIndex(this->getInstance()->getUniqueId());
        this->interfaceByIndex[(*it)->getInterfaceIndex()]=(*it);
        (*it)->processEvent(OSPFv3Interface::INTERFACE_UP_EVENT);
    }

    OSPFv3IntraAreaPrefixLSA* prefixLsa = this->originateIntraAreaPrefixLSA();
    EV_DEBUG << "Creating InterAreaPrefixLSA from IntraAreaPrefixLSA\n";
    if (prefixLsa != nullptr)
        this->installIntraAreaPrefixLSA(prefixLsa);                 // INTRA !!!

    if((this->getAreaType() == OSPFv3AreaType::STUB) && (this->getInstance()->getAreaCount()>1))
        this->originateDefaultInterAreaPrefixLSA(this);
}

bool OSPFv3Area::hasInterface(std::string interfaceName)
{
    std::map<std::string, OSPFv3Interface*>::iterator interfaceIt = this->interfaceByName.find(interfaceName);
    if(interfaceIt == this->interfaceByName.end())
        return false;

    return true;
}//hasArea

OSPFv3Interface* OSPFv3Area::getInterfaceById(int id)
{
    std::map<int, OSPFv3Interface*>::iterator interfaceIt = this->interfaceById.find(id);
    if(interfaceIt == this->interfaceById.end())
        return nullptr;

    return interfaceIt->second;
}//getInterfaceById

OSPFv3Interface* OSPFv3Area::getNetworkLSAInterface(Ipv4Address id)
{

    for (auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
    {
        if (Ipv4Address((*it)->getInterfaceId()) == id)
            return (*it);
    }

    return nullptr;

}//getInterfaceById

OSPFv3Interface* OSPFv3Area::getInterfaceByIndex(int id)
{
    std::map<int, OSPFv3Interface*>::iterator interfaceIt = this->interfaceByIndex.find(id);
    if(interfaceIt == this->interfaceByIndex.end())
        return nullptr;

    return interfaceIt->second;
}//getInterfaceByIndex

void OSPFv3Area::addInterface(OSPFv3Interface* newInterface)
{
    this->interfaceList.push_back(newInterface);
    this->interfaceByName[newInterface->getIntName()]=newInterface;
    this->interfaceById[newInterface->getInterfaceId()]=newInterface;
}//addArea

OSPFv3Interface* OSPFv3Area::findVirtualLink(Ipv4Address routerID)
{
    int interfaceNum = this->interfaceList.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((interfaceList.at(i)->getType() == OSPFv3Interface::VIRTUAL_TYPE) &&
            (interfaceList.at(i)->getNeighborById(routerID) != nullptr))
        {
            return interfaceList.at(i);
        }
    }
    return nullptr;
}

OSPFv3Interface* OSPFv3Area::getInterfaceByIndex(Ipv4Address LinkStateID)
{
    int interfaceNum = interfaceList.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((interfaceList[i]->getType() != OSPFv3Interface::VIRTUAL_TYPE) &&
            (Ipv4Address(interfaceList[i]->getInterfaceIndex()) == LinkStateID ))
        {
            return interfaceList[i];
        }
    }
    return nullptr;
}

const OSPFv3LSAHeader* OSPFv3Area::findLSA(LSAKeyType lsaKey)
{
    switch (lsaKey.LSType) {
    case ROUTER_LSA: {
        RouterLSA* lsa = this->getRouterLSAbyKey(lsaKey);
        if(lsa == nullptr) {
            return nullptr;
        }
        else {
            const OSPFv3LSAHeader* lsaHeader = &(lsa->getHeader());
            return lsaHeader;
        }
    }
    break;
    case NETWORK_LSA: {
        NetworkLSA* lsa = this->getNetworkLSAbyKey(lsaKey);
                if(lsa == nullptr) {
                    return nullptr;
                }
                else {
                    const OSPFv3LSAHeader* lsaHeader = &(lsa->getHeader());
                    return lsaHeader;
                }
    }
    break;
//    case INTER_AREA_PREFIX_LSA;
//    case INTER_AREA_ROUTER_LSA;
//    case LINK_LSA;
//    case AS_EXTERNAL_LSA

    default:
        //ASSERT(false);
        break;
    }
    return nullptr;
}

Ipv4Address OSPFv3Area::getNewRouterLinkStateID()
{
    Ipv4Address currIP = this->routerLsID;
    int newIP = currIP.getInt()+1;
    this->routerLsID = Ipv4Address(newIP);
    return currIP;
}

void OSPFv3Area::ageDatabase()
{
    long lsaCount = this->getRouterLSACount();
    bool shouldRebuildRoutingTable = false;
    long i;

    // ROUTER-LSA
    for (i = 0; i < lsaCount; i++) {
        RouterLSA *lsa = routerLSAList[i];
        unsigned short lsAge = lsa->getHeader().getLsaAge();
        bool selfOriginated = (lsa->getHeader().getAdvertisingRouter() == this->getInstance()->getProcess()->getRouterID());
//        TODO unreachability is not managed, Should be on places where it is commented
//        bool unreachable = parentRouter->isDestinationUnreachable(lsa);

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeaderForUpdate().setLsaAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->validateLSChecksum()) {   // always return true
                    EV_ERROR << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->incrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
//            if (unreachable) {
//                lsa->getHeader().setLsaAge(MAX_AGE);
//                floodLSA(lsa);
//                lsa->incrementInstallTime();
//            }
//        else

            long sequenceNumber = lsa->getHeader().getLsaSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                RouterLSA *newLSA = originateRouterLSA();
                newLSA->getHeaderForUpdate().setLsaSequenceNumber(sequenceNumber + 1);
                shouldRebuildRoutingTable |= updateRouterLSA(lsa, newLSA);
                delete newLSA;

                floodLSA(lsa);
            }

        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
            floodLSA(lsa);
            lsa->incrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(OSPFv3Neighbor::EXCHANGE_STATE | OSPFv3Neighbor::LOADING_STATE))
            {
                if (!selfOriginated /*|| unreachable*/) {
                    routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    routerLSAList[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    RouterLSA *newLSA = originateRouterLSA();
                    long sequenceNumber = lsa->getHeader().getLsaSequenceNumber();

                    newLSA->getHeaderForUpdate().setLsaSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                    shouldRebuildRoutingTable |= updateRouterLSA(lsa, newLSA);
                    delete newLSA;

                    floodLSA(lsa);
                }
            }
        }
    }

    auto routerIt = routerLSAList.begin();
    while (routerIt != routerLSAList.end()) {
        if ((*routerIt) == nullptr) {
            routerIt = routerLSAList.erase(routerIt);
        }
        else {
            routerIt++;
        }
    }


    // NETWORK-LSA
    lsaCount = networkLSAList.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short lsAge = networkLSAList[i]->getHeader().getLsaAge();
//        bool unreachable = parentRouter->isDestinationUnreachable(networkLSAs[i]);
        NetworkLSA *lsa = networkLSAList[i];
        OSPFv3Interface *localIntf = nullptr;
        if (lsa->getHeader().getAdvertisingRouter() == this->getInstance()->getProcess()->getRouterID()){
            localIntf = getNetworkLSAInterface(lsa->getHeader().getLinkStateID());
        }
        bool selfOriginated = false;

        if ((localIntf != nullptr) &&
            (localIntf->getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) &&
            (localIntf->getNeighborCount() > 0) &&
            (localIntf->hasAnyNeighborInState(OSPFv3Neighbor::FULL_STATE)))
        {
            selfOriginated = true;
        }

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1))))
        {
            lsa->getHeaderForUpdate().setLsaAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {     // always TRUE
                if (!lsa->validateLSChecksum()) {
                    EV_ERROR << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->incrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
//            TODO
//            if (unreachable) {
//                lsa->getHeader().setLsaAge(MAX_AGE);
//                floodLSA(lsa);
//                lsa->incrementInstallTime();
//            }
//            else {
            long sequenceNumber = lsa->getHeader().getLsaSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
                floodLSA(lsa);
                lsa->incrementInstallTime();
            }
            else {
                NetworkLSA *newLSA = originateNetworkLSA(localIntf);

                if (newLSA != nullptr) {
                    newLSA->getHeaderForUpdate().setLsaSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= updateNetworkLSA(lsa,newLSA);
                    delete newLSA;
                }
                else {    // no neighbors on the network -> old NetworkLSA must be flushed
                    lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
                    lsa->incrementInstallTime();
                }

                floodLSA(lsa);
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
            floodLSA(lsa);
            lsa->incrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                !hasAnyNeighborInStates(OSPFv3Neighbor::EXCHANGE_STATE | OSPFv3Neighbor::LOADING_STATE)) // Final state for Neighbor is FULL_STATE
            {
                if (!selfOriginated /*|| unreachable*/) {
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    networkLSAList[i] = nullptr;
                    shouldRebuildRoutingTable = true;
                }
                else {
                    NetworkLSA *newLSA = originateNetworkLSA(localIntf);
                    long sequenceNumber = lsa->getHeader().getLsaSequenceNumber();

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsaSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= updateNetworkLSA(lsa,newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else {    // no neighbors on the network -> old NetworkLSA must be deleted
                        delete networkLSAList[i];
                    }
                }
            }
        }
    }

    auto networkIt = networkLSAList.begin();
    while (networkIt != networkLSAList.end()) {
        if ((*networkIt) == nullptr) {
            networkIt = networkLSAList.erase(networkIt);
        }
        else {
            networkIt++;
        }
    }

    // INTRA-AREA-PREFIX-LSA
    lsaCount = intraAreaPrefixLSAList.size();
    for (i = 0; i < lsaCount; i++)
    {
        unsigned short lsAge = intraAreaPrefixLSAList[i]->getHeader().getLsaAge();
        //        bool unreachable = parentRouter->isDestinationUnreachable(networkLSAs[i]);
        IntraAreaPrefixLSA *lsa = intraAreaPrefixLSAList[i];
        OSPFv3Interface *localIntf = nullptr;
        if (lsa->getHeader().getAdvertisingRouter() == this->getInstance()->getProcess()->getRouterID()){
           localIntf = getNetworkLSAInterface(lsa->getReferencedLSID());
        }
        bool selfOriginated = false;

        if ((localIntf != nullptr) &&
           (localIntf->getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) &&
           (localIntf->getNeighborCount() > 0) &&
           (localIntf->hasAnyNeighborInState(OSPFv3Neighbor::FULL_STATE)))
        {
           selfOriginated = true;
        }

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
           lsa->getHeaderForUpdate().setLsaAge(lsAge + 1);
           if ((lsAge + 1) % CHECK_AGE == 0) {     // always TRUE
               if (!lsa->validateLSChecksum()) {
                   EV_ERROR << "Invalid LS checksum. Memory error detected!\n";
               }
           }
           lsa->incrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
        //            TODO
        //            if (unreachable) {
        //                lsa->getHeader().setLsaAge(MAX_AGE);
        //                floodLSA(lsa);
        //                lsa->incrementInstallTime();
        //            }
        //            else {
           long sequenceNumber = lsa->getHeader().getLsaSequenceNumber();
           if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
               lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
               floodLSA(lsa);
               lsa->incrementInstallTime();
           }
           else {

               IntraAreaPrefixLSA *newLSA = nullptr;
               // If this is DR, find Network LSA from which make new IntraAreaPrefix LSA
                if (localIntf != nullptr &&
                    localIntf->getType() == OSPFv3Interface::BROADCAST_TYPE)
                    {
                    NetworkLSA *netLSA = findNetworkLSA(localIntf->getInterfaceId(), this->getInstance()->getProcess()->getRouterID());
                    newLSA = originateNetIntraAreaPrefixLSA(netLSA, localIntf, false);
                }
                else // OSPFv3Interface::ROUTER_TYPE
                {
                    newLSA = originateIntraAreaPrefixLSA();
                }

                if (newLSA != nullptr) {
//                    newLSA->getHeaderForUpdate().setLsaSequenceNumber(sequenceNumber + 1);
                    shouldRebuildRoutingTable |= updateIntraAreaPrefixLSA(lsa,newLSA);
                    if (lsa != newLSA)
                        delete newLSA;
                }
                else {    // no neighbors on the network -> old NetworkLSA must be flushed
                    lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
                    lsa->incrementInstallTime();
                }
               floodLSA(lsa);
           }
        }

        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
           lsa->getHeaderForUpdate().setLsaAge(MAX_AGE);
           floodLSA(lsa);
           lsa->incrementInstallTime();
        }

        if (lsAge == MAX_AGE)
        {
            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) &&
                    !hasAnyNeighborInStates(OSPFv3Neighbor::EXCHANGE_STATE | OSPFv3Neighbor::LOADING_STATE))
            {
                if (!selfOriginated /*|| unreachable*/)
                {
                   if (this->getInstance()->getAreaCount() > 1) //this is ABR
                   {
                       //ivalidate all INTER LSA in ohter areas, which have been made from this INTRA LSA
                       for (int ar = 0; ar < this->getInstance()->getAreaCount(); ar++ )
                       {
                           OSPFv3Area* area = this->getInstance()->getArea(ar);
                           if (area->getAreaID() == this->getAreaID())
                               continue;
                           for (int prefs = 0; prefs < lsa->getPrefixesArraySize(); prefs++)
                           {
                               InterAreaPrefixLSA* interLSA  = area->findInterAreaPrefixLSAbyAddress(lsa->getPrefixes(prefs).addressPrefix, lsa->getPrefixes(prefs).prefixLen);
                               if (interLSA != nullptr)
                               {
                                   interLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
                                   area->floodLSA(interLSA);
                               }
                           }
                       }
                   }

                   intraAreaPrefixLSAByID.erase(lsa->getHeader().getLinkStateID());
                   delete lsa;
                   intraAreaPrefixLSAList[i] = nullptr;
                   shouldRebuildRoutingTable = true;
                }
                else
                {
                    IntraAreaPrefixLSA *newLSA = nullptr;
                    if (localIntf != nullptr &&
                    localIntf->getType() == OSPFv3Interface::BROADCAST_TYPE)
                    {
                        NetworkLSA *netLSA = findNetworkLSA(localIntf->getInterfaceId(), this->getInstance()->getProcess()->getRouterID());
                        newLSA = originateNetIntraAreaPrefixLSA(netLSA, localIntf, false);
                    }
                    else // OSPFv3Interface::ROUTER_TYPE
                    {
                        newLSA = originateIntraAreaPrefixLSA();
                    }

                    long sequenceNumber = lsa->getHeader().getLsaSequenceNumber();

                    if (newLSA != nullptr) {
                        newLSA->getHeaderForUpdate().setLsaSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        shouldRebuildRoutingTable |= updateIntraAreaPrefixLSA(lsa,newLSA);
                        delete newLSA;

                        floodLSA(lsa);
                    }
                    else
                    {    // no neighbors on the network -> old NetworkLSA must be flushed
                        delete intraAreaPrefixLSAList[i];
                    }
                }
            }
        }
    }



    auto intraArIt = intraAreaPrefixLSAList.begin();
    while (intraArIt != intraAreaPrefixLSAList.end()) {
        if ((*intraArIt) == nullptr) {
            intraArIt = intraAreaPrefixLSAList.erase(intraArIt);
        }
        else {
            intraArIt++;
        }
    }


    // INTER-AREA-PREFIX-LSA
    lsaCount = interAreaPrefixLSAList.size();
    for (i = 0; i < lsaCount; i++)
    {
        unsigned short lsaAge = interAreaPrefixLSAList[i]->getHeader().getLsaAge();
        bool selfOriginated = (interAreaPrefixLSAList[i]->getHeader().getAdvertisingRouter() == this->getInstance()->getProcess()->getRouterID());
        //bool unreachable = this->getInstance()->getProcess()->isDestinationUnreachable(interAreaPrefixLSAList[i]);
        InterAreaPrefixLSA *lsa = interAreaPrefixLSAList[i];

        if ((selfOriginated && (lsaAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsaAge < (MAX_AGE - 1)))) {
            lsa->getHeaderForUpdate().setLsaAge(lsaAge + 1);
            if ((lsaAge + 1) % CHECK_AGE == 0) {
                if (!lsa->validateLSChecksum()) {
                    EV_ERROR << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->incrementInstallTime();
        }
        // FIXME: refresh for inter-Area-Prefix-LSA is not working.
        //          Router need to refresh LSA so it need to go into other Area and from there make another LSA type 3 and flood him to this area

        if (lsaAge == MAX_AGE)
        {

            LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

            if (!isOnAnyRetransmissionList(lsaKey) )
            {
                if(!hasAnyNeighborInStates(OSPFv3Neighbor::EXCHANGE_STATE | OSPFv3Neighbor::LOADING_STATE))
                {
                    if (!selfOriginated /*|| unreachable*/) {
                        interAreaPrefixLSAByID.erase(lsa->getHeader().getLinkStateID());
                        delete lsa;
                        interAreaPrefixLSAList[i] = nullptr;
                        shouldRebuildRoutingTable = true;
                    }
                    else if (this->getInstance()->getAreaCount() > 1)//if its self-originate, find corresponding INTRA LSA
                    {
                        for(int ar = 0; ar < this->getInstance()->getAreaCount(); ar++)
                        {
                            OSPFv3Area* area = this->getInstance()->getArea(ar);
                            if(area->getAreaID() == this->getAreaID())
                                continue;

                            IntraAreaPrefixLSA* iapLSA = nullptr;
                            iapLSA = area->findIntraAreaPrefixByAddress(lsa->getPrefix(), lsa->getPrefixLen());
                            if (iapLSA != nullptr) // corresponding LSA has been found
                            {
                                if (iapLSA->getHeader().getLsaAge() != MAX_AGE)
                                {
                                    area->originateInterAreaPrefixLSA(iapLSA, area, true);
                                }
                                else
                                {
                                    interAreaPrefixLSAByID.erase(lsa->getHeader().getLinkStateID());
                                    delete lsa;
                                    interAreaPrefixLSAList[i] = nullptr;
                                    shouldRebuildRoutingTable = true;
                                }

                            }
                            else // corresponding LSA has NOT been found
                            {
                                // search then for correspoding INTER LSA with same prefix
                                InterAreaPrefixLSA* interLSA = area->findInterAreaPrefixLSAbyAddress(lsa->getPrefix(), lsa->getPrefixLen());
                                if (interLSA != nullptr && interLSA->getHeader().getLsaAge() != MAX_AGE)
                                {
                                    area->originateInterAreaPrefixLSA(interLSA, area);
                                }
                                else
                                {
                                    interAreaPrefixLSAByID.erase(lsa->getHeader().getLinkStateID());
                                    delete lsa;
                                    interAreaPrefixLSAList[i] = nullptr;
                                    shouldRebuildRoutingTable = true;
                                }

                            }
                        }


                    }
                }
            }
        }
//        if (crNew)
//        {
//            delete lsa;
//            interAreaPrefixLSAList[i] = nullptr;
//        }
    }

    auto interIt = interAreaPrefixLSAList.begin();
    while (interIt != interAreaPrefixLSAList.end()) {
        if ((*interIt) == nullptr) {
            interIt = interAreaPrefixLSAList.erase(interIt);
        }
        else {
            interIt++;
        }
    }



   for (int x = 0; x < interfaceList.size(); x++)
   {
       shouldRebuildRoutingTable |= interfaceList[x]->ageDatabase();
   }

    long interfaceCount = interfaceList.size();
    for (long m = 0; m < interfaceCount; m++) {
        interfaceList[m]->ageTransmittedLSALists();
    }

    if (shouldRebuildRoutingTable) {
        getInstance()->getProcess()->rebuildRoutingTable();
    }
    //TODO: add aging for missing LSAs
}

////------------------------------------- Router LSA --------------------------------------//
///*  Into any given OSPF area, a router will originate several LSAs.
//    Each router originates a router-LSA.  If the router is also the
//    Designated Router for any of the area's networks, it will
//    originate network-LSAs for those networks.
//
//    Area border routers originate a single summary-LSA for each
//    known inter-area destination.  AS boundary routers originate a
//    single AS-external-LSA for each known AS external destination.*/
RouterLSA* OSPFv3Area::originateRouterLSA()
{
    EV_DEBUG << "Originating RouterLSA (Router-LSA)\n";
    RouterLSA *routerLSA = new RouterLSA;
    OSPFv3LSAHeader& lsaHeader = routerLSA->getHeaderForUpdate();
    long interfaceCount = this->interfaceList.size();
    OSPFv3Options lsOptions;
    memset(&lsOptions, 0, sizeof(OSPFv3Options));

    //First set the LSA Header
    lsaHeader.setLsaAge(0);
    //The LSA Type is 0x2001
    lsaHeader.setLsaType(ROUTER_LSA);
    lsaHeader.setLinkStateID(this->getInstance()->getProcess()->getRouterID()); //TODO this depend on number of originated Router-LSA by this process. For now, there is always only one Router-LSA from one process
    lsaHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    lsaHeader.setLsaSequenceNumber(this->getCurrentRouterSequence());

    if(this->getInstance()->getAreaCount()>1)
        routerLSA->setBBit(true);
    //TODO - set options

    for(int i=0; i<interfaceCount; i++)
    {
        OSPFv3Interface* intf = this->interfaceList.at(i);

        if (intf->getState() == OSPFv3Interface::INTERFACE_STATE_DOWN ||
//                !intf->hasAnyNeighborInState(OSPFv3Neighbor::INIT_STATE
                intf->hasAnyNeighborInState(OSPFv3Neighbor::ATTEMPT_STATE) ||
                intf->hasAnyNeighborInState(OSPFv3Neighbor::DOWN_STATE)
                        ) {
            continue;
        }

        OSPFv3RouterLSABody routerLSABody;
        memset(&routerLSABody, 0, sizeof(OSPFv3RouterLSABody)); //FIXME : this sometimes makes first 'router' in router-LSA msgs not valid.

        switch(intf->getType())
        {
            case OSPFv3Interface::POINTTOPOINT_TYPE:
            {
                for (int nei = 0; nei < intf->getNeighborCount(); nei++)
                {

                    OSPFv3Neighbor *neighbor =  intf->getNeighbor(nei);
                    EV_DEBUG << "neighbor state = " << neighbor->getState() << "\n";

                    if ((neighbor != nullptr) && (neighbor->getState() == OSPFv3Neighbor::FULL_STATE))
                    {
                        routerLSABody.type=POINT_TO_POINT;

                        routerLSABody.interfaceID = intf->getInterfaceId();
                        routerLSABody.metric = METRIC;

                        routerLSABody.neighborInterfaceID = neighbor->getNeighborInterfaceID();
                        routerLSABody.neighborRouterID = neighbor->getNeighborID();

                        routerLSA->setRoutersArraySize(i+1);
                        routerLSA->setRouters(i, routerLSABody);
                    }

                }
            }
                break;

            case OSPFv3Interface::BROADCAST_TYPE:
            {
                routerLSABody.type=TRANSIT_NETWORK;
                OSPFv3Neighbor *DRouter =intf->getNeighborById(intf->getDesignatedID());

                if ( ((DRouter != nullptr) && (DRouter->getState() == OSPFv3Neighbor::FULL_STATE)) ||
                        ((intf->getDesignatedID() == this->getInstance()->getProcess()->getRouterID()) &&
                                intf->getNeighborCount() > 0)
                         )
                {
                    routerLSABody.interfaceID = intf->getInterfaceId();      // id of interface
                    routerLSABody.metric = METRIC;

                    routerLSABody.neighborInterfaceID = intf->getDesignatedIntID();
                    routerLSABody.neighborRouterID = intf->getDesignatedID();

                    routerLSA->setRoutersArraySize(i+1);
                    routerLSA->setRouters(i, routerLSABody);
                }
            }
                break;

            case OSPFv3Interface::VIRTUAL_TYPE:
                routerLSABody.type=VIRTUAL_LINK;
                break;
            case OSPFv3Interface::NBMA_TYPE:
            case OSPFv3Interface::POINTTOMULTIPOINT_TYPE:
                EV_DEBUG << "NBMA and P2MP for interface type is not in originate Router-LSA ymplemented yet\n";
            case OSPFv3Interface::UNKNOWN_TYPE:
            default:
                break;


        }
    }

//    RouterLSA *oldLSA = routerLSAAlreadyExists(routerLSA);
//    if (oldLSA != nullptr)
//    {
//        delete (routerLSA);
//        return oldLSA;
//    }

    this->incrementRouterSequence();
    return routerLSA;
}//originateRouterLSA

RouterLSA* OSPFv3Area::routerLSAAlreadyExists(RouterLSA* newLsa)
{
    for (auto it= this->routerLSAList.begin() ;it!=this->routerLSAList.end(); it++)
    {
        if ((*it)->getHeader().getAdvertisingRouter() == newLsa->getHeader().getAdvertisingRouter() &&
                (*it)->getHeader().getLinkStateID() == newLsa->getHeader().getLinkStateID() &&
                (*it)->getHeader().getLsaAge() != MAX_AGE &&
                (*it)->getHeader().getLsaAge() != MAX_AGE)
        {
           if ((*it)->getRoutersArraySize() == newLsa->getRoutersArraySize())
           {
               bool same = false;
               for (int x = 0; x < newLsa->getRoutersArraySize(); x++)
               {
                   if ((*it)->getRouters(x).interfaceID == newLsa->getRouters(x).interfaceID &&
                           (*it)->getRouters(x).metric == newLsa->getRouters(x).metric &&
                           (*it)->getRouters(x).neighborInterfaceID == newLsa->getRouters(x).neighborInterfaceID &&
                           (*it)->getRouters(x).neighborRouterID == newLsa->getRouters(x).neighborRouterID &&
                           (*it)->getRouters(x).type == newLsa->getRouters(x).type )
                   {
                       same = true;
                   }
                   else
                   {
                       same = false;
                       break;
                   }
               }
               if (same)
                   return (*it);
           }
       }
    }
    return nullptr;
}


RouterLSA* OSPFv3Area::getRouterLSAbyKey(LSAKeyType LSAKey)
{
    for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
    {
        if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID)
            return (*it);
    }

    return nullptr;
}

//add LSA message into list of all router-LSA for this area
bool OSPFv3Area::installRouterLSA(const OSPFv3RouterLSA *lsaC)
{
    auto lsa = lsaC->dup(); // make editable copy of lsa
    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    RouterLSA* lsaInDatabase = (RouterLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        return this->updateRouterLSA(lsaInDatabase, lsa);
    }
    else {
        RouterLSA* lsaCopy = new RouterLSA(*lsa);
        EV_DEBUG << "RouterLSA was added to routerLSAList\n";
        this->routerLSAList.push_back(lsaCopy);
        return true;
    }
}//installRouterLSA


bool OSPFv3Area::updateRouterLSA(RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa)
{
    bool different = routerLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->resetInstallTime();
//    currentLsa->getHeaderForUpdate().setLsaAge(0);//reset the age
    if (different) {
        return true;
    }
    else {
        return false;
    }
}//updateRouterLSA


bool OSPFv3Area::routerLSADiffersFrom(OSPFv3RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getNtBit() != newLsa->getNtBit()) ||
                         (currentLsa->getEBit() != newLsa->getEBit()) ||
                         (currentLsa->getBBit() != newLsa->getBBit()) ||
                         (currentLsa->getVBit() != newLsa->getVBit()) ||
                         (currentLsa->getXBit() != newLsa->getXBit()) ||
                         (currentLsa->getRoutersArraySize() != newLsa->getRoutersArraySize()));

        if (!differentBody) {
            unsigned int routersCount = currentLsa->getRoutersArraySize();
            for (unsigned int i = 0; i < routersCount; i++) {
                auto thisRouter = currentLsa->getRouters(i);
                auto lsaRouter = newLsa->getRouters(i);
                bool differentLink = ((thisRouter.type != lsaRouter.type) ||
                                      (thisRouter.metric != lsaRouter.metric) ||
                                      (thisRouter.interfaceID != lsaRouter.interfaceID) ||
                                      (thisRouter.neighborInterfaceID != lsaRouter.neighborInterfaceID) ||
                                      (thisRouter.neighborRouterID != lsaRouter.neighborRouterID));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}//routerLSADiffersFrom

void OSPFv3Area::deleteRouterLSA(int index) {
    RouterLSA *delRouter = this->routerLSAList.at(index);
    const OSPFv3LSAHeader &routerHeader = delRouter->getHeader();

    int prefixCount = this->intraAreaPrefixLSAList.size();
    for(int i=0; i<prefixCount; i++) {
       OSPFv3IntraAreaPrefixLSA* lsa = this->intraAreaPrefixLSAList.at(i);

       // odtrani dane LSA aj z intraAreaPrefixLSAList
       if (lsa->getReferencedAdvRtr() == routerHeader.getAdvertisingRouter() &&
               lsa->getReferencedLSID() == routerHeader.getLinkStateID() &&
               lsa->getReferencedLSType() == ROUTER_LSA) {
           IntraAreaPrefixLSA* prefLSA = this->intraAreaPrefixLSAList[i];
           prefLSA->getHeaderForUpdate().setLsaAge(MAX_AGE);
           this->floodLSA(prefLSA);
           this->intraAreaPrefixLSAList.erase(this->intraAreaPrefixLSAList.begin()+i);
           EV_DEBUG << "Deleting old Router-LSA, also deleting appropriate Intra-Area-Prefix-LSA\n";
           break;
       }
    }

    this->routerLSAList.erase(this->routerLSAList.begin()+index);
}

bool OSPFv3Area::floodLSA(const OSPFv3LSA* lsa, OSPFv3Interface* interface, OSPFv3Neighbor* neighbor)
{
    EV_DEBUG << "Flooding from Area to all interfaces\n";
    std::cout << this->getInstance()->getProcess()->getRouterID() << " - FLOOD LSA AREA!!" << endl;
    bool floodedBackOut = false;
    long interfaceCount = this->interfaceList.size();

    for (long i = 0; i < interfaceCount; i++) {
        if (interfaceList.at(i)->floodLSA(lsa, interface, neighbor)) {
            floodedBackOut = true;
        }
    }
    return floodedBackOut;
}

bool OSPFv3Area::hasAnyNeighborInStates(int states) const
{
    long interfaceCount = this->interfaceList.size();
    for (long i = 0; i < interfaceCount; i++)
    {
        if (interfaceList.at(i)->hasAnyNeighborInState(states))
            return true;
    }
    return false;
}

bool OSPFv3Area::hasAnyPassiveInterface() const
{
    long interfaceCount = this->interfaceList.size();
    for (long i = 0; i < interfaceCount; i++)
    {
        if (interfaceList.at(i)->isInterfacePassive())
            return true;
    }
    return false;
}

void OSPFv3Area::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{
    long interfaceCount = this->interfaceList.size();
    for (long i = 0; i < interfaceCount; i++) {
        this->interfaceList.at(i)->removeFromAllRetransmissionLists(lsaKey);
    }
}

bool OSPFv3Area::isOnAnyRetransmissionList(LSAKeyType lsaKey) const
{
    long interfaceCount = this->interfaceList.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (interfaceList.at(i)->isOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

RouterLSA *OSPFv3Area::findRouterLSA(Ipv4Address routerID)
{
    for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
    {
        if( (*it)->getHeader().getAdvertisingRouter() == routerID )
        {
            return (*it);
        }
    }
    return nullptr;
}

RouterLSA *OSPFv3Area::findRouterLSAByID(Ipv4Address linkStateID)
{
    for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++ )
    {
        if( (*it)->getHeader().getLinkStateID() == linkStateID)
        {
            return (*it);
        }
    }
    return nullptr;
}

////------------------------------------- Network LSA --------------------------------------//
NetworkLSA* OSPFv3Area::originateNetworkLSA(OSPFv3Interface* interface)
{
    if (interface->hasAnyNeighborInState(OSPFv3Neighbor::FULL_STATE))
    {
        NetworkLSA* networkLsa = new NetworkLSA();
        OSPFv3LSAHeader& lsaHeader = networkLsa->getHeaderForUpdate();
        OSPFv3Options lsOptions;
        memset(&lsOptions, 0, sizeof(OSPFv3Options));

        //First set the LSA Header
        lsaHeader.setLsaAge(0);
        //The LSA Type is 0x2002
        lsaHeader.setLsaType(NETWORK_LSA);
        lsaHeader.setLinkStateID(Ipv4Address(interface->getInterfaceId()));
        lsaHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
        lsaHeader.setLsaSequenceNumber(this->getCurrentNetworkSequence());
        this->incrementNetworkSequence();
        uint16_t packetLength = OSPFV3_LSA_HEADER_LENGTH + 4; // 4 for options field

        // body
        networkLsa->setOspfOptions(lsOptions);
        int attachedCount = interface->getNeighborCount();//+1 for this router
        if(attachedCount >= 1){
            networkLsa->setAttachedRouterArraySize(attachedCount+1);
            for(int i=0; i<attachedCount; i++){
                OSPFv3Neighbor* neighbor = interface->getNeighbor(i);
                networkLsa->setAttachedRouter(i, neighbor->getNeighborID());
                packetLength+=4;
            }
            networkLsa->setAttachedRouter(attachedCount, this->getInstance()->getProcess()->getRouterID());
            packetLength+=4;
        }

        lsaHeader.setLsaLength(packetLength);
        return networkLsa;
    }
    else {
        return nullptr;
    }
}//originateNetworkLSA

NetworkLSA* OSPFv3Area::getNetworkLSAbyKey(LSAKeyType LSAKey)
{
    for (auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++)
    {
        if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
            return (*it);
        }
    }
    return nullptr;
}//getRouterLSAByKey

bool OSPFv3Area::installNetworkLSA(const OSPFv3NetworkLSA *lsaC)
{
    auto lsa = lsaC->dup(); // make editable copy of lsa
    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    NetworkLSA* lsaInDatabase = (NetworkLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        return this->updateNetworkLSA(lsaInDatabase, lsa);
    }
    else {
        NetworkLSA* lsaCopy = new NetworkLSA(*lsa);
        this->networkLSAList.push_back(lsaCopy);
        return true;
    }
}//installNetworkLSA


bool OSPFv3Area::updateNetworkLSA(NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa)
{
    bool different = networkLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->resetInstallTime();
//    currentLsa->getHeaderForUpdate().setLsaAge(0);//reset the age
    if (different) {
        return true;
    }
    else {
        return false;
    }
}//updateNetworkLSA


bool OSPFv3Area::networkLSADiffersFrom(OSPFv3NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = (currentLsa->getOspfOptions() != newLsa->getOspfOptions());


        if (!differentBody) {
            unsigned int attachedCount = currentLsa->getAttachedRouterArraySize();
            for (unsigned int i = 0; i < attachedCount; i++) {
                bool differentLink = (currentLsa->getAttachedRouter(i)!=newLsa->getAttachedRouter(i));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}//networkLSADiffersFrom

Ipv4Address OSPFv3Area::getNewNetworkLinkStateID()
{
    Ipv4Address currIP = this->networkLsID;
    int newIP = currIP.getInt()+1;
    this->networkLsID = Ipv4Address(newIP);
    return currIP;
}//getNewNetworkLinkStateID


NetworkLSA *OSPFv3Area::findNetworkLSA(uint32_t intID, Ipv4Address routerID)
{
    for (auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++)
    {
        if( ((*it)->getHeader().getAdvertisingRouter() == routerID) && ((*it)->getHeader().getLinkStateID() == (Ipv4Address)intID) )
        {
            return (*it);
        }
    }
    return nullptr;
}

NetworkLSA *OSPFv3Area::findNetworkLSAByLSID(Ipv4Address linkStateID)
{
    for(auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++ )
    {
        if( (*it)->getHeader().getLinkStateID() == linkStateID)
        {
            return (*it);
        }
    }
    return nullptr;
}

//----------------------------------------- Inter-Area-Prefix LSA (LSA 3)------------------------------------------//
void OSPFv3Area::originateInterAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* lsa, OSPFv3Area* fromArea, bool checkDuplicate)
{
//    int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH;
//    int prefixCount = 0;

    // Separated Inter Area Prefix LSA is made for every prefix inside of Intra Area Prefix LSA
    for (int ref = 0; ref < lsa->getPrefixesArraySize(); ref++)
    {
        InterAreaPrefixLSA* newLsa = new InterAreaPrefixLSA();
        OSPFv3LSAHeader& newHeader = newLsa->getHeaderForUpdate();
        newHeader.setLsaAge(0);
        newHeader.setLsaType(INTER_AREA_PREFIX_LSA);
        newHeader.setLinkStateID(this->getInstance()->getNewInterAreaPrefixLinkStateID());
        newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
        newHeader.setLsaSequenceNumber(this->getCurrentInterAreaPrefixSequence());

        OSPFv3LSAPrefix& prefix = lsa->getPrefixesForUpdate(ref);
        newLsa->setDnBit(prefix.dnBit);
        newLsa->setLaBit(prefix.laBit);
        newLsa->setNuBit(prefix.nuBit);
        newLsa->setPBit(prefix.pBit);
        newLsa->setXBit(prefix.xBit);
        newLsa->setMetric(prefix.metric);
        newLsa->setPrefixLen(prefix.prefixLen);
        newLsa->setPrefix(prefix.addressPrefix);

        int duplicateForArea = 0;
        for(int i = 0; i < this->getInstance()->getAreaCount(); i++)
        {
            OSPFv3Area* area = this->getInstance()->getArea(i);
            if(area->getAreaID() == fromArea->getAreaID())
                continue;


            if (checkDuplicate)
            {
                InterAreaPrefixLSA* lsaDuplicate = area->InterAreaPrefixLSAAlreadyExists(newLsa);
                if (lsaDuplicate != nullptr && lsaDuplicate->getHeader().getLsaAge() != MAX_AGE) // LSA like this already exist
                {
                    duplicateForArea++;
                }
                else
                {
                    this->incrementInterAreaPrefixSequence();
                    if (area->installInterAreaPrefixLSA(newLsa))
                        area->floodLSA(newLsa);
//                    newLsa->getHeaderForUpdate().setLinkStateID(lsaDuplicate->getHeader().getLinkStateID());
//                    if (area->installInterAreaPrefixLSA(newLsa))
//                        area->floodLSA(newLsa);
                }
            }
            else
            {
                if (area->installInterAreaPrefixLSA(newLsa))
                        area->floodLSA(newLsa);
            }

        }
        if (duplicateForArea == this->getInstance()->getAreaCount()-1)
        {
            //new LSA was not installed anywhere, so subtract LinkStateID counter
            this->getInstance()->subtractInterAreaPrefixLinkStateID();
        }
    }
    //TODO - length!!!
}

void OSPFv3Area::originateInterAreaPrefixLSA(const OSPFv3LSA* prefLsa, OSPFv3Area* fromArea)
{
    LSAKeyType lsaKey;
    lsaKey.linkStateID = prefLsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = prefLsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = prefLsa->getHeader().getLsaType();

    //this check is mainly for dealing with shut downs
    OSPFv3LSA *lsaInDatabase = this->getInstance()->getProcess()->findLSA(lsaKey, fromArea->getAreaID(), fromArea->getInstance()->getInstanceID());

    for(int i = 0; i < this->getInstance()->getAreaCount(); i++)
    {
        OSPFv3Area* area = this->getInstance()->getArea(i);
        if(area->getAreaID() == fromArea->getAreaID())
            continue;

        const OSPFv3InterAreaPrefixLSA *lsa = check_and_cast<const OSPFv3InterAreaPrefixLSA *>(prefLsa);
        int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH;
        int prefixCount = 0;

        InterAreaPrefixLSA* newLsa = nullptr;

        if (lsaInDatabase != nullptr) //I've probably made already LSA type 3 from this prefLsa
            for (int inter = 0; inter < area->getInterAreaPrefixLSACount(); inter++)
            {
               InterAreaPrefixLSA* iterLsa = area->getInterAreaPrefixLSA(inter);
               if ((iterLsa->getHeader().getAdvertisingRouter() == this->getInstance()->getProcess()->getRouterID()) &&
                       (iterLsa->getPrefix() == lsa->getPrefix()) &&
                       (iterLsa->getPrefixLen() == lsa->getPrefixLen()))
               {
                   // this have already been processed.  So update old one and flood it away
                   newLsa = iterLsa;
                   break;
               }

            }
        if (newLsa == nullptr)
        {
            //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
            newLsa = new InterAreaPrefixLSA();
            OSPFv3LSAHeader& newHeader = newLsa->getHeaderForUpdate();
            newHeader.setLsaType(INTER_AREA_PREFIX_LSA);
            newHeader.setLinkStateID(area->getInstance()->getNewInterAreaPrefixLinkStateID());
            newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
            newHeader.setLsaSequenceNumber(area->getCurrentInterAreaPrefixSequence());
            area->incrementInterAreaPrefixSequence();
        }
        OSPFv3LSAHeader& newHeader2 = newLsa->getHeaderForUpdate();
        if (prefLsa->getHeader().getLsaAge() == MAX_AGE) //if this processed LSA is flooded for its invalidation
            newHeader2.setLsaAge(MAX_AGE);
        else
            newHeader2.setLsaAge(0);
        newLsa->setDnBit(lsa->getDnBit());
        newLsa->setLaBit(lsa->getLaBit());
        newLsa->setNuBit(lsa->getNuBit());
        newLsa->setPBit(lsa->getPBit());
        newLsa->setXBit(lsa->getXBit());
        newLsa->setMetric(lsa->getMetric());
        newLsa->setPrefixLen(lsa->getPrefixLen());
        newLsa->setPrefix(lsa->getPrefix());


//        area->installInterAreaPrefixLSA(newLsa);
        if (area->installInterAreaPrefixLSA(newLsa))
            area->floodLSA(newLsa);
        else if (lsaInDatabase)
            area->floodLSA(newLsa);

    }
    //TODO - length!!!
}

void OSPFv3Area::originateDefaultInterAreaPrefixLSA(OSPFv3Area* toArea)
{
    int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH;
    int prefixCount = 0;

    //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
    InterAreaPrefixLSA* newLsa = new InterAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeaderForUpdate();
    newHeader.setLsaAge(0);
    newHeader.setLsaType(INTER_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(toArea->getInstance()->getNewInterAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(toArea->getCurrentInterAreaPrefixSequence());
    toArea->incrementInterAreaPrefixSequence();

    newLsa->setDnBit(false);
    newLsa->setLaBit(false);
    newLsa->setNuBit(false);
    newLsa->setPBit(false);
    newLsa->setXBit(false);
    newLsa->setMetric(1);
    newLsa->setPrefixLen(0);

    if(this->getInstance()->getAddressFamily() == IPV4INSTANCE) {
        Ipv4Address defaultPref = Ipv4Address("0.0.0.0");
        newLsa->setPrefix(defaultPref);
    }
    else{
        Ipv6Address defaultPref = Ipv6Address("::");
        newLsa->setPrefix(defaultPref);
    }
    toArea->installInterAreaPrefixLSA(newLsa);
    //TODO - length!!!
}

bool OSPFv3Area::installInterAreaPrefixLSA(const OSPFv3InterAreaPrefixLSA* lsaC)
{
    auto lsa = lsaC->dup(); // make editable copy of lsa
    const OSPFv3LSAHeader &header = lsa->getHeader();
    cout << this->getInstance()->getProcess()->getRouterID() << "  -  " << this->getAreaID() << endl;

    EV_DEBUG << "\n\nInstalling Inter-Area-Prefix LSA:\nLink State ID: " << header.getLinkStateID() << "\nAdvertising router: " << header.getAdvertisingRouter();
    EV_DEBUG << "\nLS Seq Number: " << header.getLsaSequenceNumber() << endl;

    EV_DEBUG << "Prefix Address: " << lsa->getPrefix();
    EV_DEBUG << "\nPrefix Length: " << lsa->getPrefixLen();
    if(lsa->getDnBit())
        EV_DEBUG << "DN ";
    if(lsa->getLaBit())
        EV_DEBUG << "LA ";
    if(lsa->getNuBit())
        EV_DEBUG << "NU ";
    if(lsa->getPBit())
        EV_DEBUG << "P ";
    if(lsa->getXBit())
        EV_DEBUG << "X ";

    EV_DEBUG << ", Metric: " << lsa->getMetric() << "\n";

    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();
//    InterAreaPrefixLSA* lsaInDatabase = (InterAreaPrefixLSA*)this->getLSAbyKey(lsaKey);

    InterAreaPrefixLSA* lsaInDatabase = this->InterAreaPrefixLSAAlreadyExists(lsa);

    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        return this->updateInterAreaPrefixLSA(lsaInDatabase, lsa);
        EV_DEBUG << "Only updating\n";
    }
    else {
        InterAreaPrefixLSA* lsaCopy = new InterAreaPrefixLSA(*lsa);
        this->interAreaPrefixLSAList.push_back(lsaCopy);
        EV_DEBUG << "creating new one\n";
        return true;
    }
}

bool OSPFv3Area::updateInterAreaPrefixLSA(InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa)
{
    bool different = interAreaPrefixLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
//    currentLsa->getHeaderForUpdate().setLsaAge(0);//reset the age
    currentLsa->resetInstallTime();
    if (different) {
        return true;
    }
    else {
        return false;
    }
}

bool OSPFv3Area::interAreaPrefixLSADiffersFrom(OSPFv3InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getMetric() != newLsa->getMetric()) ||
                (currentLsa->getPrefix() != newLsa->getPrefix()) ||
                (currentLsa->getPrefixLen() != newLsa->getPrefixLen()) ||
                (currentLsa->getDnBit() != newLsa->getDnBit()) ||
                (currentLsa->getLaBit() != newLsa->getLaBit()) ||
                (currentLsa->getMetric() != newLsa->getMetric()) ||
                (currentLsa->getNuBit() != newLsa->getNuBit()) ||
                (currentLsa->getPBit() != newLsa->getPBit()) ||
                (currentLsa->getPrefixLen() != newLsa->getPrefixLen()) ||
                (currentLsa->getXBit() != newLsa->getXBit()));
    }

    return differentHeader || differentBody;
}

// return nullptr, if newLsa is not a duplicate
InterAreaPrefixLSA* OSPFv3Area::InterAreaPrefixLSAAlreadyExists(OSPFv3InterAreaPrefixLSA *newLsa)
{
    for (auto it= this->interAreaPrefixLSAList.begin(); it!=this->interAreaPrefixLSAList.end(); it++)
    {

        if ((*it)->getHeader().getAdvertisingRouter() == newLsa->getHeader().getAdvertisingRouter() &&
            (*it)->getPrefix() == newLsa->getPrefix() &&
            (*it)->getPrefixLen() == newLsa->getPrefixLen())
        {
            return (*it);
        }
    }
    return nullptr;
}

InterAreaPrefixLSA* OSPFv3Area::findInterAreaPrefixLSAbyAddress(const L3Address address, int prefixLen)
{
    for (auto it= this->interAreaPrefixLSAList.begin(); it!=this->interAreaPrefixLSAList.end(); it++)
        {
            if ((*it)->getPrefix() == address && (*it)->getPrefixLen() == prefixLen)
            {
                return (*it);
            }
        }
        return nullptr;
}


//----------------------------------------- Intra-Area-Prefix LSA (LSA 9) ------------------------------------------//
IntraAreaPrefixLSA* OSPFv3Area::originateIntraAreaPrefixLSA() //this is for non-BROADCAST links
{
    int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH;
    int prefixCount = 0;

    //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
    IntraAreaPrefixLSA* newLsa = new IntraAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeaderForUpdate();
    newHeader.setLsaAge(0);
    newHeader.setLsaType(INTRA_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(this->getNewIntraAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(this->getCurrentIntraAreaPrefixSequence());

    //for each Router LSA there is a corresponding Intra-Area-Prefix LSA
    for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
    {
        const OSPFv3LSAHeader &routerHeader = (*it)->getHeader();
        if(routerHeader.getAdvertisingRouter()!=this->getInstance()->getProcess()->getRouterID())
            continue;
        else {
            newLsa->setReferencedLSType(ROUTER_LSA);
            newLsa->setReferencedLSID(routerHeader.getLinkStateID());
            newLsa->setReferencedAdvRtr(routerHeader.getAdvertisingRouter());
        }
    }

    int currentPrefix = 1;
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
    {
        // if interface is not transit (not in state DR, BDR or DRother) or has no neighbour in FULL STATE than continue
        if((*it)->getTransitNetInt() == false || !(*it)->hasAnyNeighborInState(OSPFv3Neighbor::FULL_STATE))
        {
            InterfaceEntry *ie = this->getInstance()->getProcess()->ift->getInterfaceByName((*it)->getIntName().c_str());
            Ipv6InterfaceData* ipv6int = ie->ipv6Data();

            int numPrefixes;
            if(!v6) // is this LSA for IPv4 or Ipv6 AF ?
                numPrefixes = 1;
            else
            {
                numPrefixes = ipv6int->getNumAddresses();
            }

            for(int i=0; i<numPrefixes; i++) {
                if(this->getInstance()->getAddressFamily() == IPV4INSTANCE)
                {
                    Ipv4InterfaceData* ipv4Data = ie->ipv4Data();
                    Ipv4Address ipAdd = ipv4Data->getIPAddress();
                    OSPFv3LSAPrefix *prefix = new OSPFv3LSAPrefix();
                    prefix->prefixLen= ipv4Data->getNetmask().getNetmaskLength();
                    prefix->metric = METRIC;
                    prefix->addressPrefix=L3Address(ipAdd.getPrefix(prefix->prefixLen));
                    newLsa->setPrefixesArraySize(currentPrefix);
                    newLsa->setPrefixes(currentPrefix-1, *prefix);
                    prefixCount++;
                    currentPrefix++;
                }
                else
                {
                    Ipv6Address ipv6 = ipv6int->getAddress(i);
                    if(ipv6.isGlobal()) {//Only all the global prefixes belong to the Intra-Area-Prefix LSA
                        OSPFv3LSAPrefix *prefix = new OSPFv3LSAPrefix();
                        int rIndex = this->getInstance()->getProcess()->isInRoutingTable6(this->getInstance()->getProcess()->rt6, ipv6);
                        if (rIndex >= 0)
                            prefix->prefixLen = this->getInstance()->getProcess()->rt6->getRoute(rIndex)->getPrefixLength();
                        else
                            prefix->prefixLen = 64;
                        prefix->metric = METRIC;
                        prefix->addressPrefix=ipv6.getPrefix(prefix->prefixLen);

                        newLsa->setPrefixesArraySize(currentPrefix);
                        newLsa->setPrefixes(currentPrefix-1, *prefix);
                        prefixCount++;
                        currentPrefix++;
                    }
                }
            }
        }
    }

    //TODO - length!!!
    newLsa->setNumPrefixes(prefixCount);

    if (prefixCount == 0) //check if this LSA is not without prefixes
    {
        delete (newLsa);

        // there will be probably some old Intra-Area-Prefix LSAs, which need to be invalidated
        for (auto it= this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
        {
            if ((*it)->getHeader().getAdvertisingRouter() == this->getInstance()->getProcess()->getRouterID() &&
                    (*it)->getReferencedLSType() == ROUTER_LSA &&
                    (*it)->getHeader().getLsaAge() != MAX_AGE)
            {
              (*it)->getHeaderForUpdate().setLsaAge(MAX_AGE);
              this->floodLSA((*it));
            }
        }
        return nullptr;
    }
    // check if this LSA has not been already created
    IntraAreaPrefixLSA* prefixLsa = IntraAreaPrefixLSAAlreadyExists(newLsa);
    if (prefixLsa != nullptr)
    {
       this->subtractIntraAreaPrefixLinkStateID();
       delete (newLsa);
       return prefixLsa;
    }
    this->incrementIntraAreaPrefixSequence();
    return newLsa;
}//originateIntraAreaPrefixLSA

IntraAreaPrefixLSA* OSPFv3Area::originateNetIntraAreaPrefixLSA(NetworkLSA* networkLSA, OSPFv3Interface* interface, bool checkDuplicate)
{
    EV_DEBUG << "Originate New NETWORK INTRA AREA LSA\n";
    // get IPv6 data
    InterfaceEntry *ie = this->getInstance()->getProcess()->ift->getInterfaceByName(interface->getIntName().c_str());
    Ipv6InterfaceData* ipv6int = ie->ipv6Data();
    OSPFv3LSAHeader &header = networkLSA->getHeaderForUpdate();

    IntraAreaPrefixLSA* newLsa = new IntraAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeaderForUpdate();
    newHeader.setLsaAge(0);
    newHeader.setLsaType(INTRA_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(this->getNewIntraAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(this->getCurrentIntraAreaPrefixSequence());

    newLsa->setReferencedLSType(NETWORK_LSA);
    newLsa->setReferencedLSID(header.getLinkStateID());
    newLsa->setReferencedAdvRtr(header.getAdvertisingRouter());

    int numPrefixes;
    if(!v6) //if this is not IPV6INSTANCE
        numPrefixes = 1;
    else
    {
        numPrefixes = ipv6int->getNumAddresses();
    }
    int currentPrefix = 1;
    int prefixCount = 0;
    for(int i=0; i < numPrefixes; i++) {
        if(this->getInstance()->getAddressFamily() == IPV4INSTANCE)
        {
            Ipv4InterfaceData* ipv4Data = ie->ipv4Data();
            Ipv4Address ipAdd = ipv4Data->getIPAddress();
            OSPFv3LSAPrefix *prefix = new OSPFv3LSAPrefix();
            prefix->prefixLen= ipv4Data->getNetmask().getNetmaskLength();
            prefix->metric = METRIC;
            prefix->addressPrefix=L3Address(ipAdd.getPrefix(prefix->prefixLen));
            newLsa->setPrefixesArraySize(currentPrefix);
            newLsa->setPrefixes(currentPrefix-1, *prefix);
            prefixCount++;
            currentPrefix++;

        }
        else
        {
            Ipv6Address ipv6 = ipv6int->getAddress(i);
        //        Ipv6Address ipv6 = ipv6int->getAdvPrefix(i).prefix;
            if(ipv6.isGlobal()) {//Only all the global prefixes belong to the Intra-Area-Prefix LSA
                OSPFv3LSAPrefix *prefix = new OSPFv3LSAPrefix();
                int rIndex = this->getInstance()->getProcess()->isInRoutingTable6(this->getInstance()->getProcess()->rt6, ipv6);
                if (rIndex >= 0)
                    prefix->prefixLen = this->getInstance()->getProcess()->rt6->getRoute(rIndex)->getPrefixLength();
                else
                    prefix->prefixLen = 64;
                prefix->metric = METRIC;
                prefix->addressPrefix=ipv6.getPrefix(prefix->prefixLen);

                newLsa->setPrefixesArraySize(currentPrefix);
                newLsa->setPrefixes(currentPrefix-1, *prefix);
                prefixCount++;
                currentPrefix++;
            }
        }
    }

    newLsa->setNumPrefixes(prefixCount);

    // check if created LSA type 9 would be other or same as previous
    if (checkDuplicate)
    {
        IntraAreaPrefixLSA* prefixLsa = IntraAreaPrefixLSAAlreadyExists(newLsa);
        if (prefixLsa != nullptr)
        {
            this->subtractIntraAreaPrefixLinkStateID();
            delete (newLsa);
            return prefixLsa;
        }
    }
    this->incrementIntraAreaPrefixSequence();
    return newLsa;
}

// return nullptr, if newLsa is not a duplicate
IntraAreaPrefixLSA* OSPFv3Area::IntraAreaPrefixLSAAlreadyExists(OSPFv3IntraAreaPrefixLSA *newLsa)
{
    for (auto it= this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
    {
        if ((*it)->getHeader().getAdvertisingRouter() == newLsa->getHeader().getAdvertisingRouter() &&
                (*it)->getHeader().getLsaAge() != MAX_AGE)
        {
           if ((*it)->getReferencedLSType() == newLsa->getReferencedLSType())
           {
               if((*it)->getPrefixesArraySize() == newLsa->getPrefixesArraySize()) // or use snumPrefixes ?
               {
                   bool same = false;
                   for (int x = 0; x < newLsa->getPrefixesArraySize(); x++) //prefixCount is count of just created LSA
                   {
                       if(((*it)->getPrefixes(x).addressPrefix == newLsa->getPrefixes(x).addressPrefix) &&
                          ((*it)->getPrefixes(x).prefixLen == newLsa->getPrefixes(x).prefixLen) &&
                          ((*it)->getPrefixes(x).metric == newLsa->getPrefixes(x).metric) &&
                          ((*it)->getPrefixes(x).dnBit == newLsa->getPrefixes(x).dnBit) &&
                          ((*it)->getPrefixes(x).laBit == newLsa->getPrefixes(x).laBit) &&
                          ((*it)->getPrefixes(x).nuBit == newLsa->getPrefixes(x).nuBit) &&
                          ((*it)->getPrefixes(x).pBit == newLsa->getPrefixes(x).pBit) &&
                          ((*it)->getPrefixes(x).xBit == newLsa->getPrefixes(x).xBit)
                          )
                       {
                           same = true;
                       }
                       else
                           same = false;
                   }
                   if (same)
                   {
                       // return existing LSA type 9
                       return (*it);
                   }
               }
           }
       }
    }
    return nullptr;
}

bool OSPFv3Area::installIntraAreaPrefixLSA(const OSPFv3IntraAreaPrefixLSA *lsaC)
{
    auto lsa = lsaC->dup(); // make editable copy of lsa
    OSPFv3LSAHeader &header = lsa->getHeaderForUpdate();

    EV_DEBUG << "Installing Intra-Area-Prefix LSA:\nLink State ID: " << header.getLinkStateID() << "\nAdvertising router: " << header.getAdvertisingRouter();
    EV_DEBUG << "\nLS Seq Number: " << header.getLsaSequenceNumber() << "\nReferenced LSA Type: " << lsa->getReferencedLSType();

    for(int i = 0; i<lsa->getNumPrefixes(); i++) {
        const OSPFv3LSAPrefix &prefix = lsa->getPrefixes(i);
        EV_DEBUG << "Prefix Address: " << prefix.addressPrefix;
        EV_DEBUG << "\nPrefix Length: " << prefix.prefixLen;
        if(prefix.dnBit)
            EV_DEBUG << "DN ";
        if(prefix.laBit)
            EV_DEBUG << "LA ";
        if(prefix.nuBit)
            EV_DEBUG << "NU ";
        if(prefix.pBit)
            EV_DEBUG << "P ";
        if(prefix.xBit)
            EV_DEBUG << "X ";

        EV_DEBUG << ", Metric: " << prefix.metric << "\n";
    }

    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    if(lsa->getReferencedLSType() == NETWORK_LSA)
    {
        int intraPrefCnt = this->intraAreaPrefixLSAList.size();
        int in = 0;
        while (in < intraPrefCnt)
        {
            IntraAreaPrefixLSA* prefLSA = this->getIntraAreaPrefixLSA(in);
            bool erase = false;

            for (int prefN = 0; prefN < lsa->getPrefixesArraySize(); prefN++)
            {
                for (int prefR = 0; prefR < prefLSA->getPrefixesArraySize(); prefR++)
                {
                    L3Address netPref = lsa->getPrefixesForUpdate(prefN).addressPrefix;
                    short netPrefixLen = lsa->getPrefixes(prefN).prefixLen;
                    if(prefLSA->getReferencedLSType() == ROUTER_LSA)
                    {
                        L3Address routerPref = prefLSA->getPrefixesForUpdate(prefR).addressPrefix;
                        short routerPrefixLen = prefLSA->getPrefixes(prefR).prefixLen;

                        // if router recieve LSA type 9 from DR with IPv6 which roter have stored aj LSType 1, delete this old LSA and install new one
                        if(routerPref.getPrefix(routerPrefixLen) == netPref.getPrefix(netPrefixLen))
                        {
                            EV_DEBUG << "Deleting old IntraAreaPrefixLSA, install new one IntraAreaPrefixLSA\n";
                            this->intraAreaPrefixLSAList.erase(this->intraAreaPrefixLSAList.begin()+in);
                            erase = true;
                            break;
                        }
                    }
                }

                if (erase)
                    break;
            }
            //if something was deleted, go through whole cycle once again
            if (intraPrefCnt == this->intraAreaPrefixLSAList.size())
                in++;
            else
            {
                intraPrefCnt = this->intraAreaPrefixLSAList.size();
                in = 0;
            }
        }
    }

    if(lsa->getReferencedLSType() == ROUTER_LSA){
        int intraPrefCnt = this->getIntraAreaPrefixLSACount();
        for(int i=0; i<intraPrefCnt; i++){
            OSPFv3IntraAreaPrefixLSA* prefLSA = this->getIntraAreaPrefixLSA(i);
            if(prefLSA->getReferencedLSType() == NETWORK_LSA) {
                for (int prefR = 0; prefR < lsa->getPrefixesArraySize(); prefR++) //prefixes of incoming LSA
                {
                    L3Address routerPref = lsa->getPrefixes(prefR).addressPrefix;
                    short routerPrefixLen = lsa->getPrefixes(prefR).prefixLen;
                    for (int prefN = 0; prefN < prefLSA->getPrefixesArraySize(); prefN++) //prefixes of stored LSA
                    {
                        L3Address netPref = prefLSA->getPrefixes(prefN).addressPrefix;
                        short netPrefixLen = prefLSA->getPrefixes(prefN).prefixLen;
                        if(routerPref.getPrefix(routerPrefixLen) == netPref.getPrefix(netPrefixLen)) {
                            EV_DEBUG << "Came LSA type 9 with referenced prefix of LSType 1, have one with LSType 2, doing nothing\n"; //TODO: This become relevant when there will be support for active changing of type of link
                        }
                    }
                }
            }
        }
    }
    //chcek if this is not same LSA as router already know
    IntraAreaPrefixLSA* lsaInDatabase = (IntraAreaPrefixLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase == nullptr)
         lsaInDatabase = IntraAreaPrefixLSAAlreadyExists(lsa);

    for (auto it= this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
    {
        if ((*it)->getHeader().getAdvertisingRouter() == lsa->getHeader().getAdvertisingRouter() &&
                ((*it)->getReferencedLSType() == lsa->getReferencedLSType()) &&
                ((*it)->getHeader().getLinkStateID() < lsa->getHeader().getLinkStateID()) &&
                ((*it)->getPrefixesArraySize() != lsa->getPrefixesArraySize()) &&
                ((*it)->getHeader().getLsaAge() != MAX_AGE))
        { //this is newer LSA type 9 with different number of ref prefixes
            lsaInDatabase = (*it);
            break;
        }
    }

    if (lsaInDatabase != nullptr)
    {
        if(lsaInDatabase->getHeader().getLsaSequenceNumber() <= lsa->getHeader().getLsaSequenceNumber())
        {
            this->removeFromAllRetransmissionLists(lsaKey);
            return this->updateIntraAreaPrefixLSA(lsaInDatabase, lsa);
        }
    }
    else if (lsa->getReferencedLSType() == NETWORK_LSA || lsa->getReferencedLSType() == ROUTER_LSA)
    {
        IntraAreaPrefixLSA* lsaCopy = new IntraAreaPrefixLSA(*lsa);
        this->intraAreaPrefixLSAList.push_back(lsaCopy);

        if (this->getInstance()->getAreaCount() > 1)
            originateInterAreaPrefixLSA(lsaCopy, this , true);

        return true;
    }
    return false;

}//installIntraAreaPrefixLSA


bool OSPFv3Area::updateIntraAreaPrefixLSA(IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa)
{
    bool different = intraAreaPrefixLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->resetInstallTime();
//    currentLsa->getHeaderForUpdate().setLsaAge(0);//reset the age
    if (different) {
        return true;
    }
    else {
        return false;
    }
}//updateIntraAreaPrefixLSA

bool OSPFv3Area::intraAreaPrefixLSADiffersFrom(OSPFv3IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getNumPrefixes() != newLsa->getNumPrefixes()) ||
                         (currentLsa->getReferencedLSType() != newLsa->getReferencedLSType()) ||
                         (currentLsa->getReferencedLSID() != newLsa->getReferencedLSID()) ||
                         (currentLsa->getReferencedAdvRtr() != newLsa->getReferencedAdvRtr()));


        if (!differentBody) {
            unsigned int referenceCount = currentLsa->getNumPrefixes();
            for (unsigned int i = 0; i < referenceCount; i++) {
                OSPFv3LSAPrefix currentPrefix = currentLsa->getPrefixes(i);
                OSPFv3LSAPrefix newPrefix = newLsa->getPrefixes(i);
                bool differentLink = ((currentPrefix.addressPrefix != newPrefix.addressPrefix) ||
                                      (currentPrefix.dnBit != newPrefix.dnBit) ||
                                      (currentPrefix.laBit != newPrefix.laBit) ||
                                      (currentPrefix.metric != newPrefix.metric) ||
                                      (currentPrefix.nuBit != newPrefix.nuBit) ||
                                      (currentPrefix.pBit != newPrefix.pBit) ||
                                      (currentPrefix.prefixLen != newPrefix.prefixLen) ||
                                      (currentPrefix.xBit != newPrefix.xBit));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}//intraAreaPrefixLSADiffersFrom

Ipv4Address OSPFv3Area::getNewIntraAreaPrefixLinkStateID()
{
    Ipv4Address currIP = this->intraAreaPrefixLsID;
    int newIP = currIP.getInt()+1;
    this->intraAreaPrefixLsID = Ipv4Address(newIP);
    return currIP;
}//getNewIntraAreaPrefixStateID

void OSPFv3Area::subtractIntraAreaPrefixLinkStateID()
{
    Ipv4Address currIP = this->intraAreaPrefixLsID;
    int newIP = currIP.getInt()-1;
    this->intraAreaPrefixLsID = Ipv4Address(newIP);
}//getNewIntraAreaPrefixStateID

IntraAreaPrefixLSA* OSPFv3Area::findIntraAreaPrefixByAddress(L3Address address, int prefix)
{
    for (auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
    {
        for (int i = 0; i < (*it)->getPrefixesArraySize(); i++)
        {
            if ((*it)->getPrefixes(i).addressPrefix == address && (*it)->getPrefixes(i).prefixLen == prefix)
                return (*it);
        }
    }
    return nullptr;
}

IntraAreaPrefixLSA* OSPFv3Area::findIntraAreaPrefixLSAByReference(LSAKeyType lsaKey)
{
    for (auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
    {
        if(((*it)->getReferencedLSType() == lsaKey.LSType) && ((*it)->getReferencedLSID() == lsaKey.linkStateID) && ((*it)->getReferencedAdvRtr() == lsaKey.advertisingRouter))
        {
            return (*it);
        }
    }
    return nullptr;
}

OSPFv3LSA* OSPFv3Area::getLSAbyKey(LSAKeyType LSAKey)
{
    switch(LSAKey.LSType){
    case ROUTER_LSA:
        for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }
        break;

    case NETWORK_LSA:
        for (auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }
        break;

    case INTER_AREA_PREFIX_LSA:
        for (auto it=this->interAreaPrefixLSAList.begin(); it!=this->interAreaPrefixLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }
        break;

    case INTER_AREA_ROUTER_LSA:
        for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }
        break;

    case NSSA_LSA:
        for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }
        break;

    case INTRA_AREA_PREFIX_LSA:
        for (auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }
        break;

    case LINK_LSA:
        for (auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
        {
            LinkLSA* lsa = (*it)->getLinkLSAbyKey(LSAKey);
            if(lsa != nullptr)
                return lsa;
        }
        break;
    }
    //TODO - link lsa from interface
    //TODO - as external from top data structure


    return nullptr;
}

// add created address range into list of area's address ranges
void OSPFv3Area::addAddressRange(Ipv6AddressRange addressRange, bool advertise)
{
    int addressRangeNum = IPv6areaAddressRanges.size();
    bool found = false;
    bool erased = false;

    for (int i = 0; i < addressRangeNum; i++) {
        Ipv6AddressRange curRange = IPv6areaAddressRanges[i];
        if (curRange.contains(addressRange)) {    // contains or same
            found = true;
            if (IPv6advertiseAddressRanges[curRange] != advertise) {
                throw cRuntimeError("Inconsistent advertise settings for %s and %s address ranges in area %s",
                        addressRange.str().c_str(), curRange.str().c_str(), areaID.str(false).c_str());
            }
        }
        else if (addressRange.contains(curRange)) {
            if (IPv6advertiseAddressRanges[curRange] != advertise) {
                throw cRuntimeError("Inconsistent advertise settings for %s and %s address ranges in area %s",
                        addressRange.str().c_str(), curRange.str().c_str(), areaID.str(false).c_str());
            }
            IPv6advertiseAddressRanges.erase(curRange);
            IPv6areaAddressRanges[i] = NULL_IPV6ADDRESSRANGE;
            erased = true;
        }
    }
    if (erased && found) // the found entry contains new entry and new entry contains erased entry ==> the found entry also contains the erased entry
        throw cRuntimeError("Model error: bad contents in areaAddressRanges vector");
    if (erased) {
        auto it = IPv6areaAddressRanges.begin();
        while (it != IPv6areaAddressRanges.end()) {
            if (*it == NULL_IPV6ADDRESSRANGE)
                it = IPv6areaAddressRanges.erase(it);
            else
                it++;
        }
    }
    if (!found) {
        IPv6areaAddressRanges.push_back(addressRange);
        IPv6advertiseAddressRanges[addressRange] = advertise;
    }
}

bool OSPFv3Area::hasAddressRange(Ipv6AddressRange addressRange) const
{
    int addressRangeNum = IPv6areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (IPv6areaAddressRanges[i] == addressRange) {
            return true;
        }
    }
    return false;
}

// same check but for IPv4
void OSPFv3Area::addAddressRange(Ipv4AddressRange addressRange, bool advertise)
{
    int addressRangeNum = IPv4areaAddressRanges.size();
    bool found = false;
    bool erased = false;

    for (int i = 0; i < addressRangeNum; i++) {
        Ipv4AddressRange curRange = IPv4areaAddressRanges[i];
        if (curRange.contains(addressRange)) {    // contains or same
            found = true;
            if (IPv4advertiseAddressRanges[curRange] != advertise) {
                throw cRuntimeError("Inconsistent advertise settings for %s and %s address ranges in area %s",
                        addressRange.str().c_str(), curRange.str().c_str(), areaID.str(false).c_str());
            }
        }
        else if (addressRange.contains(curRange)) {
            if (IPv4advertiseAddressRanges[curRange] != advertise) {
                throw cRuntimeError("Inconsistent advertise settings for %s and %s address ranges in area %s",
                        addressRange.str().c_str(), curRange.str().c_str(), areaID.str(false).c_str());
            }
            IPv4advertiseAddressRanges.erase(curRange);
            IPv4areaAddressRanges[i] = NULL_IPV4ADDRESSRANGE;
            erased = true;
        }
    }
    if (erased && found) // the found entry contains new entry and new entry contains erased entry ==> the found entry also contains the erased entry
        throw cRuntimeError("Model error: bad contents in IPv4areaAddressRanges vector");
    if (erased) {
        auto it = IPv4areaAddressRanges.begin();
        while (it != IPv4areaAddressRanges.end()) {
            if (*it == NULL_IPV4ADDRESSRANGE)
                it = IPv4areaAddressRanges.erase(it);
            else
                it++;
        }
    }
    if (!found) {
        IPv4areaAddressRanges.push_back(addressRange);
        IPv4advertiseAddressRanges[addressRange] = advertise;
    }
}

bool OSPFv3Area::hasAddressRange(Ipv4AddressRange addressRange) const
{
    int addressRangeNum = IPv4areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if (IPv4areaAddressRanges[i] == addressRange) {
            return true;
        }
    }
    return false;
}


void OSPFv3Area::calculateShortestPathTree(std::vector<OSPFv3RoutingTableEntry* >& newTableIPv6, std::vector<OSPFv3IPv4RoutingTableEntry* >& newTableIPv4)
{
    EV_DEBUG << "Calculating SPF Tree for area " << this->getAreaID() << "\n";
    /*1)Initialize the algorithm's data structures. Clear the list
        of candidate vertices. Initialize the shortest-path tree to
        only the root (which is the router doing the calculation).
    */
    Ipv4Address routerID = this->getInstance()->getProcess()->getRouterID();
    bool finished = false;
    std::vector<OSPFv3LSA*> treeVertices;
    OSPFv3LSA *justAddedVertex;
    std::vector<OSPFv3LSA *> candidateVertices;
    unsigned long i, j, k;
    unsigned long lsaCount;

    if (spfTreeRoot == nullptr) {
        RouterLSA *routerLSA = findRouterLSA(routerID);
        if (routerLSA == nullptr)
        {
            RouterLSA *newLSA = originateRouterLSA();
            if (installRouterLSA(newLSA))
            {
                routerLSA = findRouterLSA(routerID);
                spfTreeRoot = routerLSA;
                floodLSA(newLSA);       //spread LSA to whole network
                delete newLSA;
            }
        }
        else
            spfTreeRoot = routerLSA;

    }
    if (spfTreeRoot == nullptr)
        return;

    lsaCount = routerLSAList.size();
    cout << "lsaCount = " << lsaCount << endl;

    for (i = 0; i < lsaCount; i++) {
        routerLSAList[i]->clearNextHops();
    }
    lsaCount = networkLSAList.size();
    for (i = 0; i < lsaCount; i++) {
        networkLSAList[i]->clearNextHops();
    }

    spfTreeRoot->setDistance(0);
    treeVertices.push_back(spfTreeRoot);   // root is first vertex in dijkstra alg
    justAddedVertex = spfTreeRoot;    // (1)

    do {
        OSPFv3LSAFunctionCode vertexType = static_cast<OSPFv3LSAFunctionCode>(justAddedVertex->getHeader().getLsaType());
        if (vertexType == ROUTER_LSA) {
            RouterLSA *routerVertex = check_and_cast<RouterLSA *>(justAddedVertex);
            if (routerVertex->getVBit()) {
                transitCapability = true;
            }

            int testCount = routerVertex->getRoutersArraySize();
            for (int iteration = 0; iteration < testCount; iteration++)
            {
                OSPFv3RouterLSABody router = routerVertex->getRouters(iteration);
                OSPFv3LSA* joiningVertex;           // joiningVertex is source vertex
                OSPFv3LSAFunctionCode joiningVertexType;
                /*The Vertex ID for a router is the OSPF Router ID.  The Vertex ID
                  for a transit network is a combination of the Interface ID and
                  OSPF Router ID of the network's Designated Router.*/
                if (router.type == TRANSIT_NETWORK) {
                    joiningVertex = findNetworkLSA(router.neighborInterfaceID, router.neighborRouterID);
                    joiningVertexType = NETWORK_LSA;
                }
                else {  // P2P
                    joiningVertex = findRouterLSA(router.neighborRouterID);
                }

                if ((joiningVertex == nullptr) ||
                (joiningVertex->getHeader().getLsaAge() == MAX_AGE)
                 || (!hasLink(joiningVertex, justAddedVertex)))    // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize = treeVertices.size();    // already visited vertices (at the beginning, only root)
                bool alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {                // if vertex, which was found is already in set of visited vertices, go to another one
                    if (treeVertices[j] == joiningVertex) {
                      alreadyOnTree = true;
                      break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
//                    EV_DEBUG << "continue\n";
                    continue;
                }

                unsigned long linkStateCost = routerVertex->getDistance() + routerVertex->getRouters(iteration).metric;
                unsigned int candidateCount = candidateVertices.size();     //candidateVertices is zero at the beginning
                OSPFv3LSA *candidate = nullptr;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != nullptr) { // (2) (d)               // first iteration, candidate is nullptr
                    RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidate);
                    unsigned long candidateDistance = routingInfo->getDistance();

                    if (linkStateCost > candidateDistance) {
                       continue;
                    }
                    if (linkStateCost < candidateDistance) {
                       routingInfo->setDistance(linkStateCost);
                       routingInfo->clearNextHops();
                    }
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                       routingInfo->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                }
                else
                {
                   if (joiningVertexType == ROUTER_LSA)
                   {
                       RouterLSA *joiningRouterVertex = check_and_cast<RouterLSA *>(joiningVertex);
                       joiningRouterVertex->setDistance(linkStateCost);
                       std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                       unsigned int nextHopCount = newNextHops->size();
                       for (k = 0; k < nextHopCount; k++) {
                           joiningRouterVertex->addNextHop((*newNextHops)[k]);
                       }
                       delete newNextHops;
                       RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningRouterVertex);
                       vertexRoutingInfo->setParent(justAddedVertex);

                       candidateVertices.push_back(joiningRouterVertex);
                   }
                   else
                   {    //joiningVertexType == NETWORK_LSA
                        NetworkLSA *joiningNetworkVertex = check_and_cast<NetworkLSA *>(joiningVertex);
                        joiningNetworkVertex->setDistance(linkStateCost);
                        std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)

                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++)
                        {
                            joiningNetworkVertex->addNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        // justAdded is source vertex
                        // joining is destination vertex
                        // joiningNetworkVertex == joiningVertex
                        RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningNetworkVertex);
                        vertexRoutingInfo->setParent(justAddedVertex);

                        candidateVertices.push_back(joiningNetworkVertex);
                   }

                }
              }  // end of for
         }    //   (vertexType == ROUTER_LSA)

        if (vertexType == NETWORK_LSA)
        {
            NetworkLSA *networkVertex = check_and_cast<NetworkLSA *>(justAddedVertex);
            unsigned int routerCount = networkVertex->getAttachedRouterArraySize();
            for (i = 0; i < routerCount; i++) {    // (2)
                RouterLSA *joiningVertex = findRouterLSA(networkVertex->getAttachedRouter(i));
                if ((joiningVertex == nullptr) ||
                    (joiningVertex->getHeader().getLsaAge() == MAX_AGE) ||
                    (!hasLink(joiningVertex, justAddedVertex)))    // (from, to)     (2) (b)
                {
                    continue;
                }
                unsigned int treeSize = treeVertices.size();
                bool alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c) already on tree, continue
                    continue;
                }

                unsigned long linkStateCost = networkVertex->getDistance();    // link cost from network to router is always 0
                unsigned int candidateCount = candidateVertices.size();
                OSPFv3LSA *candidate = nullptr;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != nullptr) {    // (2) (d)
                    RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidate);
                    unsigned long candidateDistance = routingInfo->getDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->setDistance(linkStateCost);
                        routingInfo->clearNextHops();
                    }
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                }
                else {
                    joiningVertex->setDistance(linkStateCost);
                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)

                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        joiningVertex->addNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                    RoutingInfo *vertexRoutingInfo = check_and_cast<RoutingInfo *>(joiningVertex);
                    vertexRoutingInfo->setParent(justAddedVertex);

                    candidateVertices.push_back(joiningVertex);
                }
            }
        }

        if (candidateVertices.empty()) {    // (3)
            finished = true;
        }
        else
        {
            unsigned int candidateCount = candidateVertices.size();
            unsigned long minDistance = LS_INFINITY;
            OSPFv3LSA *closestVertex = candidateVertices[0];

            // this for-cycle edit distane from source vertex to all others adjacent vertices by dijstra algorithm
            for (i = 0; i < candidateCount; i++)
            {
                RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidateVertices[i]);
                unsigned long currentDistance = routingInfo->getDistance();

                if (currentDistance < minDistance) {
                    closestVertex = candidateVertices[i];
                    minDistance = currentDistance;
                }
                else {
                    if (currentDistance == minDistance) {
                        if ((closestVertex->getHeader().getLsaType() == ROUTER_LSA) &&
                            (candidateVertices[i]->getHeader().getLsaType() == NETWORK_LSA))
                        {
                            closestVertex = candidateVertices[i];
                        }
                    }
                }
            }

            treeVertices.push_back(closestVertex);      // treeVertices is the main SPF tree
            // delete selected closestVertex  from candidateVertices
            for (auto it = candidateVertices.begin(); it != candidateVertices.end(); it++) {
                if ((*it) == closestVertex) {
                    candidateVertices.erase(it);
                    break;
                }
            }
            if (closestVertex->getHeader().getLsaType() == ROUTER_LSA) {
                RouterLSA *routerLSA = check_and_cast<RouterLSA *>(closestVertex);

                int arrSiz = routerLSA->getRoutersArraySize();
                // find intraAreaPrefix LSA  for this vertex
                if ((routerLSA->getBBit() || routerLSA->getEBit()) &&
                     (routerLSA->getHeader().getAdvertisingRouter() != this->getInstance()->getProcess()->getRouterID())) //in broadcast network, only if this is ABR or ASBR
                {
                    int attached = -1; // find out which route from Router_LSA is already in treeVertices

                    //check routeCounts and if it is more than 1, visited Vertex take bigger priority
                    if (arrSiz > 1)
                    {
                        unsigned int tree = treeVertices.size();    // already visited vertices (at the beginning, only root)

                        for (int t = 0; t < tree; t++)
                        {
                            for (int r = 0; r < arrSiz; r++)
                            {
                                if (treeVertices[t]->getHeader().getAdvertisingRouter() == routerLSA->getRouters(r).neighborRouterID )
                                {
                                  attached = r;
                                  break;
                                }
                            }
                            if (attached >= 0) //found match
                                break;
                        }
                    }

                    if (attached < 0) //if no match was found, use first in routerArray of this router-LSA
                        attached = 0;

                    if (arrSiz > 0)
                    {
                        LSAKeyType lsaKey;
                        if (routerLSA->getRouters(attached).type == POINT_TO_POINT)
                        {
                            lsaKey.linkStateID = (Ipv4Address)routerLSA->getHeader().getLinkStateID();
                            lsaKey.advertisingRouter = routerLSA->getHeader().getAdvertisingRouter();
                        }
                        else if(routerLSA->getRouters(attached).type == TRANSIT_NETWORK) //link is connected into BROADCAST network
                        {
                            lsaKey.linkStateID = (Ipv4Address)routerLSA->getRouters(attached).neighborInterfaceID;
                            lsaKey.advertisingRouter = routerLSA->getRouters(attached).neighborRouterID;
                        }
                        else
                            continue;
                        lsaKey.LSType = routerLSA->getRouters(attached).type ;
                        addRouterEntry(routerLSA, lsaKey, newTableIPv6, newTableIPv4);
                    }
                }

                // check if router has any host networks
                if (routerLSA->getHeader().getAdvertisingRouter() != this->getInstance()->getProcess()->getRouterID())
                {
                    LSAKeyType lsaKey;
                    lsaKey.linkStateID = (Ipv4Address)routerLSA->getHeader().getLinkStateID();
                    lsaKey.advertisingRouter = routerLSA->getHeader().getAdvertisingRouter();
                    lsaKey.LSType = ROUTER_LSA;
                    addRouterEntry(routerLSA, lsaKey, newTableIPv6, newTableIPv4);
                }
            }

            if (closestVertex->getHeader().getLsaType() == NETWORK_LSA) {
                NetworkLSA *networkLSA = check_and_cast<NetworkLSA *>(closestVertex);

                // address is extracted from Intra-Area-Prefix-LSA
                LSAKeyType lsaKey;
                lsaKey.linkStateID = networkLSA->getHeader().getLinkStateID();
                lsaKey.advertisingRouter = networkLSA->getHeader().getAdvertisingRouter();
                lsaKey.LSType = NETWORK_LSA;  //navyse

                L3Address destinationID;
                uint8_t prefixLen;
                OSPFv3IntraAreaPrefixLSA *iapLSA = findIntraAreaPrefixLSAByReference(lsaKey);
                if (iapLSA != nullptr)
                {
                    for (int i = 0; i < iapLSA->getPrefixesArraySize(); i++) {
                        destinationID = iapLSA->getPrefixes(i).addressPrefix;
                        prefixLen = iapLSA->getPrefixes(i).prefixLen;
                        unsigned int nextHopCount = networkLSA->getNextHopCount();
                        bool overWrite = false;
                        if (destinationID.getType() == L3Address::IPv6)         // for ipv6 AF
                        {
                            OSPFv3RoutingTableEntry *entry = nullptr;
                            unsigned long routeCount = newTableIPv6.size();
                            Ipv6Address longestMatch(Ipv6Address::UNSPECIFIED_ADDRESS);

                            for (int rt = 0; rt < routeCount; rt++) {
                                if (newTableIPv6[rt]->getDestinationType() == OSPFv3RoutingTableEntry::NETWORK_DESTINATION) {
                                    OSPFv3RoutingTableEntry *routingEntry = newTableIPv6[rt];
                                    Ipv6Address entryAddress = routingEntry->getDestPrefix();

                                    if (entryAddress == destinationID.toIpv6())
                                    {
                                        if (destinationID.toIpv6() > longestMatch) {
                                            longestMatch = destinationID.toIpv6();
                                            entry = routingEntry;
                                        }
                                    }
                                }
                            }
                            if (entry != nullptr) {
                                const OSPFv3LSA *entryOrigin = entry->getLinkStateOrigin();
                                if ((entry->getCost() != networkLSA->getDistance()) ||
                                    (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                                {
                                    overWrite = true;
                                }
                            }

                            if ((entry == nullptr) || (overWrite)) {
                                if (entry == nullptr) {
                                    entry = new OSPFv3RoutingTableEntry(this->getInstance()->ift, destinationID.toIpv6(), prefixLen, IRoute::OSPF);
                                }
                                entry->setLinkStateOrigin(networkLSA);
                                entry->setArea(areaID);
                                entry->setPathType(OSPFv3RoutingTableEntry::INTRAAREA);
                                entry->setCost(networkLSA->getDistance());
                                entry->setDestinationType(OSPFv3RoutingTableEntry::NETWORK_DESTINATION);
                                entry->setOptionalCapabilities(networkLSA->getOspfOptions());


                                for (int j = 0; j < nextHopCount; j++) {
                                    entry->addNextHop(networkLSA->getNextHop(j));
                                }
                                if (!overWrite) {
                                    newTableIPv6.push_back(entry);
                                }
                            }
                        }
                        else        // for IPv4 AF
                        {
                            OSPFv3IPv4RoutingTableEntry *entry = nullptr;
                            unsigned long routeCount = newTableIPv4.size();
                            Ipv4Address longestMatch(Ipv4Address::UNSPECIFIED_ADDRESS);

                            for (int rt = 0; rt < routeCount; rt++) {
                                if (newTableIPv4[rt]->getDestinationType() == OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION) {
                                    OSPFv3IPv4RoutingTableEntry *routingEntry = newTableIPv4[rt];
                                    Ipv4Address entryAddress = routingEntry->getDestination();

                                    if (entryAddress == destinationID.toIpv4())
                                    {
                                        if (destinationID.toIpv4() > longestMatch) {
                                            longestMatch = destinationID.toIpv4();
                                            entry = routingEntry;
                                        }
                                    }
                                }
                            }
                            if (entry != nullptr) {
                                const OSPFv3LSA *entryOrigin = entry->getLinkStateOrigin();
                                if ((entry->getCost() != networkLSA->getDistance()) ||
                                    (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                                {
                                    overWrite = true;
                                }
                            }

                            if ((entry == nullptr) || (overWrite)) {
                                if (entry == nullptr) {
                                    entry = new OSPFv3IPv4RoutingTableEntry(this->getInstance()->ift, destinationID.toIpv4(), prefixLen, IRoute::OSPF);
                                }
                                entry->setLinkStateOrigin(networkLSA);
                                entry->setArea(areaID);
                                entry->setPathType(OSPFv3IPv4RoutingTableEntry::INTRAAREA);
                                entry->setCost(networkLSA->getDistance());
                                entry->setDestinationType(OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION);
                                entry->setOptionalCapabilities(networkLSA->getOspfOptions());
                                for (int j = 0; j < nextHopCount; j++) {
                                    entry->addNextHop(networkLSA->getNextHop(j)); // tu mi to vklada IPv6 next hop pre IPv4 AF
                                }

                                if (!overWrite)
                                    newTableIPv4.push_back(entry);
                            }
                        }
                    }
                }
                else {
                    continue; // there is no LSA type 9 which would care a IP addresses for this calculating route, so skip to next one.
                }
            }
            justAddedVertex = closestVertex;
            }// end of else not empty()
    } while (!finished);
}


// add Router-LSA  entry into newTable based on AF which is for this process active
void OSPFv3Area::addRouterEntry(RouterLSA* routerLSA, LSAKeyType lsaKey, std::vector<OSPFv3RoutingTableEntry *>& newTableIPv6, std::vector<OSPFv3IPv4RoutingTableEntry *>& newTableIPv4 )
{
    L3Address destinationID;
    uint8_t prefixLen;

    OSPFv3IntraAreaPrefixLSA *iapLSA = findIntraAreaPrefixLSAByReference(lsaKey); //find appropriate LSA type 9 based on reference in packet.

    if (iapLSA != nullptr)
    {
        for (int i = 0; i < iapLSA->getPrefixesArraySize(); i++)
        {
            destinationID = iapLSA->getPrefixes(i).addressPrefix;
            prefixLen = iapLSA->getPrefixes(i).prefixLen;

            // if this LSA is part of IPv6 AF
            if (destinationID.getType() == L3Address::IPv6)
            {
                OSPFv3RoutingTableEntry *entry = new OSPFv3RoutingTableEntry(this->getInstance()->ift, destinationID.toIpv6(), prefixLen, IRoute::OSPF);
                unsigned int nextHopCount = routerLSA->getNextHopCount();
                OSPFv3RoutingTableEntry::RoutingDestinationType destinationType = OSPFv3RoutingTableEntry::NETWORK_DESTINATION;

                entry->setLinkStateOrigin(routerLSA);
                entry->setArea(areaID);
                entry->setPathType(OSPFv3RoutingTableEntry::INTRAAREA);
                entry->setCost(routerLSA->getDistance());
                if (routerLSA->getBBit())
                    destinationType |= OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION;

                if (routerLSA->getEBit())
                    destinationType |= OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION;

                entry->setDestinationType(destinationType);
                entry->setOptionalCapabilities(routerLSA->getOspfOptions());
                for (int j = 0; j < nextHopCount; j++) {
                    entry->addNextHop(routerLSA->getNextHop(j));
                }

                newTableIPv6.push_back(entry);
            }
            else // if this LSA is part of IPv4 AF
            {
                OSPFv3IPv4RoutingTableEntry *entry = new OSPFv3IPv4RoutingTableEntry(this->getInstance()->ift, destinationID.toIpv4(), prefixLen, IRoute::OSPF);
                unsigned int nextHopCount = routerLSA->getNextHopCount();
                OSPFv3RoutingTableEntry::RoutingDestinationType destinationType = OSPFv3RoutingTableEntry::NETWORK_DESTINATION;

                entry->setLinkStateOrigin(routerLSA);
                entry->setArea(areaID);
                entry->setPathType(OSPFv3IPv4RoutingTableEntry::INTRAAREA);
                entry->setCost(routerLSA->getDistance());
                if (routerLSA->getBBit()) {
                destinationType |= OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION;
                }
                if (routerLSA->getEBit()) {
                destinationType |= OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION;
                }

                entry->setDestinationType(destinationType);
                entry->setOptionalCapabilities(routerLSA->getOspfOptions());
                for (int j = 0; j < nextHopCount; j++) {
                    entry->addNextHop(routerLSA->getNextHop(j));
                }

                newTableIPv4.push_back(entry);
            }

        }
    }
}

/**
 * Browse through the newTable looking for entries describing the same destination
 * as the currentLSA. If a cheaper route is found then skip this LSA(return true), else
 * note those which are of equal or worse cost than the currentCost.
 */
// for IPv6 AF
bool OSPFv3Area::findSameOrWorseCostRoute(const std::vector<OSPFv3RoutingTableEntry *>& newTable,
        const InterAreaPrefixLSA& interAreaPrefixLSA,
        unsigned short currentCost,
        bool& destinationInRoutingTable,
        std::list<OSPFv3RoutingTableEntry *>& sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    long routeCount = newTable.size();
    Ipv6AddressRange destination;
    destination.prefix = interAreaPrefixLSA.getPrefix().toIpv6();
    destination.prefixLength = interAreaPrefixLSA.getPrefixLen();

    for (long j = 0; j < routeCount; j++) {
        OSPFv3RoutingTableEntry *routingEntry = newTable[j];
        bool foundMatching = false;
        if (interAreaPrefixLSA.getHeader().getLsaType() == INTER_AREA_PREFIX_LSA) {
            if ((routingEntry->getDestinationType() == OSPFv3RoutingTableEntry::NETWORK_DESTINATION) &&
                isSameNetwork(destination.prefix, destination.prefixLength, routingEntry->getDestPrefix(), routingEntry->getPrefixLength()))
            {
                foundMatching = true;
            }
        }
        else {
            if ((((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (destination.prefix == routingEntry->getDestPrefix()) &&
                (destination.prefixLength == routingEntry->getPrefixLength()))
            {
                foundMatching = true;
            }
        }

        if (foundMatching) {
            destinationInRoutingTable = true;

            /* If the matching entry is an INTRAAREA getRoute(intra-area paths are
             * always preferred to other paths of any cost), or it's a cheaper INTERAREA
             * route, then skip this LSA.
             */
            if ((routingEntry->getPathType() == OSPFv3RoutingTableEntry::INTRAAREA) ||
                ((routingEntry->getPathType() == OSPFv3RoutingTableEntry::INTERAREA) &&
                 (routingEntry->getCost() < currentCost)))
            {
                return true;
            }
            else {
                // if it's an other INTERAREA path
                if ((routingEntry->getPathType() == OSPFv3RoutingTableEntry::INTERAREA) &&
                    (routingEntry->getCost() >= currentCost))
                {
                    sameOrWorseCost.push_back(routingEntry);
                }    // else it's external -> same as if not in the table
            }
        }
    }
    return false;
}

//  for IPv4 AF
bool OSPFv3Area::findSameOrWorseCostRoute(const std::vector<OSPFv3IPv4RoutingTableEntry *>& newTable,
        const InterAreaPrefixLSA& interAreaPrefixLSA,
        unsigned short currentCost,
        bool& destinationInRoutingTable,
        std::list<OSPFv3IPv4RoutingTableEntry *>& sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    long routeCount = newTable.size();
    Ipv4AddressRange destination;
    destination.address = interAreaPrefixLSA.getPrefix().toIpv4();
    destination.mask = destination.address.makeNetmask(interAreaPrefixLSA.getPrefixLen());

    for (long j = 0; j < routeCount; j++) {
        OSPFv3IPv4RoutingTableEntry *routingEntry = newTable[j];
        bool foundMatching = false;

        if (interAreaPrefixLSA.getHeader().getLsaType() == INTER_AREA_PREFIX_LSA) {
            if ((routingEntry->getDestinationType() == OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION) &&
                isSameNetwork(destination.address, destination.mask, routingEntry->getDestination(), routingEntry->getNetmask()))
            {
                foundMatching = true;
            }
        }
        else {
            if ((((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                 ((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                (destination.address == routingEntry->getDestination()) &&
                (destination.mask == routingEntry->getNetmask()))
            {
                foundMatching = true;
            }
        }

        if (foundMatching) {
            destinationInRoutingTable = true;

            /* If the matching entry is an INTRAAREA getRoute(intra-area paths are
             * always preferred to other paths of any cost), or it's a cheaper INTERAREA
             * route, then skip this LSA.
             */
            if ((routingEntry->getPathType() == OSPFv3IPv4RoutingTableEntry::INTRAAREA) ||
                ((routingEntry->getPathType() == OSPFv3IPv4RoutingTableEntry::INTERAREA) &&
                 (routingEntry->getCost() < currentCost)))
            {
                return true;
            }
            else {
                // if it's an other INTERAREA path
                if ((routingEntry->getPathType() == OSPFv3IPv4RoutingTableEntry::INTERAREA) &&
                    (routingEntry->getCost() >= currentCost))
                {
                    sameOrWorseCost.push_back(routingEntry);
                }    // else it's external -> same as if not in the table
            }
        }
    }
    return false;
}

/**
 * Returns a new RoutingTableEntry based on the input Inter-Area-Prefix LSA, with the input cost
 * and the borderRouterEntry's next hops.
 */
// for IPv6 AF
OSPFv3RoutingTableEntry *OSPFv3Area::createRoutingTableEntryFromInterAreaPrefixLSA(const InterAreaPrefixLSA& interAreaPrefixLSA,
        unsigned short entryCost,
        const OSPFv3RoutingTableEntry& borderRouterEntry) const
{
    Ipv6AddressRange destination;

    destination.prefix = interAreaPrefixLSA.getPrefix().toIpv6();
    destination.prefixLength = interAreaPrefixLSA.getPrefixLen();
    //TODO: AS boundary is not implemented
    OSPFv3RoutingTableEntry *newEntry = new OSPFv3RoutingTableEntry(this->getInstance()->ift, destination.prefix, destination.prefixLength,IRoute::OSPF);

    if (interAreaPrefixLSA.getHeader().getLsaType() == INTER_AREA_PREFIX_LSA)
    {
        newEntry->setDestinationType(OSPFv3RoutingTableEntry::NETWORK_DESTINATION);
    }
    else {
        newEntry->setDestinationType(OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION);
    }
    newEntry->setLinkStateOrigin(&interAreaPrefixLSA);
    newEntry->setArea(areaID);
    newEntry->setPathType(OSPFv3RoutingTableEntry::INTERAREA);
    newEntry->setCost(entryCost);

    unsigned int nextHopCount = borderRouterEntry.getNextHopCount();
    for (unsigned int j = 0; j < nextHopCount; j++) {
        newEntry->addNextHop(borderRouterEntry.getNextHop(j));
    }

    return newEntry;
}

// for IPv4 AF
OSPFv3IPv4RoutingTableEntry *OSPFv3Area::createRoutingTableEntryFromInterAreaPrefixLSA(const InterAreaPrefixLSA& interAreaPrefixLSA,
        unsigned short entryCost,
        const OSPFv3IPv4RoutingTableEntry& borderRouterEntry) const
{
    Ipv4AddressRange destination;
    destination.address = interAreaPrefixLSA.getPrefix().toIpv4();
    destination.mask = destination.address.makeNetmask(interAreaPrefixLSA.getPrefixLen());
    OSPFv3IPv4RoutingTableEntry *newEntry = new OSPFv3IPv4RoutingTableEntry(this->getInstance()->ift, destination.address, destination.mask.getNetmaskLength(),IRoute::OSPF);

    if (interAreaPrefixLSA.getHeader().getLsaType() == INTER_AREA_PREFIX_LSA)
        newEntry->setDestinationType(OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION);
    else
        newEntry->setDestinationType(OSPFv3IPv4RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION);

    newEntry->setLinkStateOrigin(&interAreaPrefixLSA);
    newEntry->setArea(areaID);
    newEntry->setPathType(OSPFv3IPv4RoutingTableEntry::INTERAREA);
    newEntry->setCost(entryCost);

    unsigned int nextHopCount = borderRouterEntry.getNextHopCount();
    for (unsigned int j = 0; j < nextHopCount; j++) {
        newEntry->addNextHop(borderRouterEntry.getNextHop(j));
    }

    return newEntry;
}

void OSPFv3Area::calculateInterAreaRoutes(std::vector<OSPFv3RoutingTableEntry* >& newTableIPv6, std::vector<OSPFv3IPv4RoutingTableEntry* >& newTableIPv4)
{
    EV_DEBUG << "Calculating Inter-Area Routes for Backbone\n";
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = interAreaPrefixLSAList.size();

    // go through all LSA 3
    for (i = 0; i < lsaCount; i++)
    {
        InterAreaPrefixLSA *currentLSA = interAreaPrefixLSAList[i];
        const OSPFv3LSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getMetric();
        unsigned short lsAge = currentHeader.getLsaAge();
        Ipv4Address originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == this->getInstance()->getProcess()->getRouterID());

       /* (1) If the cost specified by the LSA is LSInfinity, or if the
            LSA's LS age is equal to MaxAge, then examine the the next
            LSA.

        (2) If the LSA was originated by the calculating router itself,
            examine the next LSA.*/
        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }
        char lsType = currentHeader.getLsaType();

        // for IPv6 AF
        if (v6)
        {
            unsigned long routeCount = newTableIPv6.size();
            Ipv6AddressRange destination;

            // get Prefix and Prefix length from LSA 3
            destination.prefix = currentLSA->getPrefix().toIpv6();
            destination.prefixLength = currentLSA->getPrefixLen();

            if ((lsType == INTER_AREA_PREFIX_LSA) && (this->getInstance()->getProcess()->hasAddressRange(destination))) {    // (3)
                bool foundIntraAreaRoute = false;
                // look for an "Active" INTRA_AREA route
                for (j = 0; j < routeCount; j++) {
                    OSPFv3RoutingTableEntry *routingEntry = newTableIPv6[j];

                    if ((routingEntry->getDestinationType() == OSPFv3RoutingTableEntry::NETWORK_DESTINATION) &&
                        (routingEntry->getPathType() == OSPFv3RoutingTableEntry::INTRAAREA) &&
                        destination.containedByRange(routingEntry->getDestPrefix(), routingEntry->getPrefixLength()))
                    {
                        foundIntraAreaRoute = true;
                        break;
                    }
                }
                if (foundIntraAreaRoute) {
                    continue;
                }
            }

            OSPFv3RoutingTableEntry *borderRouterEntry = nullptr;
            LinkLSA* linkLSA  = nullptr;
            for(int iface = 0; iface < interfaceList.size(); iface++)
            {

                linkLSA = interfaceList[iface]->findLinkLSAbyAdvRouter(originatingRouter);
                if (linkLSA != nullptr)
                    break;
            }
            if (linkLSA != nullptr)
            {
                // The routingEntry describes a route to an other area -> look for the border router originating it
                for (j = 0; j < routeCount; j++)
                {    // (4) N == destination, BR == borderRouterEntry
                    OSPFv3RoutingTableEntry *routingEntry = newTableIPv6[j];

                    EV_DEBUG << "\n";
                    for (int pfxs = 0; pfxs < linkLSA->getPrefixesArraySize(); pfxs++)
                    {
                        const OSPFv3LSAHeader header = routingEntry->getLinkStateOrigin()->getHeader();

                    // ak mamu seba LSA 9 v ramci rovnakej area s IPckou aku obsahuje novy routingEntry zaznam, tak...
                        if ((routingEntry->getArea() == areaID) &&
                            (((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                             ((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                             (header.getAdvertisingRouter() == linkLSA->getHeader().getAdvertisingRouter()) &&
                            (routingEntry->getDestPrefix() == linkLSA->getPrefixes(pfxs).addressPrefix.toIpv6()))
                        {
                            borderRouterEntry = routingEntry;
                            break;
                        }
                    }
                    if (borderRouterEntry != nullptr)
                        break;
                }
            }
            else //in LinkLSA found nothing, check Intra-Area-Prefix-LSA
            {
                //find Router LSA for originate router of Inter Area Prefix LSA
                RouterLSA *routerLSA =  findRouterLSA(originatingRouter);

                if (routerLSA == nullptr) {
                    continue;
                }
                //if founded RouterLSA has no valuable information.
                if (routerLSA->getRoutersArraySize() < 1) {
                    continue;
                }
                for (int ro = 0; ro <  routerLSA->getRoutersArraySize(); ro++)
                {
                    LSAKeyType lsaKey;
                    lsaKey.advertisingRouter = routerLSA->getRouters(ro).neighborRouterID;
                    lsaKey.linkStateID = Ipv4Address(routerLSA->getRouters(ro).neighborInterfaceID);
                    lsaKey.LSType = routerLSA->getRouters(ro).type;

                    OSPFv3IntraAreaPrefixLSA *iapLSA = findIntraAreaPrefixLSAByReference(lsaKey);
                    if (iapLSA == nullptr)
                        continue;
                    // The routingEntry describes a route to an other area -> look for the border router originating it
                    for (j = 0; j < routeCount; j++)
                    {    // (4) N == destination, BR == borderRouterEntry
                        OSPFv3RoutingTableEntry *routingEntry = newTableIPv6[j];

                        EV_DEBUG << "\n";
                        for (int pfxs = 0; pfxs < iapLSA->getPrefixesArraySize(); pfxs++)
                        {
                            const OSPFv3LSAHeader header = routingEntry->getLinkStateOrigin()->getHeader();

                        // ak mamu seba LSA 9 v ramci rovnakej area s IPckou aku obsahuje novy routingEntry zaznam, tak...
                            if ((routingEntry->getArea() == areaID) &&
                                (((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                                 ((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                                 (header.getAdvertisingRouter() == iapLSA->getHeader().getAdvertisingRouter()) &&
                                (routingEntry->getDestPrefix() == iapLSA->getPrefixes(pfxs).addressPrefix.toIpv6()))
                            {
                                borderRouterEntry = routingEntry;
                                break;
                            }
                        }
                        if (borderRouterEntry != nullptr)
                            break;
                    }
                    if (borderRouterEntry != nullptr)
                        break;
                }
            }

            if (borderRouterEntry == nullptr) {
                continue;
            }
            else
            {    // (5)
                /* "Else, this LSA describes an inter-area path to destination N,
                 * whose cost is the distance to BR plus the cost specified in the LSA.
                 * Call the cost of this inter-area path IAC."
                 */
                bool destinationInRoutingTable = true;
                EV_DEBUG <<"\n";
                unsigned short currentCost = routeCost + borderRouterEntry->getCost();
                std::list<OSPFv3RoutingTableEntry *> sameOrWorseCost;

                if (findSameOrWorseCostRoute(newTableIPv6,
                            *currentLSA,
                            currentCost,
                            destinationInRoutingTable,
                            sameOrWorseCost))
                {
                    continue;
                }

                if (destinationInRoutingTable && (sameOrWorseCost.size() > 0)) {
                    OSPFv3RoutingTableEntry *equalEntry = nullptr;

                    /* Look for an equal cost entry in the sameOrWorseCost list, and
                     * also clear the more expensive entries from the newTable.
                     */
                    for (auto checkedEntry : sameOrWorseCost) {
                        if (checkedEntry->getCost() > currentCost) {
                            for (auto entryIt = newTableIPv6.begin(); entryIt != newTableIPv6.end(); entryIt++) {
                                if (checkedEntry == (*entryIt)) {
                                    newTableIPv6.erase(entryIt);
                                    break;
                                }
                            }
                        }
                        else {    // EntryCost == currentCost
                            equalEntry = checkedEntry;    // should be only one - if there are more they are ignored
                        }
                    }
                    unsigned long nextHopCount = borderRouterEntry->getNextHopCount();
                    if (equalEntry != nullptr) {
                        /* Add the next hops of the border router advertising this destination
                         * to the equal entry.
                         */
                        for (unsigned long j = 0; j < nextHopCount; j++) {
                            equalEntry->addNextHop(borderRouterEntry->getNextHop(j));
                        }
                    }
                    else {
                        OSPFv3RoutingTableEntry *newEntry = createRoutingTableEntryFromInterAreaPrefixLSA(*currentLSA, currentCost, *borderRouterEntry);
                        ASSERT(newEntry != nullptr);
                        newTableIPv6.push_back(newEntry);
                    }
                }
                else {
                    OSPFv3RoutingTableEntry *newEntry = createRoutingTableEntryFromInterAreaPrefixLSA(*currentLSA, currentCost, *borderRouterEntry);
                    ASSERT(newEntry != nullptr);
                    newTableIPv6.push_back(newEntry);
                }
            }
        } // end of if AF ipv6
        else    // IPv4 AF
        {
            unsigned long routeCount = newTableIPv4.size();
            Ipv4AddressRange destination;

            // get Prefix and Prefix length from LSA 3
            destination.address = currentLSA->getPrefix().toIpv4();
            destination.mask = destination.address.makeNetmask(currentLSA->getPrefixLen());

            if ((lsType == INTER_AREA_PREFIX_LSA) && (this->getInstance()->getProcess()->hasAddressRange(destination))) {    // (3)
                bool foundIntraAreaRoute = false;
                // look for an "Active" INTRAAREA route
                for (j = 0; j < routeCount; j++) {
                    OSPFv3IPv4RoutingTableEntry *routingEntry = newTableIPv4[j];

                    if ((routingEntry->getDestinationType() == OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION) &&
                        (routingEntry->getPathType() == OSPFv3IPv4RoutingTableEntry::INTRAAREA) &&
                        destination.containedByRange(routingEntry->getDestination(), routingEntry->getNetmask()))
                    {
                        foundIntraAreaRoute = true;
                        break;
                    }
                }
                if (foundIntraAreaRoute) {
                    continue;
                }
            }

            OSPFv3IPv4RoutingTableEntry *borderRouterEntry = nullptr;
            //find Router LSA for originate router of Inter Area Prefix LSA
            RouterLSA *routerLSA =  findRouterLSA(originatingRouter);

            //if founded RouterLSA has no valuable information.
            if (routerLSA == nullptr) {
                continue;
            }
            if (routerLSA->getRoutersArraySize() < 1) {
                continue;
            }
            //from Router LSA routers search for Intra Area Prefix LSA
            OSPFv3IntraAreaPrefixLSA *iapLSA = nullptr;
            for (int rIndex = 0; rIndex < routerLSA->getRoutersArraySize(); rIndex++)
            {
                LSAKeyType lsaKey;
                lsaKey.linkStateID = (Ipv4Address)routerLSA->getRouters(rIndex).neighborInterfaceID;
                lsaKey.advertisingRouter = routerLSA->getRouters(rIndex).neighborRouterID;
                lsaKey.LSType = NETWORK_LSA;

                iapLSA = findIntraAreaPrefixLSAByReference(lsaKey);
                if (iapLSA == nullptr)
                    continue;
            }


            if (iapLSA == nullptr)
                continue;

            // The routingEntry describes a route to an other area -> look for the border router originating it
            for (j = 0; j < routeCount; j++) {    // (4) N == destination, BR == borderRouterEntry
                OSPFv3IPv4RoutingTableEntry *routingEntry = newTableIPv4[j];
                for (int pfxs = 0; pfxs < iapLSA->getPrefixesArraySize(); pfxs++)
                {
                    // if I have LSA type 9 in same area with IP same as in new calculated routingEntry, then...
                    if ((routingEntry->getArea() == areaID) &&
                        (((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                         ((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                        (routingEntry->getDestination() == iapLSA->getPrefixes(pfxs).addressPrefix.toIpv4()))
                    {
                        borderRouterEntry = routingEntry;
                        break;
                    }
                }
                if (borderRouterEntry != nullptr)
                    break;
            }

            if (borderRouterEntry == nullptr) {
                continue;
            }
            else
            {    // (5)
                /* "Else, this LSA describes an inter-area path to destination N,
                 * whose cost is the distance to BR plus the cost specified in the LSA.
                 * Call the cost of this inter-area path IAC."
                 */
                bool destinationInRoutingTable = true;
                unsigned short currentCost = routeCost + borderRouterEntry->getCost();
                std::list<OSPFv3IPv4RoutingTableEntry *> sameOrWorseCost;

                if (findSameOrWorseCostRoute(newTableIPv4,
                            *currentLSA,
                            currentCost,
                            destinationInRoutingTable,
                            sameOrWorseCost))
                {
                    continue;
                }

                if (destinationInRoutingTable && (sameOrWorseCost.size() > 0)) {
                    OSPFv3IPv4RoutingTableEntry *equalEntry = nullptr;

                    /* Look for an equal cost entry in the sameOrWorseCost list, and
                     * also clear the more expensive entries from the newTable.
                     */
                    for (auto checkedEntry : sameOrWorseCost) {
                        if (checkedEntry->getCost() > currentCost) {
                            for (auto entryIt = newTableIPv4.begin(); entryIt != newTableIPv4.end(); entryIt++) {
                                if (checkedEntry == (*entryIt)) {
                                    newTableIPv4.erase(entryIt);
                                    break;
                                }
                            }
                        }
                        else {    // EntryCost == currentCost
                            equalEntry = checkedEntry;    // should be only one - if there are more they are ignored
                        }
                    }
                    unsigned long nextHopCount = borderRouterEntry->getNextHopCount();

                    if (equalEntry != nullptr) {
                        /* Add the next hops of the border router advertising this destination
                         * to the equal entry.
                         */
                        for (unsigned long j = 0; j < nextHopCount; j++) {
                            equalEntry->addNextHop(borderRouterEntry->getNextHop(j));
                        }
                    }
                    else {
                        OSPFv3IPv4RoutingTableEntry *newEntry = createRoutingTableEntryFromInterAreaPrefixLSA(*currentLSA, currentCost, *borderRouterEntry);
                        ASSERT(newEntry != nullptr);
                        newTableIPv4.push_back(newEntry);
                    }
                }
                else {
                    OSPFv3IPv4RoutingTableEntry *newEntry = createRoutingTableEntryFromInterAreaPrefixLSA(*currentLSA, currentCost, *borderRouterEntry);
                    ASSERT(newEntry != nullptr);
                    newTableIPv4.push_back(newEntry);
                }
            }
        }
    }
}

// chcek if router has link into calculating router through LSAs
bool OSPFv3Area::hasLink(OSPFv3LSA *fromLSA, OSPFv3LSA *toLSA) const
{
    unsigned int i;
    RouterLSA *fromRouterLSA = dynamic_cast<RouterLSA *>(fromLSA);
    if (fromRouterLSA != nullptr) {
        unsigned int linkCount = fromRouterLSA->getRoutersArraySize();
        RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(toLSA);
        if (toRouterLSA != nullptr) {
            for (i = 0; i < linkCount; i++) {
                OSPFv3RouterLSABody& link = fromRouterLSA->getRoutersForUpdate(i);
                OSPFv3RouterLSAType linkType = static_cast<OSPFv3RouterLSAType>(link.type);

                if (((linkType == POINT_TO_POINT) ||
                     (linkType == VIRTUAL_LINK)) &&
                    (link.neighborRouterID == toRouterLSA->getHeader().getAdvertisingRouter()))
                {
                    return true;
                }
            }
        }
        else {
            NetworkLSA *toNetworkLSA = dynamic_cast<NetworkLSA *>(toLSA);
            if (toNetworkLSA != nullptr) {
                for (i = 0; i < linkCount; i++) {
                    OSPFv3RouterLSABody& link = fromRouterLSA->getRoutersForUpdate(i);

                    if ((link.type == TRANSIT_NETWORK) &&
                        (link.neighborRouterID == toNetworkLSA->getHeader().getAdvertisingRouter()))
                    {
                        return true;
                    }
                    if (link.type == RESERVED)
                    {
                        return true;
                    }
                }
            }
        }
    }
    else {
        NetworkLSA *fromNetworkLSA = dynamic_cast<NetworkLSA *>(fromLSA);
        if (fromNetworkLSA != nullptr) {
            unsigned int routerCount = fromNetworkLSA->getAttachedRouterArraySize();
            RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(toLSA);
            if (toRouterLSA != nullptr) {
                for (i = 0; i < routerCount; i++) {
                    if (fromNetworkLSA->getAttachedRouter(i) == toRouterLSA->getHeader().getAdvertisingRouter()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool OSPFv3Area::nextHopAlreadyExists(std::vector<NextHop> *hops, NextHop nextHop) const
{

    for (int i=0; i < hops->size(); i++)
    {
        if ((*hops)[i].advertisingRouter == nextHop.advertisingRouter &&
                       (*hops)[i].hopAddress == nextHop.hopAddress &&
                       (*hops)[i].ifIndex == nextHop.ifIndex)
            return true;
    }
    return false;
}

std::vector<NextHop> *OSPFv3Area::calculateNextHops(OSPFv3LSA *destination, OSPFv3LSA *parent) const
{
    EV_DEBUG << "Calculating Next Hops\n";
    std::vector<NextHop> *hops = new std::vector<NextHop>;
    unsigned long i, j;
    RouterLSA *routerLSA = dynamic_cast<RouterLSA *>(parent);
    if (routerLSA != nullptr) { // if parrent is ROUTER_LSA
        if (routerLSA != spfTreeRoot) {
            unsigned int nextHopCount = routerLSA->getNextHopCount();
            for (i = 0; i < nextHopCount; i++) {
                if (!this->nextHopAlreadyExists(hops, routerLSA->getNextHop(i)))
                    hops->push_back(routerLSA->getNextHop(i));
            }
            return hops;
        }
        else {
            RouterLSA *destinationRouterLSA = dynamic_cast<RouterLSA *>(destination);
            if (destinationRouterLSA != nullptr) {  // if destination is ROUTER_LSA
                unsigned long interfaceNum = interfaceList.size();
                for (i = 0; i < interfaceNum; i++) {
                    OSPFv3Interface::OSPFv3InterfaceType intfType = interfaceList[i]->getType();

                    // if its P2P interface, set one neighbor and if it is destination too, set one nexthop and store it into hosts
                    if ((intfType == OSPFv3Interface::POINTTOPOINT_TYPE) ||
                        ((intfType == OSPFv3Interface::VIRTUAL_TYPE) &&
                         (interfaceList[i]->getState() > OSPFv3Interface::INTERFACE_STATE_LOOPBACK)))
                    {

                        OSPFv3Neighbor *ptpNeighbor = interfaceList[i]->getNeighborCount() > 0 ? interfaceList[i]->getNeighbor(0) : nullptr;
                       // by neighbor find appropriate Link LSA
                       if (ptpNeighbor != nullptr)
                       {
                           NextHop nextHop;
                           LSAKeyType lsaKey;
                           lsaKey.linkStateID = Ipv4Address(ptpNeighbor->getNeighborInterfaceID());
                           lsaKey.advertisingRouter = ptpNeighbor->getNeighborID();
                           lsaKey.LSType = LINK_LSA;
                           LinkLSA* linklsa = interfaceList[i]->getLinkLSAbyKey(lsaKey);

                           if (linklsa != nullptr)
                           {
                               nextHop.ifIndex = interfaceList[i]->getInterfaceId();
                               nextHop.hopAddress = linklsa->getLinkLocalInterfaceAdd();
                               nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter();

                               if (!this->nextHopAlreadyExists(hops, nextHop))
                                   hops->push_back(nextHop);

                               break;
                           }
                       }
                    }
                    if (intfType == OSPFv3Interface::POINTTOMULTIPOINT_TYPE) {
                        EV_DEBUG << "P2MP in in next hop calculation not implemented yes\n"; //TODO
                    }
                } // for ()
            }
            else {          //  else destination is NETWORK_LSA
                NetworkLSA *destinationNetworkLSA = dynamic_cast<NetworkLSA *>(destination);
                if (destinationNetworkLSA != nullptr) {
                    Ipv4Address networkDesignatedRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter();
                    Ipv4Address networkDRintID = destinationNetworkLSA->getHeader().getLinkStateID();
                    unsigned long interfaceNum = interfaceList.size();
                    for (i = 0; i < interfaceNum; i++)
                    {
                        OSPFv3Interface::OSPFv3InterfaceType intfType = interfaceList[i]->getType();
                        if (((intfType == OSPFv3Interface::BROADCAST_TYPE) ||
                             (intfType == OSPFv3Interface::NBMA_TYPE)) &&
                            (interfaceList[i]->getDesignatedID() == networkDesignatedRouter)
                            && ((Ipv4Address)interfaceList[i]->getDesignatedIntID() == networkDRintID))
                        {
                            NextHop nextHop;
                            nextHop.ifIndex = interfaceList[i]->getInterfaceId();
                            if (v6)
                                nextHop.hopAddress = Ipv6Address::UNSPECIFIED_ADDRESS;
                            else
                                nextHop.hopAddress = Ipv4Address::UNSPECIFIED_ADDRESS;
                            nextHop.advertisingRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter();
                            if (!this->nextHopAlreadyExists(hops, nextHop))
                                  hops->push_back(nextHop);
                        }
                    }
                }
            }
        }

    }
    else { // if parent is NETWORK_LSA
        NetworkLSA *networkLSA = dynamic_cast<NetworkLSA *>(parent);
        if (networkLSA != nullptr)
        {
           if (networkLSA->getParent() != spfTreeRoot) { //ak som network a moj parent nie je spfTreeRoot, vrat v�etky nexthopy
               unsigned int nextHopCount = networkLSA->getNextHopCount();
               for (i = 0; i < nextHopCount; i++) {
                   if (!this->nextHopAlreadyExists(hops, networkLSA->getNextHop(i)))
                       hops->push_back(networkLSA->getNextHop(i));
               }
               return hops;
           }
           else
           {
               // for Network-LSA, Link State ID is ID of interface by which it is connected into network
               Ipv4Address parentLinkStateID = parent->getHeader().getAdvertisingRouter();

               RouterLSA *destinationRouterLSA = dynamic_cast<RouterLSA *>(destination);
               if (destinationRouterLSA != nullptr) {
                   const Ipv4Address& destinationRouterID = destinationRouterLSA->getHeaderForUpdate().getLinkStateID();
                   unsigned int linkCount = destinationRouterLSA->getRoutersArraySize();
                   for (i = 0; i < linkCount; i++) {
                       OSPFv3RouterLSABody& link = destinationRouterLSA->getRoutersForUpdate(i);
                       NextHop nextHop;

                       if (((link.type == TRANSIT_NETWORK) &&
                            (link.neighborRouterID == parentLinkStateID))
//                               ||  ((link.getType() == STUB_LINK) &&
//                            ((link.getLinkID() & Ipv4Address(link.getLinkData())) == (parentLinkStateID & networkLSA->getNetworkMask())))
                            )
                       {
                           unsigned long interfaceNum = interfaceList.size();
                           for (j = 0; j < interfaceNum; j++) {
                               OSPFv3Interface::OSPFv3InterfaceType intfType = interfaceList[j]->getType();
                               if (((intfType == OSPFv3Interface::BROADCAST_TYPE) ||
                                    (intfType == OSPFv3Interface::NBMA_TYPE)) &&
                                   (interfaceList[j]->getDesignatedID() == parentLinkStateID))
                               {
                                   OSPFv3Neighbor *nextHopNeighbor = interfaceList[j]->getNeighborById(destinationRouterID);

                                   // by neighbor find appropriate Link LSA
                                   if (nextHopNeighbor != nullptr)
                                   {
                                        LSAKeyType lsaKey;
                                        lsaKey.linkStateID = Ipv4Address(nextHopNeighbor->getNeighborInterfaceID());
                                        lsaKey.advertisingRouter = nextHopNeighbor->getNeighborID();
                                        lsaKey.LSType = LINK_LSA;

                                        LinkLSA* linklsa = interfaceList[j]->getLinkLSAbyKey(lsaKey);

                                        if (linklsa != nullptr)
                                        {
                                            nextHop.ifIndex = interfaceList[j]->getInterfaceId();
                                            nextHop.hopAddress = linklsa->getLinkLocalInterfaceAdd();
                                            nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter();
                                            if (!this->nextHopAlreadyExists(hops, nextHop))
                                                hops->push_back(nextHop);
                                        }
                                   }
                               }
                           }
                       }
                   }
               }
               // else Destination is Router - should not be possible
           }
        }
    }
    return hops;
}

void OSPFv3Area::recheckInterAreaPrefixLSAs( std::vector<OSPFv3RoutingTableEntry* >& newTableIPv6)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = interAreaPrefixLSAList.size();

    for (i = 0; i < lsaCount; i++)
    {
        InterAreaPrefixLSA *currentLSA = interAreaPrefixLSAList[i];
        const OSPFv3LSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getMetric();
        unsigned short lsAge = currentHeader.getLsaAge();
        Ipv4Address originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == this->getInstance()->getProcess()->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }

        unsigned long routeCount = newTableIPv6.size();
        char lsaType = currentHeader.getLsaType();
        OSPFv3RoutingTableEntry *destinationEntry = nullptr;
        Ipv6AddressRange destination;

        destination.prefix = currentLSA->getPrefix().toIpv6(); //from LSA type 3
        destination.prefixLength = currentLSA->getPrefixLen();

        for (j = 0; j < routeCount; j++) {    // (3)
            OSPFv3RoutingTableEntry *routingEntry = newTableIPv6[j];
            bool foundMatching = false;

            if (lsaType == INTER_AREA_PREFIX_LSA) {
                if ((routingEntry->getDestinationType() == OSPFv3RoutingTableEntry::NETWORK_DESTINATION) &&
                    (destination.prefix == routingEntry->getDestPrefix()) &&
                    (destination.prefixLength == routingEntry->getPrefixLength()))
                {
                    foundMatching = true;
                }
            }
            else {
                if (
                    (
                            ((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                            ((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)
                    ) &&
                    (destination.prefix == routingEntry->getDestPrefix()) &&
                    (destination.prefixLength == routingEntry->getPrefixLength())
                    )
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                OSPFv3RoutingTableEntry::RoutingPathType pathType = routingEntry->getPathType();

                if ((pathType == OSPFv3RoutingTableEntry::TYPE1_EXTERNAL) ||
                    (pathType == OSPFv3RoutingTableEntry::TYPE2_EXTERNAL) ||
                    (routingEntry->getArea() != BACKBONE_AREAID))
                {
                    break;
                }
                else {
                    destinationEntry = routingEntry;
                    break;
                }
            }
        }
        if (destinationEntry == nullptr) {
            continue;
        }

        OSPFv3RoutingTableEntry *borderRouterEntry = nullptr;
        unsigned short currentCost = routeCost;

        RouterLSA *routerLSA =  findRouterLSA(originatingRouter);

        //if founded RouterLSA has no valuable information.
        if (routerLSA == nullptr) {
           continue;
        }
        if (routerLSA->getRoutersArraySize() < 1) {
           continue;
        }
        //from Router LSA routers search for Intra Area Prefix LSA
        OSPFv3IntraAreaPrefixLSA *iapLSA  = nullptr;
        for (int rIndex = 0; rIndex < routerLSA->getRoutersArraySize(); rIndex++)
        {
            LSAKeyType lsaKey;
            lsaKey.linkStateID = (Ipv4Address)routerLSA->getRouters(rIndex).neighborInterfaceID; //TODO : what if it is another link for network
            lsaKey.advertisingRouter = routerLSA->getRouters(rIndex).neighborRouterID;
            lsaKey.LSType = routerLSA->getRouters(rIndex).type;

            iapLSA = findIntraAreaPrefixLSAByReference(lsaKey);
            if (iapLSA == nullptr)
               continue;
        }


        if (iapLSA == nullptr)
           continue;

       for (j = 0; j < routeCount; j++) {    // (4) BR == borderRouterEntry
            OSPFv3RoutingTableEntry *routingEntry = newTableIPv6[j];
            for (int pfxs = 0; pfxs < iapLSA->getPrefixesArraySize(); pfxs++)
            {
                if ((routingEntry->getArea() == areaID) &&
                    (((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                     ((routingEntry->getDestinationType() & OSPFv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                    (routingEntry->getDestPrefix() == iapLSA->getPrefixes(pfxs).addressPrefix.toIpv6())) // find out, whether destination is router who originated this LSA type 3
                {
                    borderRouterEntry = routingEntry;;
                    currentCost += borderRouterEntry->getCost();
                    break;
                }
            }
        }
        if (borderRouterEntry == nullptr) {
            continue;
        }
        else {    // (5)
            if (currentCost <= destinationEntry->getCost()) {
                if (currentCost < destinationEntry->getCost()) {
                    destinationEntry->clearNextHops();
                }

                unsigned long nextHopCount = borderRouterEntry->getNextHopCount();

                for (j = 0; j < nextHopCount; j++) {
                    destinationEntry->addNextHop(borderRouterEntry->getNextHop(j));
                }
            }
        }
    }
}



void OSPFv3Area::recheckInterAreaPrefixLSAs( std::vector<OSPFv3IPv4RoutingTableEntry* >& newTableIPv4)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = interAreaPrefixLSAList.size();

    for (i = 0; i < lsaCount; i++)
    {
        InterAreaPrefixLSA *currentLSA = interAreaPrefixLSAList[i];
        const OSPFv3LSAHeader& currentHeader = currentLSA->getHeader();

        unsigned long routeCost = currentLSA->getMetric();
        unsigned short lsAge = currentHeader.getLsaAge();
        Ipv4Address originatingRouter = currentHeader.getAdvertisingRouter();
        bool selfOriginated = (originatingRouter == this->getInstance()->getProcess()->getRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) {    // (1) and(2)
            continue;
        }

        unsigned long routeCount = newTableIPv4.size();
        char lsaType = currentHeader.getLsaType();
        OSPFv3IPv4RoutingTableEntry *destinationEntry = nullptr;
        Ipv4AddressRange destination;

        destination.address = currentLSA->getPrefix().toIpv4(); //from LSA type 3
        destination.mask = destination.address.makeNetmask(currentLSA->getPrefixLen());

        for (j = 0; j < routeCount; j++) {    // (3)
            OSPFv3IPv4RoutingTableEntry *routingEntry = newTableIPv4[j];
            bool foundMatching = false;

            if (lsaType == INTER_AREA_PREFIX_LSA) {
                if ((routingEntry->getDestinationType() == OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION) &&
                    (destination.address == routingEntry->getDestination()) &&
                    (destination.mask == routingEntry->getDestination().makeNetmask(routingEntry->getPrefixLength())))
                {
                    foundMatching = true;
                }
            }
            else
            {
                if (
                    (
                            ((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                            ((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)
                    ) &&
                    (destination.address == routingEntry->getDestination()) &&
                    (destination.mask == routingEntry->getDestination().makeNetmask(routingEntry->getPrefixLength()))
                    )
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                OSPFv3IPv4RoutingTableEntry::RoutingPathType pathType = routingEntry->getPathType();
                if ((pathType == OSPFv3IPv4RoutingTableEntry::TYPE1_EXTERNAL) ||
                    (pathType == OSPFv3IPv4RoutingTableEntry::TYPE2_EXTERNAL) ||
                    (routingEntry->getArea() != BACKBONE_AREAID))
                {
                    break;
                }
                else {
                    destinationEntry = routingEntry;
                    break;
                }
            }
        }
        if (destinationEntry == nullptr) {
            continue;
        }

        OSPFv3IPv4RoutingTableEntry *borderRouterEntry = nullptr;
        unsigned short currentCost = routeCost;

        RouterLSA *routerLSA =  findRouterLSA(originatingRouter);

        //if founded RouterLSA has no valuable information.
        if (routerLSA == nullptr) {
           continue;
        }
        if (routerLSA->getRoutersArraySize() < 1) {
           continue;
        }
        //from Router LSA routers search for Intra Area Prefix LSA
        OSPFv3IntraAreaPrefixLSA *iapLSA  = nullptr;
        for (int rIndex = 0; rIndex < routerLSA->getRoutersArraySize(); rIndex++)
        {
            LSAKeyType lsaKey;
            lsaKey.linkStateID = (Ipv4Address)routerLSA->getRouters(rIndex).neighborInterfaceID;
            lsaKey.advertisingRouter = routerLSA->getRouters(rIndex).neighborRouterID;
            lsaKey.LSType = routerLSA->getRouters(rIndex).type;

            iapLSA = findIntraAreaPrefixLSAByReference(lsaKey);
            if (iapLSA == nullptr)
                continue;
        }


        if (iapLSA == nullptr)
            continue;

       for (j = 0; j < routeCount; j++) {    // (4) BR == borderRouterEntry
           OSPFv3IPv4RoutingTableEntry *routingEntry = newTableIPv4[j];
            for (int pfxs = 0; pfxs < iapLSA->getPrefixesArraySize(); pfxs++)
{

                if ((routingEntry->getArea() == areaID) &&
                    (((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) ||
                     ((routingEntry->getDestinationType() & OSPFv3IPv4RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0)) &&
                    (routingEntry->getDestination() == iapLSA->getPrefixes(pfxs).addressPrefix.toIpv4())) // zistujem ci destination je router, ktory tuto LSA 3 spravu vytvoril
                {
                    borderRouterEntry = routingEntry;;
                    currentCost += borderRouterEntry->getCost();
                    break;
                }
            }
        }
        if (borderRouterEntry == nullptr) {
            continue;
        }
        else {    // (5)
            if (currentCost <= destinationEntry->getCost()) {
                if (currentCost < destinationEntry->getCost()) {
                    destinationEntry->clearNextHops();
                }

                unsigned long nextHopCount = borderRouterEntry->getNextHopCount();

                for (j = 0; j < nextHopCount; j++) {
                    destinationEntry->addNextHop(borderRouterEntry->getNextHop(j));
                }
            }
        }
    }
}


void OSPFv3Area::debugDump()
{
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
        EV_DEBUG << "\t\tinterface id: " << (*it)->getIntName() << "\n";

}//debugDump

std::string OSPFv3Area::detailedInfo() const
{//TODO - adjust so that it pnly prints LSAs that are there
    std::stringstream out;

    out << "OSPFv3 1 address-family ";
    if(this->getInstance()->getAddressFamily()==IPV4INSTANCE)
        out << "ipv4 (router-id ";
    else
        out << "ipv6 (router-id ";

    out << this->getInstance()->getProcess()->getRouterID();
    out << ")\n\n";

    if(this->routerLSAList.size()>0) {
        out << "Router Link States (Area " << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tFragment ID\tLink count\tBits\n";
        for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++) {
            OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
            bool bitsEmpty = true;
            out << header.getAdvertisingRouter()<<"\t\t";
            out << header.getLsaAge() <<"\t0x" << std::hex << header.getLsaSequenceNumber() << std::dec<<"\t0\t\t";
            out << (*it)->getRoutersArraySize()<< "\t\t"; //link Count
            if((*it)->getNtBit()) {
                out << "Nt ";
                bitsEmpty = false;
            }
            if((*it)->getXBit()){
                out << "x ";
                bitsEmpty = false;
            }
            if((*it)->getVBit()){
                out << "V ";
                bitsEmpty = false;
            }
            if((*it)->getEBit()){
                out << "E ";
                bitsEmpty = false;
            }
            if((*it)->getBBit()){
                out << "B";
                bitsEmpty = false;
            }
            if(bitsEmpty)
                out << "None";

            out << endl;
        }
    }

    if(this->networkLSAList.size()>0) {
        out << "\nNet Link States (Area " << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tLink State ID\tRtr count\n";
        for(auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++) {
            OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
            out << header.getAdvertisingRouter()<<"\t\t";
            out << header.getLsaAge()<<"\t0x" << std::hex << header.getLsaSequenceNumber() << std::dec <<"\t"<<header.getLinkStateID().str(false)<<"\t\t" << (*it)->getAttachedRouterArraySize() << "\n";
        }
    }

    if(this->interAreaPrefixLSAList.size()>0) {
        out << "\nInter Area Prefix Link States (Area " << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tPrefix\n";
        for(auto it = this->interAreaPrefixLSAList.begin(); it != this->interAreaPrefixLSAList.end(); it++){
            OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
            out << header.getAdvertisingRouter()<<"\t\t";
            out << header.getLsaAge()<<"\t0x" << std::hex << header.getLsaSequenceNumber() << std::dec <<"\t";

            L3Address addrPref = (*it)->getPrefix();
            if(this->getInstance()->getAddressFamily()==IPV4INSTANCE) {
                out << (*it)->getPrefix().str() << "/" << (int)(*it)->getPrefixLen() << endl;
            }
            else if(this->getInstance()->getAddressFamily()==IPV6INSTANCE) {
                Ipv6Address ipv6addr = addrPref.toIpv6();
                ipv6addr = ipv6addr.getPrefix((*it)->getPrefixLen());
                if(ipv6addr == Ipv6Address::UNSPECIFIED_ADDRESS)
                    out << "::/0" << endl;
                else
                    out << ipv6addr.str() << "/" << (int)(*it)->getPrefixLen() << endl;
            }
        }
    }

    out << "\nLink (Type-8) Link States (Area " << this->getAreaID().str(false) << ")\n" ;
    out << "ADV Router\tAge\tSeq#\t\tLink State ID\tInterface\n";
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++) {
        int linkLSACount = (*it)->getLinkLSACount();
        for(int i = 0; i<linkLSACount; i++) {
            OSPFv3LSAHeader& header = (*it)->getLinkLSA(i)->getHeaderForUpdate();
            out << header.getAdvertisingRouter()<<"\t\t";
            out << header.getLsaAge()<<"\t0x" << std::hex << header.getLsaSequenceNumber() << std::dec <<"\t"<<header.getLinkStateID().str(false)<<"\t\t"<< (*it)->getIntName() << "\n";
        }
    }

    if(this->intraAreaPrefixLSAList.size()>0) {
        out << "\nIntra Area Prefix Link States (Area" << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tLink ID\t\tRef-lstype\tRef-LSID\n";
        for(auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++) {
            OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
            out << header.getAdvertisingRouter() << "\t\t";
            if ((*it)->getReferencedLSType()  == 2)
                out << header.getLsaAge() << "\t0x" << std::hex << header.getLsaSequenceNumber() << std::dec << "\t" << header.getLinkStateID().str(false) << "\t\t0x200" << (*it)->getReferencedLSType() << "\t\t" << (*it)->getReferencedLSID().str(false)<<"\n\n";
            else
                out << header.getLsaAge() << "\t0x" << std::hex << header.getLsaSequenceNumber() << std::dec << "\t" << header.getLinkStateID().str(false) << "\t\t0x200" << (*it)->getReferencedLSType() << "\t\t0\n\n";
        }
    }

    // out stream with detail for every LSA (for Debug)
   /* out << "ROUTER LSA LIST .size = " <<  routerLSAList.size() << "\n";
    for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++) {
        OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
        out << "AdvertisingRouter =\t" << header.getAdvertisingRouter()<< endl;
        out << "LinkStateID =\t\t" << header.getLinkStateID() << endl;
        out << "Age = \t\t\t" << header.getLsaAge() << endl;
        out << "type\t\tinterfaceID\t\tneighborIntID\t\tneighborRouterID\n";
        for (int i = 0; i < (*it)->getRoutersArraySize(); i++)
           out << (int)(*it)->getRouters(i).type << "\t\t" << (*it)->getRouters(i).interfaceID << "\t\t\t" << (*it)->getRouters(i).neighborInterfaceID << "\t\t\t" << (*it)->getRouters(i).neighborRouterID << "\n";
        out << endl;
    }

    out << endl;

    out <<  "NETWORK LSA LIST .size() = " << networkLSAList.size() << endl;
    for (auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++) {
        OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
        out << "AdvertisingRouter =\t" << header.getAdvertisingRouter()<< endl;
        out << "LinkStateID =\t\t" << header.getLinkStateID() << endl;
        out << "Age = \t\t\t" << header.getLsaAge() << endl;
        out << "Attached Router:" << endl;
        for (int i = 0; i < (*it)->getAttachedRouterArraySize(); i++)
            out << (*it)->getAttachedRouter(i) << endl;
    }
    out << endl;

    for (int  itf = 0; itf < this->getInterfaceCount() ; itf++ )
    {
        OSPFv3Interface *iface = this->getInterface(itf);
        out << "IFACE = " << iface->getIntName() << "\nLINK LSA LIST = " << iface->getLinkLSACount() << "\n";
        for (int ix = 0; ix <  iface->getLinkLSACount(); ix++)
        {
            OSPFv3LSAHeader& header = iface->getLinkLSA(ix)->getHeaderForUpdate();
            out << "AdvertisingRouter =\t" << header.getAdvertisingRouter()<< "\n";
            out << "LinkStateID =\t\t" << header.getLinkStateID() << "\n";
            out << "Age = \t\t\t" << header.getLsaAge() << endl;

            out << "link-local add = " << iface->getLinkLSA(ix)->getLinkLocalInterfaceAdd() << "\n";
            out << "prefixes = " <<  iface->getLinkLSA(ix)->getPrefixesArraySize() << "\n";

            for (int d = 0 ; d < iface->getLinkLSA(ix)->getPrefixesArraySize(); d++)
            {
                if (iface->getLinkLSA(ix)->getPrefixes(d).addressPrefix.getType() == L3Address::IPv4) // je to ipv6
                {
                    out << "\t" <<iface->getLinkLSA(ix)->getPrefixes(d).addressPrefix.toIpv4().str() << "/" << (int)iface->getLinkLSA(ix)->getPrefixes(d).prefixLen << "\n";

                }
                else
                {
                    out << "\t" << iface->getLinkLSA(ix)->getPrefixes(d).addressPrefix.toIpv6().str() << "/" << (int) iface->getLinkLSA(ix)->getPrefixes(d).prefixLen << "\n";

                }
            }
            out << "\n";
        }
    }

    out << "INTER AREA LSA LIST = " << interAreaPrefixLSAList.size() << "\n";
    for (auto it=this->interAreaPrefixLSAList.begin(); it!=this->interAreaPrefixLSAList.end(); it++) {
       OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
       out << "AdvertisingRouter =\t" << header.getAdvertisingRouter()<< "\n";
       out << "LinkStateID =\t\t" << header.getLinkStateID() << "\n";
       out << "Age = \t\t\t" << header.getLsaAge() << endl;
       out << "prefix =\t\t " << (*it)->getPrefix() << "\n";
       out << "prefixLen =\t\t " << (int)(*it)->getPrefixLen() << "\n";
       out << "metric =\t\t " << (int)(*it)->getMetric() << "\n";
    }
    out << endl;
    out <<  "INTRA AREA PREFIX = " << intraAreaPrefixLSAList.size() << "\n";
    for (auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++) {
        OSPFv3LSAHeader& header = (*it)->getHeaderForUpdate();
        out << "AdvertisingRouter =\t" << header.getAdvertisingRouter()<< "\n";
        out << "LinkStateID =\t\t" << header.getLinkStateID() << "\n";
        out << "Age = \t\t\t" << header.getLsaAge() << endl;
        out << "getPrefixesArraySize = " << (*it)->getPrefixesArraySize() << "\n";
        out << "getReferencedLSType = " << (*it)->getReferencedLSType() << "\n";
        out << "getReferencedLSID = " << (*it)->getReferencedLSID() << "\n";
        out << "getReferencedAdvRtr = " << (*it)->getReferencedAdvRtr() << "\n";
        out << "prefixes :" << "\n";
        for (int i = 0; i < (*it)->getPrefixesArraySize(); i++) {

            out << "addressPrefix = "<< (*it)->getPrefixes(i).addressPrefix << "/" << int((*it)->getPrefixes(i).prefixLen) << "\n";
            out << "metric = "<< int((*it)->getPrefixes(i).metric) << "\n";
        }
        out << "\n";
    }
    out << "\n\n";
*/



    return out.str();

}//detailedInfo
}//namespace inet
