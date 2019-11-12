
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/routing/ospfv3/process/Ospfv3Process.h"

namespace inet {
namespace ospfv3 {

Define_Module(Ospfv3Process);

Ospfv3Process::Ospfv3Process()
{
//    this->processID = processID;
}

Ospfv3Process::~Ospfv3Process()
{
    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++) {
        delete instances[i];
    }
    long routeCount = routingTableIPv4.size();
    for (long k = 0; k < routeCount; k++) {
        delete routingTableIPv4[k];
    }
    routeCount = routingTableIPv6.size();
    for (long k = 0; k < routeCount; k++) {
        delete routingTableIPv6[k];
    }

    this->clearTimer(ageTimer);
    delete ageTimer;
}

void Ospfv3Process::initialize(int stage)
{
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        this->containingModule=findContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt6 = getModuleFromPar<Ipv6RoutingTable>(par("routingTableModule6"), this);
        rt4 = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);

        this->routerID = Ipv4Address(par("routerID").stringValue());
        this->processID = (int)par("processID");
        this->parseConfig(par("interfaceConfig"));

        registerService(Protocol::ospf, nullptr, gate("splitterIn"));
        registerProtocol(Protocol::ospf, gate("splitterOut"), nullptr);

        cMessage* init = new cMessage();
        init->setKind(INIT_PROCESS);
        scheduleAt(simTime()+ OSPFV3_START, init);
        WATCH_PTRVECTOR(this->instances);
        WATCH_PTRVECTOR(this->routingTableIPv6);
        WATCH_PTRVECTOR(this->routingTableIPv4);

        ageTimer = new cMessage("Ospfv3Process::DatabaseAgeTimer", DATABASE_AGE_TIMER);
        ageTimer->setContextPointer(this);

        this->setTimer(ageTimer, 1.0);
    }
}

void Ospfv3Process::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        this->handleTimer(msg);
    }
    else {
        Packet *pk = check_and_cast<Packet *>(msg);
        const auto& packet = pk->peekAtFront<Ospfv3Packet>();

        auto protocol = pk->getTag<PacketProtocolTag>()->getProtocol();     //check if this is ICMPv6 msg
        if (protocol == &Protocol::icmpv6) {
           EV_ERROR << "ICMPv6 error received -- discarding\n";
           delete msg;
        }

        if (packet->getRouterID()==this->getRouterID()) //is it this router who originated the message?
            delete msg;
        else {
            Ospfv3Instance* instance = this->getInstanceById(packet->getInstanceID());
            if (instance == nullptr) {//Is there an instance with this number?
                EV_DEBUG << "Instance with this ID not found, dropping\n";
                delete msg;
            }
            else {
                instance->processPacket(pk);
            }
        }
    }
}//handleMessage

/*return index of the Ipv4 table if the route is found, -1 else*/
int Ospfv3Process::isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv4Route *entry = rtTable->getRoute(i);
        if (Ipv4Address::maskedAddrAreEqual(addr, entry->getDestination(), entry->getNetmask())) {
            return i;
        }
    }
    return -1;
}

int Ospfv3Process::isInRoutingTable6(Ipv6RoutingTable *rtTable, Ipv6Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv6Route *entry = rtTable->getRoute(i);
        if (addr.getPrefix(entry->getPrefixLength()) ==  entry->getDestPrefix().getPrefix(entry->getPrefixLength())) {
            return i;
        }
    }
    return -1;
}

int Ospfv3Process::isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (ifTable->getInterface(i)->findProtocolData<Ipv4InterfaceData>()->getIPAddress() == addr) {
            return i;
        }
    }
    return -1;
}

int Ospfv3Process::isInInterfaceTable6(IInterfaceTable *ifTable, Ipv6Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        for (int j = 0; j < ifTable->getInterface(i)->findProtocolData<Ipv6InterfaceData>()->getNumAddresses(); j++) {
            if (ifTable->getInterface(i)->findProtocolData<Ipv6InterfaceData>()->getAddress(j) == addr) {
                return i;
            }
        }
    }
    return -1;
}

