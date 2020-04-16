
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateDown.h"

namespace inet {
namespace ospfv3 {

// FIXME!!! Should come from a global unique number generator module.
unsigned long Ospfv3Neighbor::ddSequenceNumberInitSeed = 0;

Ospfv3Neighbor::Ospfv3Neighbor(Ipv4Address newId, Ospfv3Interface* parent)
{
    EV_DEBUG << "$$$$$$ New Ospfv3Neighbor has been created\n";
    this->neighborId = newId;
    this->state = new Ospfv3NeighborStateDown;
    this->containingInterface = parent;
    this->neighborsDesignatedRouter = NULL_IPV4ADDRESS;
    this->neighborsBackupDesignatedRouter = NULL_IPV4ADDRESS;
    this->neighborsRouterDeadInterval = DEFAULT_DEAD_INTERVAL;
    //this is always only link local address
    this->neighborIPAddress = Ipv6Address::UNSPECIFIED_ADDRESS;
    if (this->getInterface()->getArea()->getInstance()->getAddressFamily() == IPV6INSTANCE) {
        this->neighborsDesignatedIP = Ipv6Address::UNSPECIFIED_ADDRESS;
        this->neighborsBackupIP = Ipv6Address::UNSPECIFIED_ADDRESS;
    }
    else {
        this->neighborsDesignatedIP = Ipv4Address::UNSPECIFIED_ADDRESS;
        this->neighborsBackupIP = Ipv4Address::UNSPECIFIED_ADDRESS;
    }

    inactivityTimer = new cMessage("Ospfv3Neighbor::NeighborInactivityTimer", NEIGHBOR_INACTIVITY_TIMER);
    inactivityTimer->setContextPointer(this);
    pollTimer = new cMessage("Ospfv3Neighbor::NeighborPollTimer", NEIGHBOR_POLL_TIMER);
    pollTimer->setContextPointer(this);
    ddRetransmissionTimer = new cMessage("Ospfv3Neighbor::NeighborDDRetransmissionTimer", NEIGHBOR_DD_RETRANSMISSION_TIMER);
    ddRetransmissionTimer->setContextPointer(this);
    updateRetransmissionTimer = new cMessage("Ospfv3Neighbor::Neighbor::NeighborUpdateRetransmissionTimer", NEIGHBOR_UPDATE_RETRANSMISSION_TIMER);
    updateRetransmissionTimer->setContextPointer(this);
    requestRetransmissionTimer = new cMessage("Ospfv3sNeighbor::NeighborRequestRetransmissionTimer", NEIGHBOR_REQUEST_RETRANSMISSION_TIMER);
    requestRetransmissionTimer->setContextPointer(this);
}//constructor

Ospfv3Neighbor::~Ospfv3Neighbor()
{
    reset();
    Ospfv3Process *proc = this->getInterface()->getArea()->getInstance()->getProcess();
    proc->clearTimer(inactivityTimer);
    proc->clearTimer(pollTimer);
    proc->clearTimer(ddRetransmissionTimer);
    proc->clearTimer(updateRetransmissionTimer);
    proc->clearTimer(requestRetransmissionTimer);
    delete inactivityTimer;
    delete pollTimer;
    delete ddRetransmissionTimer;
    delete updateRetransmissionTimer;
    delete requestRetransmissionTimer;
    if (previousState != nullptr) {
        delete previousState;
    }
    delete state;
}//destructor

Ospfv3Neighbor::Ospfv3NeighborStateType Ospfv3Neighbor::getState() const
{
    return state->getState();
}

void Ospfv3Neighbor::processEvent(Ospfv3Neighbor::Ospfv3NeighborEventType event)
{
    EV_DEBUG << "Passing event number " << event << " to state\n";
    this->state->processEvent(this, event);
}

void Ospfv3Neighbor::changeState(Ospfv3NeighborState *newState, Ospfv3NeighborState *currentState)
{
    if (this->previousState != nullptr) {
        EV_DEBUG << "Changing neighbor from state" << currentState->getNeighborStateString() << " to " << newState->getNeighborStateString() << "\n";
        delete this->previousState;
    }
    this->state = newState;
    this->previousState = currentState;
}

void Ospfv3Neighbor::reset()
{
    EV_DEBUG << "Reseting the neighbor " << this->getNeighborID() << "\n";
    for (auto retIt = linkStateRetransmissionList.begin();
         retIt != linkStateRetransmissionList.end();
         retIt++)
    {
        delete (*retIt);
    }
    linkStateRetransmissionList.clear();

    for (auto it = databaseSummaryList.begin(); it != databaseSummaryList.end(); it++) {
        delete (*it);
    }
    databaseSummaryList.clear();
    for (auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        delete (*it);
    }
    linkStateRequestList.clear();

    this->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(ddRetransmissionTimer);
    clearUpdateRetransmissionTimer();
    clearRequestRetransmissionTimer();

    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

bool Ospfv3Neighbor::needAdjacency()
{
    Ospfv3Interface::Ospfv3InterfaceType interfaceType = this->getInterface()->getType();
    Ipv4Address routerID = this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID();
    Ipv4Address dRouter = this->getInterface()->getDesignatedID();
    Ipv4Address backupDRouter = this->getInterface()->getBackupID();

    if ((interfaceType == Ospfv3Interface::POINTTOPOINT_TYPE) ||
        (interfaceType == Ospfv3Interface::POINTTOMULTIPOINT_TYPE) ||
        (interfaceType == Ospfv3Interface::VIRTUAL_TYPE) ||
        (dRouter == routerID) ||
        (backupDRouter == routerID) ||
        (!designatedRoutersSetUp &&
                neighborsDesignatedRouter == NULL_IPV4ADDRESS)||
        (this->getNeighborID() == dRouter) ||
        (this->getNeighborID() == backupDRouter))
    {
        EV_DEBUG << "I need an adjacency with router " << this->getNeighborID()<<"\n";
        return true;
    }
    else {
        return false;
    }
}

void Ospfv3Neighbor::initFirstAdjacency()
{
    ddSequenceNumber = getUniqueULong();
    firstAdjacencyInited = true;
}

unsigned long Ospfv3Neighbor::getUniqueULong()
{
    return ddSequenceNumberInitSeed++;
}

void Ospfv3Neighbor::startUpdateRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->setTimer(this->getUpdateRetransmissionTimer(), this->getInterface()->getRetransmissionInterval());
    this->updateRetransmissionTimerActive = true;
    EV_DEBUG << "Starting UPDATE TIMER\n";
}

void Ospfv3Neighbor::clearUpdateRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(this->getUpdateRetransmissionTimer());
    this->updateRetransmissionTimerActive = false;
}

void Ospfv3Neighbor::startRequestRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->setTimer(this->requestRetransmissionTimer, this->getInterface()->getRetransmissionInterval());
    this->requestRetransmissionTimerActive = true;
}