void Ospfv3Process::parseConfig(cXMLElement* interfaceConfig)
{
    EV_DEBUG << "Parsing config on process " << this->processID << endl;
    //Take each interface
    cXMLElementList intList = interfaceConfig->getElementsByTagName("Interface");
    for (auto interfaceIt=intList.begin(); interfaceIt!=intList.end(); interfaceIt++) {
        const char* interfaceName = (*interfaceIt)->getAttribute("name");
        InterfaceEntry *myInterface = CHK(ift->findInterfaceByName(interfaceName));

        if (myInterface->isLoopback()) {
            const char * ipv41 = "127.0.0.0";
            Ipv4Address tmpipv4;
            tmpipv4.set(ipv41);
            int i = isInRoutingTable(rt4, tmpipv4);
            if (i != -1) {
                rt4->deleteRoute(rt4->getRoute(i));
            }
        }
        //interface ipv6 configuration
        cXMLElementList ipAddrList = (*interfaceIt)->getElementsByTagName("Ipv6Address");
        for (auto & ipv6Rec : ipAddrList) {
            const char * addr6c = ipv6Rec->getNodeValue();

            std::string add6 = addr6c;
            std::string prefix6 = add6.substr(0, add6.find("/"));
            Ipv6InterfaceData * intfData6 = myInterface->findProtocolData<Ipv6InterfaceData>();
            int prefLength;
            Ipv6Address address6;
            if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
                 throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

            address6 = Ipv6Address(prefix6.c_str());

            Ipv6InterfaceData::AdvPrefix p;
            p.prefix = address6;
            p.prefixLength = prefLength;

            if (isInInterfaceTable6(ift, address6) < 0) {
                intfData6->assignAddress(address6, false, SIMTIME_ZERO, SIMTIME_ZERO);

                // add this routes to routing table
                Ipv6Route *route = new Ipv6Route(p.prefix.getPrefix(prefLength), p.prefixLength, IRoute::IFACENETMASK);
                route->setInterface(myInterface);
                route->setExpiryTime(SIMTIME_ZERO);
                route->setMetric(0);
                route->setAdminDist(Ipv6Route::dDirectlyConnected);

                rt6->addRoutingProtocolRoute(route);
            }
        }

        //interface ipv4 configuration
        Ipv4Address addr;
        Ipv4Address mask;
        Ipv4InterfaceData * intfData = myInterface->findProtocolData<Ipv4InterfaceData>(); //new Ipv4InterfaceData();
        bool alreadySet = false;

        cXMLElementList ipv4AddrList = (*interfaceIt)->getElementsByTagName("IPAddress");
        if (ipv4AddrList.size() == 1) {
            for (auto & ipv4Rec : ipv4AddrList) {
                const char * addr4c = ipv4Rec->getNodeValue(); //from string make ipv4 address and store to interface config
                addr = (Ipv4Address(addr4c));
                if (isInInterfaceTable(ift, addr) >= 0) { // prevention from seting same interface by second process
                    alreadySet = true;
                    continue;
                }
                intfData->setIPAddress(addr);
            }
            if (!alreadySet) {
                cXMLElementList ipv4MaskList = (*interfaceIt)->getElementsByTagName("Mask");
                if (ipv4MaskList.size() != 1)
                    throw cRuntimeError("Interface %s has more or less than one mask", interfaceName);

                for (auto & ipv4Rec : ipv4MaskList) {
                    const char * mask4c = ipv4Rec->getNodeValue();
                    mask =  (Ipv4Address(mask4c));
                    intfData->setNetmask(mask);

                    //add directly connected ip address to routing table
                    Ipv4Address networkAdd = (Ipv4Address((intfData->getIPAddress()).getInt() & (intfData->getNetmask()).getInt()));
                    Ipv4Route *entry = new Ipv4Route;

                    entry->setDestination(networkAdd);
                    entry->setNetmask(intfData->getNetmask());
                    entry->setInterface(myInterface);
                    entry->setMetric(0);
                    entry->setSourceType(IRoute::IFACENETMASK);

                    rt4->addRoute(entry);
                }
            }
        }
        else if (ipv4AddrList.size() > 1)
            throw cRuntimeError("Interface %s has more than one IPv4 address ", interfaceName);

        const char* routerPriority = nullptr;
        const char* helloInterval = nullptr;
        const char* deadInterval = nullptr;
        const char* interfaceCost = nullptr;
        const char* interfaceType = nullptr;
        Ospfv3Interface::Ospfv3InterfaceType interfaceTypeNum;
        bool passiveInterface = false;


        cXMLElementList process = (*interfaceIt)->getElementsByTagName("Process");
        if (process.size() > 2)
            throw cRuntimeError("More than two processes configured for interface %s", (*interfaceIt)->getAttribute("name"));

        //Check whether it belongs to this process
        int processCount = process.size();
        for (int i = 0; i < processCount; i++) {
            int procId = atoi(process.at(i)->getAttribute("id"));
            if (procId != this->processID)
                continue;

            EV_DEBUG << "Creating new interface "  << interfaceName << " in process " << procId << endl;

            //Parsing instances
            cXMLElementList instList = process.at(i)->getElementsByTagName("Instance");
            for (auto instIt=instList.begin(); instIt!=instList.end(); instIt++) {
                const char* instId = (*instIt)->getAttribute("instanceID");
                const char* addressFamily = (*instIt)->getAttribute("AF");

                //Get the router priority for this interface and instance
                cXMLElementList interfaceOptions = (*instIt)->getElementsByTagName("RouterPriority");
                if (interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple router priority is configured for interface %s", interfaceName);

                if (interfaceOptions.size()!=0)
                    routerPriority = interfaceOptions.at(0)->getNodeValue();

                //get the hello interval for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("HelloInterval");
                if (interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple HelloInterval value is configured for interface %s", interfaceName);

                if (interfaceOptions.size()!=0)
                    helloInterval = interfaceOptions.at(0)->getNodeValue();

                //get the dead interval for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("DeadInterval");
                if (interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple DeadInterval value is configured for interface %s", interfaceName);

                if (interfaceOptions.size()!=0)
                    deadInterval = interfaceOptions.at(0)->getNodeValue();

                //get the interface cost for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("InterfaceCost");
                if (interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple InterfaceCost value is configured for interface %s", interfaceName);

                if (interfaceOptions.size() != 0)
                    interfaceCost = interfaceOptions.at(0)->getNodeValue();

                //get the interface cost for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("InterfaceType");
                if (interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple InterfaceType value is configured for interface %s", interfaceName);

                if (interfaceOptions.size() != 0) {
                    interfaceType = interfaceOptions.at(0)->getNodeValue();
                    if (strcmp(interfaceType, "Broadcast")==0)
                        interfaceTypeNum = Ospfv3Interface::BROADCAST_TYPE;
                    else if (strcmp(interfaceType, "PointToPoint")==0)
                        interfaceTypeNum = Ospfv3Interface::POINTTOPOINT_TYPE;
                    else if (strcmp(interfaceType, "NBMA")==0)
                        interfaceTypeNum = Ospfv3Interface::NBMA_TYPE;
                    else if (strcmp(interfaceType, "PointToMultipoint")==0)
                        interfaceTypeNum = Ospfv3Interface::POINTTOMULTIPOINT_TYPE;
                    else if (strcmp(interfaceType, "Virtual")==0)
                        interfaceTypeNum = Ospfv3Interface::VIRTUAL_TYPE;
                    else
                        interfaceTypeNum = Ospfv3Interface::UNKNOWN_TYPE;
                }
                else
                    throw cRuntimeError("Interface type needs to be specified for interface %s", interfaceName);

                //find out whether the interface is passive
                interfaceOptions = (*instIt)->getElementsByTagName("PassiveInterface");
                if (interfaceOptions.size() > 1)
                    throw cRuntimeError("Multiple PassiveInterface value is configured for interface %s", interfaceName);

                if (interfaceOptions.size() != 0) {
                    if (strcmp(interfaceOptions.at(0)->getNodeValue(), "True") == 0)
                        passiveInterface = true;
                }

                int instIdNum;

                if (instId == nullptr) {
                    EV_DEBUG << "Address Family " << addressFamily << endl;
                    if (strcmp(addressFamily, "IPv4") == 0) {
                        EV_DEBUG << "IPv4 instance\n";
                        instIdNum = DEFAULT_IPV4_INSTANCE;
                    }
                    else if (strcmp(addressFamily, "IPv6") == 0) {
                        EV_DEBUG << "IPv6 instance\n";
                        instIdNum = DEFAULT_IPV6_INSTANCE;
                    }
                    else
                        throw cRuntimeError("Unknown address family in process %d", this->getProcessID());
                }
                else
                    instIdNum = atoi(instId);

                //TODO - check range of instance ID.
                //check for multiple definition of one instance
                Ospfv3Instance* instance = this->getInstanceById(instIdNum);
                //if (instance != nullptr)
                // throw cRuntimeError("Multiple Ospfv3 instance with the same instance ID configured for process %d on interface %s", this->getProcessID(), interfaceName);

                if (instance == nullptr) {
                    if (strcmp(addressFamily, "IPv4") == 0)
                        instance = new Ospfv3Instance(instIdNum, this, IPV4INSTANCE);
                    else
                        instance = new Ospfv3Instance(instIdNum, this, IPV6INSTANCE);

                    EV_DEBUG << "Adding instance " << instIdNum << " to process " << this->processID << endl;
                    this->addInstance(instance);
                }

                // multiarea configuration is not supported
                cXMLElementList areasList = (*instIt)->getElementsByTagName("Area");
                for (auto areasIt=areasList.begin(); areasIt!=areasList.end(); areasIt++) {
                    const char* areaId = (*areasIt)->getNodeValue();
                    Ipv4Address areaIP = Ipv4Address(areaId);
                    const char* areaType = (*areasIt)->getAttribute("type");
                    Ospfv3AreaType type = NORMAL;

                    if (areaType != nullptr) {
                        if (strcmp(areaType, "stub") == 0)
                            type = STUB;
                        else if (strcmp(areaType, "nssa") == 0)
                            type = NSSA;
                    }

                    const char* summary = (*areasIt)->getAttribute("summary");

                    if (summary != nullptr) {
                        if (strcmp(summary, "no") == 0) {
                            if (type == STUB)
                                type = TOTALLY_STUBBY;
                            else if (type == NSSA)
                                type = NSSA_TOTALLY_STUB;
                        }
                    }

                    //insert area if it's not already there and assign this interface
                    Ospfv3Area* area;
                    if (!(instance->hasArea(areaIP))) {
                        area = new Ospfv3Area(areaIP, instance, type);
                        instance->addArea(area);
                    }
                    else
                        area = instance->getAreaById(areaIP);

                    if (!area->hasInterface(std::string(interfaceName))) {
                        Ospfv3Interface* newInterface = new Ospfv3Interface(interfaceName, this->containingModule, this, interfaceTypeNum, passiveInterface);
                        if (helloInterval != nullptr)
                            newInterface->setHelloInterval(atoi(helloInterval));

                        if (deadInterval!=nullptr)
                            newInterface->setDeadInterval(atoi(deadInterval));

                        if (interfaceCost!=nullptr)
                            newInterface->setInterfaceCost(atoi(interfaceCost));

                        if (routerPriority!=nullptr) {
                            int rtrPrio = atoi(routerPriority);
                            if (rtrPrio < 0 || rtrPrio > 255)
                                throw cRuntimeError("Router priority out of range on interface %s", interfaceName);

                            newInterface->setRouterPriority(rtrPrio);
                        }

                        newInterface->setArea(area);

                        cXMLElementList ipAddrList = (*interfaceIt)->getElementsByTagName("Ipv6Address");
                        for (auto & ipv6Rec : ipAddrList) {
                            const char * addr6c = ipv6Rec->getNodeValue();
                            std::string add6 = addr6c;
                            std::string prefix6 = add6.substr(0, add6.find("/"));
                            int prefLength;
                            Ipv6Address address6;
                            if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
                                 throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

                            address6 = Ipv6Address(prefix6.c_str());

                            Ipv6AddressRange ipv6addRange; //add directly networks into addressRange for given area
                            ipv6addRange.prefix = address6; //add only network prefix
                            ipv6addRange.prefixLength = prefLength;
                            area->addAddressRange(ipv6addRange, true);
                            //TODO:  Address added into Adressrange have Advertise always set to true. Exclude link-local (?)
                        }
                        cXMLElementList ipv4AddrList = (*interfaceIt)->getElementsByTagName("IPAddress");
                        if (ipv4AddrList.size() == 1) {
                            Ipv4AddressRange ipv4addRange; // also create addressRange for IPv4
                            for (auto & ipv4Rec : ipv4AddrList) {
                                const char * addr4c = ipv4Rec->getNodeValue(); //from string make ipv4 address and store to interface config
                                ipv4addRange.address = Ipv4Address(addr4c);
                            }

                            cXMLElementList ipv4MaskList = (*interfaceIt)->getElementsByTagName("Mask");
                            if (ipv4MaskList.size() != 1)
                                throw cRuntimeError("Interface %s has more or less than one mask", interfaceName);

                            for (auto & ipv4Rec : ipv4MaskList) {
                                const char * mask4c = ipv4Rec->getNodeValue();
                                ipv4addRange.mask = Ipv4Address(mask4c);
                            }
                            area->addAddressRange(ipv4addRange, true);
                        }
                        EV_DEBUG << "I am " << this->getOwner()->getOwner()->getName() << " on int " << newInterface->getInterfaceLLIP() << " with area " << area->getAreaID() <<"\n";
                        area->addInterface(newInterface);
                    }
                }
            }
        }
    }
}//parseConfig

void Ospfv3Process::ageDatabase()
{
    bool shouldRebuildRoutingTable = false;

    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++) {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++)
            instances[i]->getArea(j)->ageDatabase();

        if (shouldRebuildRoutingTable)
            rebuildRoutingTable();
    }
} // ageDatabase

// for IPv6 AF
bool Ospfv3Process::hasAddressRange(const Ipv6AddressRange& addressRange) const
{
    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++) {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++) {
           if (instances[i]->getArea(j)->hasAddressRange(addressRange))
               return true;
        }
    }
    return false;
}

// for IPv4 AF
bool Ospfv3Process::hasAddressRange(const Ipv4AddressRange& addressRange) const
{
    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++) {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++) {
           if (instances[i]->getArea(j)->hasAddressRange(addressRange)) {
               return true;
           }
        }
    }
    return false;
}

void Ospfv3Process::handleTimer(cMessage* msg)
{
    switch (msg->getKind()) {
        case INIT_PROCESS:
            for (auto it=this->instances.begin(); it!=this->instances.end(); it++)
                (*it)->init();

            this->debugDump();
            delete msg;
        break;

        case HELLO_TIMER_INIT:  // timer set by process, for initialisation of hello msgs
        case HELLO_TIMER:       // timer set by interface every 10 sec
        {
            Ospfv3Interface* interface;
            if (!(interface=reinterpret_cast<Ospfv3Interface*>(msg->getContextPointer()))) {
                //TODO - Print some error (?)
                delete msg;
            }
            else {
                EV_DEBUG << "Process received msg, sending event HELLO_TIMER_EVENT\n";
                interface->processEvent(Ospfv3Interface::HELLO_TIMER_EVENT);
            }
        }
        break;

        case WAIT_TIMER:
        {
            Ospfv3Interface* interface;
            if (!(interface=reinterpret_cast<Ospfv3Interface*>(msg->getContextPointer()))) {
                //TODO - Print some error (?)
                delete msg;
            }
            else {
                EV_DEBUG << "Process received msg, sending event WAIT_TIMER_EVENT\n";
                interface->processEvent(Ospfv3Interface::WAIT_TIMER_EVENT);
            }
        }
        break;

        case ACKNOWLEDGEMENT_TIMER: {
            Ospfv3Interface *intf;
            if (!(intf = reinterpret_cast<Ospfv3Interface *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid InterfaceAcknowledgementTimer.\n";
                delete msg;
            }
            else {
                intf->processEvent(Ospfv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT);
            }
        }
        break;

        case NEIGHBOR_INACTIVITY_TIMER: {
            Ospfv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Ospfv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Inactivity Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Ospfv3Neighbor::INACTIVITY_TIMER);
                Ospfv3Interface* intf = neighbor->getInterface();
                int neighborCnt = intf->getNeighborCount();
                for (int i=0; i<neighborCnt; i++) {
                    Ospfv3Neighbor* currNei = intf->getNeighbor(i);
                    if (currNei->getNeighborID() == neighbor->getNeighborID()) {
                        intf->removeNeighborByID(neighbor->getNeighborID());
                        delete neighbor;
                        break;
                    }
                }

                intf->processEvent(Ospfv3Interface::NEIGHBOR_CHANGE_EVENT);
            }
        }
        break;

        case NEIGHBOR_POLL_TIMER: {
            Ospfv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Ospfv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Poll Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Ospfv3Neighbor::POLL_TIMER);
            }
        }
        break;

        case NEIGHBOR_DD_RETRANSMISSION_TIMER: {
            Ospfv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Ospfv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborDDRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Database Description Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Ospfv3Neighbor::DD_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_UPDATE_RETRANSMISSION_TIMER: {
            Ospfv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Ospfv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Update Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Ospfv3Neighbor::UPDATE_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_REQUEST_RETRANSMISSION_TIMER: {
            Ospfv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Ospfv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Request Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Ospfv3Neighbor::REQUEST_RETRANSMISSION_TIMER);
            }
        }
        break;

        case DATABASE_AGE_TIMER: {
            EV_DEBUG << "Ageing the database\n";
            this->setTimer(ageTimer, 1.0);
            this->ageDatabase();
        }
        break;

        default:
            break;
    }
}