void Ospfv3Neighbor::clearRequestRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(this->getRequestRetransmissionTimer());
    this->requestRetransmissionTimerActive = false;
}

void Ospfv3Neighbor::sendDDPacket(bool init)
{
    EV_DEBUG << "Start of function send DD Packet\n";
    const auto& ddPacket = makeShared<Ospfv3DatabaseDescriptionPacket>();

    //common header first
    ddPacket->setType(ospf::DATABASE_DESCRIPTION_PACKET);
    ddPacket->setRouterID(this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID());
    ddPacket->setAreaID(this->getInterface()->getArea()->getAreaID());
    ddPacket->setInstanceID(this->getInterface()->getArea()->getInstance()->getInstanceID());

    //DD packet next
    Ospfv3Options options;
    memset(&options, 0, sizeof(Ospfv3Options));
    options.eBit = this->getInterface()->getArea()->getExternalRoutingCapability();
    ddPacket->setOptions(options);
    ddPacket->setInterfaceMTU(this->getInterface()->getInterfaceMTU());
    Ospfv3DdOptions ddOptions;
    ddPacket->setSequenceNumber(this->ddSequenceNumber);

    B packetSize = OSPFV3_HEADER_LENGTH + OSPFV3_DD_HEADER_LENGTH;

    if (init || databaseSummaryList.empty()) {
        ddPacket->setLsaHeadersArraySize(0);
    }
    else {
        while (!this->databaseSummaryList.empty()) {/// && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
            unsigned long headerCount = ddPacket->getLsaHeadersArraySize();
            Ospfv3LsaHeader *lsaHeader = *(databaseSummaryList.begin());
            ddPacket->setLsaHeadersArraySize(headerCount + 1);
            ddPacket->setLsaHeaders(headerCount, *lsaHeader);
            delete lsaHeader;
            databaseSummaryList.pop_front();
            packetSize += OSPFV3_LSA_HEADER_LENGTH;
        }
    }

    EV_DEBUG << "DatabaseSummatyListCount = " << this->getDatabaseSummaryListCount() << endl;
    if (init) {
        ddOptions.iBit = true;
        ddOptions.mBit = true;
        ddOptions.msBit = true;
    }
    else {
        ddOptions.iBit = false;
        ddOptions.mBit = (databaseSummaryList.empty()) ? false : true;
        ddOptions.msBit = (databaseExchangeRelationship == Ospfv3Neighbor::MASTER) ? true : false;
    }

    ddPacket->setDdOptions(ddOptions);

    ddPacket->setPacketLengthField(packetSize.get());
    ddPacket->setChunkLength(packetSize);
    Packet *pk = new Packet();
    pk->insertAtBack(ddPacket);

    //TODO - ddPacket does not include Virtual Links and HopLimit. Also Checksum is not calculated
    if (this->getInterface()->getType() == Ospfv3Interface::POINTTOPOINT_TYPE) {
        EV_DEBUG << "(P2P link ) Send DD Packet to OSPF MCAST\n";
            this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(pk,Ipv6Address::ALL_OSPF_ROUTERS_MCAST, this->getInterface()->getIntName().c_str());
    }
    else {
        EV_DEBUG << "Send DD Packet to " <<  this->getNeighborIP() << "\n";
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(pk,this->getNeighborIP(), this->getInterface()->getIntName().c_str());
    }
}