void Ospfv3Process::setTimer(cMessage* msg, double delay = 0)
{
    scheduleAt(simTime()+delay, msg);
}

void Ospfv3Process::activateProcess()
{
    Enter_Method_Silent();
    this->isActive=true;
    cMessage* init = new cMessage();
    init->setKind(HELLO_TIMER_INIT);
    scheduleAt(simTime(), init);
}//activateProcess

void Ospfv3Process::debugDump()
{
    EV_DEBUG << "Process " << this->getProcessID() << "\n";
    for (auto it=this->instances.begin(); it!=this->instances.end(); it++)
        (*it)->debugDump();
}//debugDump

Ospfv3Instance* Ospfv3Process::getInstanceById(int instanceId)
{
    std::map<int, Ospfv3Instance*>::iterator instIt = this->instancesById.find(instanceId);
    if (instIt == this->instancesById.end())
        return nullptr;

    return instIt->second;
}

void Ospfv3Process::addInstance(Ospfv3Instance* newInstance)
{
    Ospfv3Instance* check = this->getInstanceById(newInstance->getInstanceID());
    if (check==nullptr) {
        this->instances.push_back(newInstance);
        this->instancesById[newInstance->getInstanceID()]=newInstance;
    }
}

void Ospfv3Process::sendPacket(Packet *packet, Ipv6Address destination, const char* ifName, short hopLimit)
{
    InterfaceEntry *ie = CHK(this->ift->findInterfaceByName(ifName));
    Ipv6InterfaceData *ipv6int = ie->getProtocolData<Ipv6InterfaceData>();

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ospf);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    packet->addTagIfAbsent<L3AddressReq>()->setDestAddress(destination);
    packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(ipv6int->getLinkLocalAddress());
    packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(hopLimit);
    const auto& ospfPacket = packet->peekAtFront<Ospfv3Packet>();

    switch (ospfPacket->getType()) {
        case ospf::HELLO_PACKET: {
            packet->setName("Ospfv3_HelloPacket");
//            const auto& helloPacket = packet->peekAtFront<Ospfv3HelloPacket>();
//            printHelloPacket(helloPacket.get(), destination, outputIfIndex);
        }
        break;
        case ospf::DATABASE_DESCRIPTION_PACKET: {
            packet->setName("Ospfv3_DDPacket");
//            const auto& ddPacket = packet->peekAtFront<Ospfv3DatabaseDescription>();
//            printDatabaseDescriptionPacket(ddPacket.get(), destination, outputIfIndex);
        }
        break;

        case ospf::LINKSTATE_REQUEST_PACKET: {
            packet->setName("Ospfv3_LSRPacket");
//            const auto& requestPacket = packet->peekAtFront<Ospfv3LinkStateRequest>();
//            printLinkStateRequestPacket(requestPacket.get(), destination, outputIfIndex);
        }
        break;

        case ospf::LINKSTATE_UPDATE_PACKET: {
            packet->setName("Ospfv3_LSUPacket");
//            const auto& updatePacket = packet->peekAtFront<Ospfv3LsUpdate>();
//            printLinkStateUpdatePacket(updatePacket.get(), destination, outputIfIndex);
        }
        break;

        case ospf::LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            packet->setName("Ospfv3_LSAckPacket");
//            const auto& ackPacket = packet->peekAtFront<Ospfv3LsAck>();
//            printLinkStateAcknowledgementPacket(ackPacket.get(), destination, outputIfIndex);
        }
        break;

        default:
            break;
    }
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    this->send(packet, "splitterOut");
}//sendPacket