void Ospfv3Neighbor::sendLinkStateRequestPacket()
{
    const auto& requestPacket = makeShared<Ospfv3LinkStateRequestPacket>();
    requestPacket->setType(ospf::LINKSTATE_REQUEST_PACKET);
    requestPacket->setRouterID(this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID());
    requestPacket->setAreaID(this->getInterface()->getArea()->getAreaID());
    requestPacket->setInstanceID(this->getInterface()->getArea()->getInstance()->getInstanceID());

    //long maxPacketSize = (((IP_MAX_HEADER_BYTES + OSPF_HEADER_LENGTH + OSPF_REQUEST_LENGTH) > parentInterface->getMTU()) ?
    //                      IPV4_DATAGRAM_LENGTH :
    //                      parentInterface->getMTU()) - IP_MAX_HEADER_BYTES;
    B packetSize = OSPFV3_HEADER_LENGTH;

    if (linkStateRequestList.empty()) {
        requestPacket->setRequestsArraySize(0);
    }
    else {
        auto it = linkStateRequestList.begin();

        while (it != linkStateRequestList.end()) { //TODO - maxpacketsize
            unsigned long requestCount = requestPacket->getRequestsArraySize();
            Ospfv3LsaHeader *requestHeader = (*it);
            Ospfv3LsRequest request;

            request.lsaType = requestHeader->getLsaType();
            request.lsaID = requestHeader->getLinkStateID();
            request.advertisingRouter = requestHeader->getAdvertisingRouter();

            requestPacket->setRequestsArraySize(requestCount + 1);
            requestPacket->setRequests(requestCount, request);

            packetSize += OSPFV3_LSR_LENGTH;
            it++;
        }
    }

    requestPacket->setChunkLength(packetSize);
    //TODO - TTL and Checksum calculation  for LS Request is not implemented yet

    Packet *pk = new Packet();
    pk->insertAtBack(requestPacket);
    if (this->getInterface()->getType() == Ospfv3Interface::POINTTOPOINT_TYPE) {
            this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(pk,Ipv6Address::ALL_OSPF_ROUTERS_MCAST, this->getInterface()->getIntName().c_str());
    }
    else {
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(pk,this->getNeighborIP(), this->getInterface()->getIntName().c_str());
    }
}