Ospfv3Lsa* Ospfv3Process::findLSA(LSAKeyType lsaKey, Ipv4Address areaID, int instanceID)
{
    Ospfv3Instance* instance = this->getInstanceById(instanceID);
    Ospfv3Area* area = instance->getAreaById(areaID);
    return area->getLSAbyKey(lsaKey);
}

bool Ospfv3Process::floodLSA(const Ospfv3Lsa* lsa, Ipv4Address areaID, Ospfv3Interface* interface, Ospfv3Neighbor* neighbor)
{
    EV_DEBUG << "Flooding LSA from router " << lsa->getHeader().getAdvertisingRouter() << " with ID " << lsa->getHeader().getLinkStateID() << "\n";
    bool floodedBackOut = false;

    if (lsa != nullptr) {
        Ospfv3Instance* instance = interface->getArea()->getInstance();
        if (lsa->getHeader().getLsaType() == AS_EXTERNAL_LSA) {
            long areaCount = instance->getAreaCount();
            for (long i = 0; i < areaCount; i++) {
                Ospfv3Area* area = instance->getArea(i);
                if (area->getExternalRoutingCapability()) {
                    if (area->floodLSA(lsa, interface, neighbor)) {
                        floodedBackOut = true;
                    }
                }
            }
        }
        else {
            if (Ospfv3Area* area = instance->getAreaById(areaID)) {
                floodedBackOut = area->floodLSA(lsa, interface, neighbor);
            }
        }
    }

    return floodedBackOut;
}

bool Ospfv3Process::installLSA(const Ospfv3Lsa *lsa, int instanceID, Ipv4Address areaID    /*= BACKBONE_AREAID*/, Ospfv3Interface* intf)
{
    EV_DEBUG << "Ospfv3Process::installLSA\n";
    switch (lsa->getHeader().getLsaType()) {
        case ROUTER_LSA: {
            Ospfv3Instance* instance = this->getInstanceById(instanceID);
            if (Ospfv3Area* area = instance->getAreaById(areaID)) {
                Ospfv3RouterLsa *ospfRouterLSA = check_and_cast<Ospfv3RouterLsa *>(const_cast<Ospfv3Lsa*>(lsa));
                return area->installRouterLSA(ospfRouterLSA);
            }
        }
        break;

        case NETWORK_LSA: {
            Ospfv3Instance* instance = this->getInstanceById(instanceID);
            if (Ospfv3Area* area = instance->getAreaById(areaID)) {
                Ospfv3NetworkLsa *ospfNetworkLSA = check_and_cast<Ospfv3NetworkLsa *>(const_cast<Ospfv3Lsa*>(lsa));
                return area->installNetworkLSA(ospfNetworkLSA);
            }
        }
        break;

        case INTER_AREA_PREFIX_LSA: {
            Ospfv3Instance* instance = this->getInstanceById(instanceID);
            if (Ospfv3Area* area = instance->getAreaById(areaID)) {
                Ospfv3InterAreaPrefixLsa *ospfInterAreaLSA = check_and_cast<Ospfv3InterAreaPrefixLsa *>(const_cast<Ospfv3Lsa*>(lsa));
                return area->installInterAreaPrefixLSA(ospfInterAreaLSA);
            }
        }
        break;

        case LINK_LSA: {
            Ospfv3Instance* instance = this->getInstanceById(instanceID);
            if (Ospfv3Area* area = instance->getAreaById(areaID)) {
                (void)area; //FIXME set but unused 'area' variable
                Ospfv3LinkLsa *ospfLinkLSA = check_and_cast<Ospfv3LinkLsa *>(const_cast<Ospfv3Lsa*>(lsa));
                return intf->installLinkLSA(ospfLinkLSA);
            }
        }
        break;

        case INTRA_AREA_PREFIX_LSA: {
            Ospfv3Instance* instance = this->getInstanceById(instanceID);
            if (Ospfv3Area* area = instance->getAreaById(areaID)) {
                Ospfv3IntraAreaPrefixLsa* intraLSA = check_and_cast<Ospfv3IntraAreaPrefixLsa *>(const_cast<Ospfv3Lsa*>(lsa));
                return area->installIntraAreaPrefixLSA(intraLSA);
            }
        }
        break;
        default:
            ASSERT(false);
            break;
    }
    return false;
}