void Ospfv3Neighbor::createDatabaseSummary()
{
    Ospfv3Area* area = this->getInterface()->getArea();
    int routerLSACount = area->getRouterLSACount();

    for (int i=0; i<routerLSACount; i++) {
        Ospfv3LsaHeader* lsaHeader = new Ospfv3LsaHeader(area->getRouterLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int networkLSACount = area->getNetworkLSACount();
    for (int i=0; i<networkLSACount; i++) {
        Ospfv3LsaHeader* lsaHeader = new Ospfv3LsaHeader(area->getNetworkLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int interAreaPrefixCount = area->getInterAreaPrefixLSACount();
    for (int i=0; i<interAreaPrefixCount; i++) {
        Ospfv3LsaHeader* lsaHeader = new Ospfv3LsaHeader(area->getInterAreaPrefixLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int linkLsaCount = this->getInterface()->getLinkLSACount();
    for (int i=0; i<linkLsaCount; i++) {
        Ospfv3LsaHeader* lsaHeader = new Ospfv3LsaHeader(this->getInterface()->getLinkLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int intraAreaPrefixCnt = area->getIntraAreaPrefixLSACount();
    for (int i=0; i<intraAreaPrefixCnt; i++) {
        Ospfv3LsaHeader* lsaHeader = new Ospfv3LsaHeader(area->getIntraAreaPrefixLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }
}

void Ospfv3Neighbor::retransmitUpdatePacket()
{
    EV_DEBUG << "Retransmitting update packet\n";
    const auto& updatePacket = makeShared<Ospfv3LinkStateUpdatePacket>();

    updatePacket->setType(ospf::LINKSTATE_UPDATE_PACKET);
    updatePacket->setRouterID(this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID());
    updatePacket->setAreaID(this->getInterface()->getArea()->getAreaID());
    updatePacket->setInstanceID(this->getInterface()->getArea()->getInstance()->getInstanceID());

    bool packetFull = false;
    unsigned short lsaCount = 0;
    B packetLength = OSPFV3_HEADER_LENGTH + OSPFV3_LSA_HEADER_LENGTH;
    auto it = linkStateRetransmissionList.begin();

    while (!packetFull && (it != linkStateRetransmissionList.end())) {
        uint16_t lsaType = (*it)->getHeader().getLsaType();
        const Ospfv3RouterLsa *routerLSA = (lsaType == ROUTER_LSA) ? dynamic_cast<Ospfv3RouterLsa *>(*it) : nullptr;
        const Ospfv3NetworkLsa *networkLSA = (lsaType == NETWORK_LSA) ? dynamic_cast<Ospfv3NetworkLsa *>(*it) : nullptr;
        const Ospfv3LinkLsa *linkLSA = (lsaType == LINK_LSA) ? dynamic_cast<Ospfv3LinkLsa *>(*it) : nullptr;
        const Ospfv3InterAreaPrefixLsa *interAreaPrefixLSA = (lsaType == INTER_AREA_PREFIX_LSA) ? dynamic_cast<Ospfv3InterAreaPrefixLsa *>(*it) : nullptr;
        const Ospfv3IntraAreaPrefixLsa *intraAreaPrefixLSA = (lsaType == INTRA_AREA_PREFIX_LSA) ? dynamic_cast<Ospfv3IntraAreaPrefixLsa *>(*it) : nullptr;
//        OSPFASExternalLSA *asExternalLSA = (lsaType == AS_EXTERNAL_LSA_TYPE) ? dynamic_cast<OSPFASExternalLSA *>(*it) : nullptr;
        B lsaSize;
        bool includeLSA = false;

        switch (lsaType) {
            case ROUTER_LSA:
                if (routerLSA != nullptr) {
                    lsaSize = calculateLSASize(routerLSA);
                }
                break;

            case NETWORK_LSA:
                if (networkLSA != nullptr) {
                    lsaSize = calculateLSASize(networkLSA);
                }
                break;
            case INTER_AREA_PREFIX_LSA:
                if (interAreaPrefixLSA != nullptr) {
                    lsaSize = calculateLSASize(interAreaPrefixLSA);
                }
                break;
//            case AS_EXTERNAL_LSA_TYPE:
//                if (asExternalLSA != nullptr) {
//                    lsaSize = calculateLSASize(asExternalLSA);
//                }
//                break;
//
//            default:
//                break;
            case LINK_LSA:
                if (linkLSA != nullptr)
                    lsaSize = calculateLSASize(linkLSA);
                break;
            case INTRA_AREA_PREFIX_LSA:
                if (intraAreaPrefixLSA != nullptr) {
                    lsaSize = calculateLSASize(intraAreaPrefixLSA);
                }
                break;
        }

        if (B(packetLength + lsaSize).get() < this->getInterface()->getInterfaceMTU()) {
            includeLSA = true;
            lsaCount++;
        }
        else {
            if ((lsaCount == 0) && (packetLength + lsaSize < IPV6_DATAGRAM_LENGTH)) {
                includeLSA = true;
                lsaCount++;
                packetFull = true;
            }
        }

        if (includeLSA) {
            packetLength += lsaSize;
            switch (lsaType) {
                case ROUTER_LSA:
                    if (routerLSA != nullptr) {
                        unsigned int routerLSACount = updatePacket->getRouterLSAsArraySize();

                        updatePacket->setRouterLSAsArraySize(routerLSACount + 1);
                        updatePacket->setRouterLSAs(routerLSACount, *routerLSA);

                        unsigned short lsAge = updatePacket->getRouterLSAs(routerLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getRouterLSAsForUpdate(routerLSACount).getHeaderForUpdate().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getRouterLSAsForUpdate(routerLSACount).getHeaderForUpdate().setLsaAge(MAX_AGE);
                        }
                    }
                    break;

                case NETWORK_LSA:
                    if (networkLSA != nullptr) {
                        unsigned int networkLSACount = updatePacket->getNetworkLSAsArraySize();

                        updatePacket->setNetworkLSAsArraySize(networkLSACount + 1);
                        updatePacket->setNetworkLSAs(networkLSACount, *networkLSA);

                        unsigned short lsAge = updatePacket->getNetworkLSAs(networkLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getNetworkLSAsForUpdate(networkLSACount).getHeaderForUpdate().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getNetworkLSAsForUpdate(networkLSACount).getHeaderForUpdate().setLsaAge(MAX_AGE);
                        }
                    }
                    break;
                case INTER_AREA_PREFIX_LSA:
                    if (interAreaPrefixLSA != nullptr) {
                        unsigned int interAreaPrefixLSACount = updatePacket->getInterAreaPrefixLSAsArraySize();

                        updatePacket->setInterAreaPrefixLSAsArraySize(interAreaPrefixLSACount + 1);
                        updatePacket->setInterAreaPrefixLSAs(interAreaPrefixLSACount, *interAreaPrefixLSA);

                        unsigned short lsAge = updatePacket->getInterAreaPrefixLSAs(interAreaPrefixLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getInterAreaPrefixLSAsForUpdate(interAreaPrefixLSACount).getHeaderForUpdate().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getInterAreaPrefixLSAsForUpdate(interAreaPrefixLSACount).getHeaderForUpdate().setLsaAge(MAX_AGE);
                        }
                    }
                    break;
//                case AS_EXTERNAL_LSA_TYPE:
//                    if (asExternalLSA != nullptr) {
//                        unsigned int asExternalLSACount = updatePacket->getAsExternalLSAsArraySize();
//
//                        updatePacket->setAsExternalLSAsArraySize(asExternalLSACount + 1);
//                        updatePacket->setAsExternalLSAs(asExternalLSACount, *asExternalLSA);
//
//                        unsigned short lsAge = updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().getLsAge();
//                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
//                            updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().setLsAge(lsAge + parentInterface->getTransmissionDelay());
//                        }
//                        else {
//                            updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().setLsAge(MAX_AGE);
//                        }
//                    }
//                    break;
                case LINK_LSA:
                    if (linkLSA != nullptr) {
                        unsigned int linkLSACount = updatePacket->getLinkLSAsArraySize();

                        updatePacket->setLinkLSAsArraySize(linkLSACount + 1);
                        updatePacket->setLinkLSAs(linkLSACount, *linkLSA);

                        unsigned short lsAge = updatePacket->getLinkLSAs(linkLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getLinkLSAsForUpdate(linkLSACount).getHeaderForUpdate().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getLinkLSAsForUpdate(linkLSACount).getHeaderForUpdate().setLsaAge(MAX_AGE);
                        }
                    }
                    break;
                case INTRA_AREA_PREFIX_LSA:
                    if (intraAreaPrefixLSA != nullptr) {
                        unsigned int intraAreaPrefixLSACount = updatePacket->getIntraAreaPrefixLSAsArraySize();

                        updatePacket->setIntraAreaPrefixLSAsArraySize(intraAreaPrefixLSACount + 1);
                        updatePacket->setIntraAreaPrefixLSAs(intraAreaPrefixLSACount, *intraAreaPrefixLSA);

                        unsigned short lsAge = updatePacket->getIntraAreaPrefixLSAs(intraAreaPrefixLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getIntraAreaPrefixLSAsForUpdate(intraAreaPrefixLSACount).getHeaderForUpdate().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getIntraAreaPrefixLSAsForUpdate(intraAreaPrefixLSACount).getHeaderForUpdate().setLsaAge(MAX_AGE);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        it++;
    }

    EV_DEBUG << "Retransmit - packet length: "<<packetLength<<"\n";
    updatePacket->setChunkLength(B(packetLength)); //IPV6 HEADER BYTES
    Packet *pk = new Packet();
    pk->insertAtBack(updatePacket);

    this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(pk, this->getNeighborIP(), this->getInterface()->getIntName().c_str());
    //TODO
//    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
//    messageHandler->sendPacket(updatePacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
}

void Ospfv3Neighbor::addToRetransmissionList(const Ospfv3Lsa *lsa)
{
    auto it = linkStateRetransmissionList.begin();
    for ( ; it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID()) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsa->getHeader().getAdvertisingRouter().getInt()))
        {
            break;
        }
    }

    Ospfv3Lsa *lsaCopy = lsa->dup();
//    switch (lsaC->getHeader().getLsaType()) {
//        case ROUTER_LSA:
//            lsaCopy = new Ospfv3RouterLsa((const_cast<Ospfv3Lsa*>(lsaC))));
//            break;
//
//        case NETWORK_LSA:
//            lsaCopy = new Ospfv3NetworkLsa(*(check_and_cast<Ospfv3NetworkLsa *>(const_cast<Ospfv3Lsa*>(lsaC))));
//            break;
//
//        case INTER_AREA_PREFIX_LSA:
//            lsaCopy = new Ospfv3InterAreaPrefixLsa(*(check_and_cast<Ospfv3InterAreaPrefixLsa* >(const_cast<Ospfv3Lsa*>(lsaC))));
//            break;
////        case AS_EXTERNAL_LSA_TYPE:
////            lsaCopy = new OSPFASExternalLSA(*(check_and_cast<OSPFASExternalLSA *>(lsa)));
////            break;
//
//        case LINK_LSA:
//            lsaCopy = new Ospfv3LinkLsa(*(check_and_cast<Ospfv3LinkLsa *>(const_cast<Ospfv3Lsa*>(lsaC))));
//            break;
//
//        case INTRA_AREA_PREFIX_LSA:
//            lsaCopy = new Ospfv3IntraAreaPrefixLsa(*(check_and_cast<Ospfv3IntraAreaPrefixLsa *>(const_cast<Ospfv3Lsa*>(lsaC))));
//            break;
//
//        default:
//            ASSERT(false);    // error
//            break;
//    }

    // if LSA is on retransmission list then replace it
    if (it != linkStateRetransmissionList.end()) {
        delete (*it);
        *it = static_cast<Ospfv3Lsa *>(lsaCopy);
    }
    else {
        //if not then add it
        linkStateRetransmissionList.push_back(static_cast<Ospfv3Lsa *>(lsaCopy));
    }
}

void Ospfv3Neighbor::removeFromRetransmissionList(LSAKeyType lsaKey)
{
    auto it = linkStateRetransmissionList.begin();
    int counter = 0;
    while (it != linkStateRetransmissionList.end())
    {

        EV_DEBUG << counter++ << " - HEADER in retransmition list:\n" <<  (*it)->getHeader() << "\n";
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRetransmissionList.erase(it);
        }
        else
        {
            it++;
        }
    }
}//removeFromRetransmissionList

bool Ospfv3Neighbor::isLinkStateRequestListEmpty(LSAKeyType lsaKey) const
{
    for (auto lsa : linkStateRetransmissionList) {
        if ((lsa->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            (lsa->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

Ospfv3Lsa *Ospfv3Neighbor::findOnRetransmissionList(LSAKeyType lsaKey)
{
    for (auto & elem : linkStateRetransmissionList) {
        if (((elem)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((elem)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return elem;
        }
    }
    return nullptr;
}


bool Ospfv3Neighbor::retransmitDatabaseDescriptionPacket()
{
    EV_DEBUG << "Retransmitting DD Packet\n";
    if (lastTransmittedDDPacket != nullptr) {
        Packet *ddPacket = new Packet(*lastTransmittedDDPacket);
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(ddPacket, this->getNeighborIP(), this->getInterface()->getIntName().c_str());
        return true;
    }
    else {
        EV_DEBUG << "But no packet is in the line\n";
        return false;
    }
}

void Ospfv3Neighbor::addToRequestList(const Ospfv3LsaHeader *lsaHeader)
{
    linkStateRequestList.push_back(new Ospfv3LsaHeader(*lsaHeader));
    EV_DEBUG << "Currently on request list:\n";
    for (auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        EV_DEBUG << "\tType: "<<(*it)->getLsaType() << ", ID: " << (*it)->getLinkStateID() << ", Adv: " << (*it)->getAdvertisingRouter() << "\n";
    }
}

bool Ospfv3Neighbor::isLSAOnRequestList(LSAKeyType lsaKey)
{
    if (findOnRequestList(lsaKey)==nullptr)
        return false;

    return true;
}

Ospfv3LsaHeader* Ospfv3Neighbor::findOnRequestList(LSAKeyType lsaKey)
{
    //linkStateRequestList - list of LSAs that need to be received from the neighbor
    for (auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
                ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return *it;
        }
    }
    return nullptr;
}

void Ospfv3Neighbor::removeFromRequestList(LSAKeyType lsaKey)
{
    auto it = linkStateRequestList.begin();
    while (it != linkStateRequestList.end()) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRequestList.erase(it);
        }
        else {
            it++;
        }
    }
    if ((getState() == Ospfv3Neighbor::LOADING_STATE) && (linkStateRequestList.empty())) {
        clearRequestRetransmissionTimer();
        processEvent(Ospfv3Neighbor::LOADING_DONE);
    }
}//removeFromRequestList

void Ospfv3Neighbor::addToTransmittedLSAList(LSAKeyType lsaKey)
{
    TransmittedLSA transmit;

    transmit.lsaKey = lsaKey;
    transmit.age = 0;

    transmittedLSAs.push_back(transmit);
}//addToTransmittedLSAList

bool Ospfv3Neighbor::isOnTransmittedLSAList(LSAKeyType lsaKey) const
{
    for (std::list<TransmittedLSA>::const_iterator it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        if ((it->lsaKey.linkStateID == lsaKey.linkStateID) &&
            (it->lsaKey.advertisingRouter == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}//isOnTransmittedLSAList

void Ospfv3Neighbor::ageTransmittedLSAList()
{
    auto it = transmittedLSAs.begin();
    while ((it != transmittedLSAs.end()) && (it->age == MIN_LS_ARRIVAL)) {
        transmittedLSAs.pop_front();
        it = transmittedLSAs.begin();
    }
//    for (long i = 0; i < transmittedLSAs.size(); i++)
//    {
//        transmittedLSAs[i].age++;
//    }
    for (it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        it->age++;
    }
}//ageTransmittedLSAList

void Ospfv3Neighbor::deleteLastSentDDPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}//deleteLastSentDDPacket

} // namespace ospfv3
} // namespace inet