void Ospfv3Process::calculateASExternalRoutes(std::vector<Ospfv3RoutingTableEntry* > newTableIPv6, std::vector<Ospfv3Ipv4RoutingTableEntry* > newTableIPv4)
{
    EV_DEBUG << "Calculating AS External Routes (Not ymplemented yet)\n";
}

void Ospfv3Process::rebuildRoutingTable()
{
    unsigned long instanceCount = this->instances.size();
    std::vector<Ospfv3RoutingTableEntry *> newTableIPv6;
    std::vector<Ospfv3Ipv4RoutingTableEntry *> newTableIPv4;

    for (unsigned int k=0; k<instanceCount; k++) {
        Ospfv3Instance* currInst = this->instances.at(k);
        unsigned long areaCount = currInst->getAreaCount();
        bool hasTransitAreas = false;
        (void)hasTransitAreas; //FIXME set but not used variable
        unsigned long i;

        EV_INFO << "Rebuilding routing table for instance " << this->instances.at(k)->getInstanceID() << ":\n";

        // 2) Intra area routes are calculated using SPF algo
        for (i = 0; i < areaCount; i++) {
            currInst->getArea(i)->calculateShortestPathTree(newTableIPv6, newTableIPv4);
            if (currInst->getArea(i)->getTransitCapability()) {
                hasTransitAreas = true;
            }
        }
        // 3) Inter-area routes are calculated by examining summary-LSAs (on backbone only)
        if (areaCount > 1) {
            Ospfv3Area *backbone = currInst->getAreaById(BACKBONE_AREAID);
            if (backbone != nullptr) {
                backbone->calculateInterAreaRoutes(newTableIPv6, newTableIPv4);
            }
        }
        else {
            if (areaCount == 1) {
                currInst->getArea(0)->calculateInterAreaRoutes(newTableIPv6, newTableIPv4);
            }
        }

        // 4) On BDR - Transit area LSAs(summary) are examined - find better paths then in 2) and 3)  TODO - this part of protocol is not supported yet
        /* if (hasTransitAreas) {
            for (i = 0; i < areaCount; i++) {
                if (currInst->getArea(i)->getTransitCapability()) {
                    if (currInst->getAddressFamily() == IPV6INSTANCE)
                        currInst->getArea(i)->recheckInterAreaPrefixLSAs(newTableIPv6);
                    else //IPV4INSTACE
                        currInst->getArea(i)->recheckInterAreaPrefixLSAs(newTableIPv4);
                }
            }
        }*/

        //5) Routes to external destinations are calculated TODO - this part of protocol is not supported yet
        // calculateASExternalRoutes(newTableIPv6, newTableIPv4);

        // backup the routing table
        unsigned long routeCount = routingTableIPv6.size();
        std::vector<Ospfv3RoutingTableEntry *> oldTableIPv6;
        std::vector<Ospfv3Ipv4RoutingTableEntry *> oldTableIPv4;

        if (currInst->getAddressFamily() == IPV6INSTANCE) {
            // IPv6 AF should not clear IPv4 Routing table and vice versa
            oldTableIPv6.assign(routingTableIPv6.begin(), routingTableIPv6.end());
            routingTableIPv6.clear();
            routingTableIPv6.assign(newTableIPv6.begin(), newTableIPv6.end());

            std::vector<Ipv6Route *> eraseEntriesIPv6;
            unsigned long routingEntryNumber = rt6->getNumRoutes();
            // remove entries from the IPv6 routing table inserted by the OSPF module
            for (i = 0; i < routingEntryNumber; i++) {
                Ipv6Route *entry = rt6->getRoute(i);
                if (entry->getSourceType() == IRoute::OSPF)
                    eraseEntriesIPv6.push_back(entry);
            }
            unsigned int eraseCount = eraseEntriesIPv6.size();
            for (i = 0; i < eraseCount; i++) {
                rt6->deleteRoute(eraseEntriesIPv6[i]);
            }
        }
        else {
            oldTableIPv4.assign(routingTableIPv4.begin(), routingTableIPv4.end());
            routingTableIPv4.clear();
            routingTableIPv4.assign(newTableIPv4.begin(), newTableIPv4.end());

            std::vector<Ipv4Route *> eraseEntriesIPv4;
            unsigned long routingEntryNumber = rt4->getNumRoutes();
            // remove entries from the IPv4 routing table inserted by the OSPF module
            for (i = 0; i < routingEntryNumber; i++) {
                Ipv4Route *entry = rt4->getRoute(i);
                if (entry->getSourceType() == IRoute::OSPF)
                    eraseEntriesIPv4.push_back(entry);
            }
            unsigned int eraseCount = eraseEntriesIPv4.size();
            for (i = 0; i < eraseCount; i++) {
                rt4->deleteRoute(eraseEntriesIPv4[i]);
            }
        }

        // add the new routing entries
        if (currInst->getAddressFamily() == IPV6INSTANCE) {
            routeCount = routingTableIPv6.size();
            EV_DEBUG  << "rebuild , routeCount - " << routeCount << "\n";

            for (i = 0; i < routeCount; i++) {
                if (routingTableIPv6[i]->getDestinationType() == Ospfv3RoutingTableEntry::NETWORK_DESTINATION) {
                    if (routingTableIPv6[i]->getNextHopCount() > 0) {
                        if (routingTableIPv6[i]->getNextHop(0).hopAddress != Ipv6Address::UNSPECIFIED_ADDRESS) {
                            Ipv6Route *route = new Ipv6Route(routingTableIPv6[i]->getDestinationAsGeneric().toIpv6(), routingTableIPv6[i]->getPrefixLength(), routingTableIPv6[i]->getSourceType());
                            route->setNextHop   (routingTableIPv6[i]->getNextHop(0).hopAddress);
                            route->setMetric    (routingTableIPv6[i]->getMetric());
                            route->setInterface (routingTableIPv6[i]->getInterface());
                            route->setExpiryTime(routingTableIPv6[i]->getExpiryTime());
                            route->setAdminDist (Ipv6Route::dOSPF);

                            rt6->addRoutingProtocolRoute(route);
                        }
                    }
                }
            }

            routeCount = oldTableIPv6.size();
            for (i = 0; i < routeCount; i++)
                delete (oldTableIPv6[i]);
        }
        else {
            routeCount = routingTableIPv4.size();
            for (i = 0; i < routeCount; i++) {
                if (routingTableIPv4[i]->getDestinationType() == Ospfv3Ipv4RoutingTableEntry::NETWORK_DESTINATION) {
                    if (routingTableIPv4[i]->getNextHopCount() > 0) {
                        if (routingTableIPv4[i]->getNextHop(0).hopAddress != Ipv4Address::UNSPECIFIED_ADDRESS) {
                            Ipv4Route *route = new Ipv4Route();
                            route->setDestination   (routingTableIPv4[i]->getDestinationAsGeneric().toIpv4());
                            route->setNetmask       (route->getDestination().makeNetmask(routingTableIPv4[i]->getPrefixLength()));
                            route->setSourceType    (routingTableIPv4[i]->getSourceType());
                            route->setNextHop       (routingTableIPv4[i]->getNextHop(0).hopAddress);
                            route->setMetric        (routingTableIPv4[i]->getMetric());
                            route->setInterface     (routingTableIPv4[i]->getInterface());
                            route->setAdminDist     ((Ipv4Route::dOSPF));
                            rt4->addRoute(route);
                        }
                    }
                }
            }

            routeCount = oldTableIPv4.size();
            for (i = 0; i < routeCount; i++)
               delete (oldTableIPv4[i]);
        }

        if (currInst->getAddressFamily() == IPV6INSTANCE) {
            EV_INFO << "Routing table was rebuilt.\n"
                    << "Results (IPv6):\n";

            routeCount = routingTableIPv6.size();
            for (i = 0; i < routeCount; i++)
                EV_INFO << *routingTableIPv6[i] << "\n";
        }
        else { //IPV4INSTANCE
            EV_INFO << "Routing table was rebuilt.\n"
                    << "Results (IPv4):\n";

            routeCount = routingTableIPv6.size();
            for (i = 0; i < routeCount; i++)
                EV_INFO << *routingTableIPv4[i] << "\n";
        }
    }
} // end of rebuildRoutingTable

} // namespace ospfv3
}//namespace inet

