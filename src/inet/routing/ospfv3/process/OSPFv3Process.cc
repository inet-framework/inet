#include "inet/routing/ospfv3/process/OSPFv3Process.h"

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet{

Define_Module(OSPFv3Process);

OSPFv3Process::OSPFv3Process()
{
//    this->processID = processID;
}

OSPFv3Process::~OSPFv3Process()
{
}

void OSPFv3Process::initialize(int stage){

    if(stage == INITSTAGE_ROUTING_PROTOCOLS){
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

        ageTimer = new cMessage();
        ageTimer->setKind(DATABASE_AGE_TIMER);
        ageTimer->setContextPointer(this);
        ageTimer->setName("OSPFv3Process::DatabaseAgeTimer");

        this->setTimer(ageTimer, 1.0);
    }
}
void OSPFv3Process::handleMessage(cMessage* msg)
{
    if(msg->isSelfMessage())
    {
        this->handleTimer(msg);
    }
    else
    {
        Packet *pk = check_and_cast<Packet *>(msg);
        const auto& packet = pk->peekAtFront<OSPFv3Packet>();

        auto protocol = pk->getTag<PacketProtocolTag>()->getProtocol();     //check if this is ICMPv6 msg
        if (protocol == &Protocol::icmpv6) {
           EV_ERROR << "ICMPv6 error received -- discarding\n";
           delete msg;
       }

        if(packet->getRouterID()==this->getRouterID()) //is it this router who originated the message?
            delete msg;
        else {
            OSPFv3Instance* instance = this->getInstanceById(packet->getInstanceID());
            if(instance == nullptr){//Is there an instance with this number?
                EV_DEBUG << "Instance with this ID not found, dropping\n";
                delete msg;//TODO - some warning??
            }
            else {
                instance->processPacket(pk);
            }
        }
    }
}//handleMessage

/*return index of the Ipv4 table if the route is found, -1 else*/
int OSPFv3Process::isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv4Route *entry = rtTable->getRoute(i);
        if (Ipv4Address::maskedAddrAreEqual(addr, entry->getDestination(), entry->getNetmask())) {
            return i;
        }
    }
    return -1;
}
int OSPFv3Process::isInRoutingTable6(Ipv6RoutingTable *rtTable, Ipv6Address addr)
{
    for (int i = 0; i < rtTable->getNumRoutes(); i++) {
        const Ipv6Route *entry = rtTable->getRoute(i);
        if (addr.getPrefix(entry->getPrefixLength()) ==  entry->getDestPrefix().getPrefix(entry->getPrefixLength()))
        {
            return i;
        }
    }
    return -1;
}

int OSPFv3Process::isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (ifTable->getInterface(i)->ipv4Data()->getIPAddress() == addr) {
            return i;
        }
    }
    return -1;
}

int OSPFv3Process::isInInterfaceTable6(IInterfaceTable *ifTable, Ipv6Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        for (int j = 0; j < ifTable->getInterface(i)->ipv6Data()->getNumAddresses(); j++) {
            if (ifTable->getInterface(i)->ipv6Data()->getAddress(j) == addr) {
                return i;
            }
        }
    }
    return -1;
}

void OSPFv3Process::parseConfig(cXMLElement* interfaceConfig)
{
    EV_DEBUG << "Parsing config on process " << this->processID << endl;
    //Take each interface
    cXMLElementList intList = interfaceConfig->getElementsByTagName("Interface");
    for(auto interfaceIt=intList.begin(); interfaceIt!=intList.end(); interfaceIt++)
    {

        const char* interfaceName = (*interfaceIt)->getAttribute("name");
        InterfaceEntry *myInterface = (ift->getInterfaceByName(interfaceName));

        if (myInterface->isLoopback()) {
            const char * ipv41 = "127.0.0.0";
            Ipv4Address tmpipv4;
            tmpipv4.set(ipv41);
            int i = isInRoutingTable(rt4, tmpipv4);
            if (i != -1){
                rt4->deleteRoute(rt4->getRoute(i));
            }
        }
        //interface ipv6 configuration
        cXMLElementList ipAddrList = (*interfaceIt)->getElementsByTagName("Ipv6Address");
        for (auto & ipv6Rec : ipAddrList)
        {
            const char * addr6c = ipv6Rec->getNodeValue();

            std::string add6 = addr6c;
            std::string prefix6 = add6.substr(0, add6.find("/"));
            Ipv6InterfaceData * intfData6 = myInterface->ipv6Data();
            int prefLength;
            Ipv6Address address6;
            if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
                 throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

            address6 = Ipv6Address(prefix6.c_str());

            Ipv6InterfaceData::AdvPrefix p;
            p.prefix = address6;
            p.prefixLength = prefLength;

            if(isInInterfaceTable6(ift, address6) < 0)
            {
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
         Ipv4InterfaceData * intfData = myInterface->ipv4Data(); //new Ipv4InterfaceData();
         bool alreadySet = false;


         cXMLElementList ipv4AddrList = (*interfaceIt)->getElementsByTagName("IPAddress");
         if (ipv4AddrList.size() == 1)
         {
             for (auto & ipv4Rec : ipv4AddrList)
             {
                 const char * addr4c = ipv4Rec->getNodeValue(); //from string make ipv4 address and store to interface config
                 addr = (Ipv4Address(addr4c));
                 if (isInInterfaceTable(ift, addr) >= 0) // prevention from seting same interface by second process
                 {
                     alreadySet = true;
                     continue;
                 }
                 intfData->setIPAddress(addr);

             }
             if (!alreadySet)
             {
                 cXMLElementList ipv4MaskList = (*interfaceIt)->getElementsByTagName("Mask");
                 if (ipv4MaskList.size() != 1)
                     throw cRuntimeError("Interface %s has more or less than one mask", interfaceName);

                 for (auto & ipv4Rec : ipv4MaskList)
                 {
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
        OSPFv3Interface::OSPFv3InterfaceType interfaceTypeNum;
        bool passiveInterface = false;


        cXMLElementList process = (*interfaceIt)->getElementsByTagName("Process");
        if(process.size()>2)
            throw cRuntimeError("More than two processes configured for interface %s", (*interfaceIt)->getAttribute("name"));

        //Check whether it belongs to this process
        int processCount = process.size();
        for(int i = 0; i < processCount; i++){
            int procId = atoi(process.at(i)->getAttribute("id"));
            if(procId != this->processID)
                continue;

            EV_DEBUG << "Creating new interface "  << interfaceName << " in process " << procId << endl;

            //Parsing instances
            cXMLElementList instList = process.at(i)->getElementsByTagName("Instance");
            for(auto instIt=instList.begin(); instIt!=instList.end(); instIt++)
            {
                const char* instId = (*instIt)->getAttribute("instanceID");
                const char* addressFamily = (*instIt)->getAttribute("AF");

                //Get the router priority for this interface and instance
                cXMLElementList interfaceOptions = (*instIt)->getElementsByTagName("RouterPriority");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple router priority is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    routerPriority = interfaceOptions.at(0)->getNodeValue();

                //get the hello interval for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("HelloInterval");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple HelloInterval value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    helloInterval = interfaceOptions.at(0)->getNodeValue();

                //get the dead interval for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("DeadInterval");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple DeadInterval value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    deadInterval = interfaceOptions.at(0)->getNodeValue();

                //get the interface cost for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("InterfaceCost");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple InterfaceCost value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    interfaceCost = interfaceOptions.at(0)->getNodeValue();

                //get the interface cost for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("InterfaceType");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple InterfaceType value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0){
                    interfaceType = interfaceOptions.at(0)->getNodeValue();
                    if(strcmp(interfaceType, "Broadcast")==0)
                        interfaceTypeNum = OSPFv3Interface::BROADCAST_TYPE;
                    else if(strcmp(interfaceType, "PointToPoint")==0)
                        interfaceTypeNum = OSPFv3Interface::POINTTOPOINT_TYPE;
                    else if(strcmp(interfaceType, "NBMA")==0)
                        interfaceTypeNum = OSPFv3Interface::NBMA_TYPE;
                    else if(strcmp(interfaceType, "PointToMultipoint")==0)
                        interfaceTypeNum = OSPFv3Interface::POINTTOMULTIPOINT_TYPE;
                    else if(strcmp(interfaceType, "Virtual")==0)
                        interfaceTypeNum = OSPFv3Interface::VIRTUAL_TYPE;
                    else
                        interfaceTypeNum = OSPFv3Interface::UNKNOWN_TYPE;
                }
                else
                    throw cRuntimeError("Interface type needs to be specified for interface %s", interfaceName);

                //find out whether the interface is passive
                interfaceOptions = (*instIt)->getElementsByTagName("PassiveInterface");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple PassiveInterface value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0){
                    if(strcmp(interfaceOptions.at(0)->getNodeValue(), "True")==0)
                        passiveInterface = true;
                }



                int instIdNum;

                if(instId==nullptr) {
                    EV_DEBUG << "Address Family " << addressFamily << endl;
                    if(strcmp(addressFamily, "IPv4")==0) {
                        EV_DEBUG << "IPv4 instance\n";
                        instIdNum = DEFAULT_IPV4_INSTANCE;
                    }
                    else if(strcmp(addressFamily, "IPv6")==0) {
                        EV_DEBUG << "IPv6 instance\n";
                        instIdNum = DEFAULT_IPV6_INSTANCE;
                    }
                    else
                        throw cRuntimeError("Unknown address family in process %d", this->getProcessID());
                }
                else
                    instIdNum = atoi(instId);

                //TODO - check range of instance ID
                //check for multiple definition of one instance
                OSPFv3Instance* instance = this->getInstanceById(instIdNum);
                //if(instance != nullptr)
                // throw cRuntimeError("Multiple OSPFv3 instance with the same instance ID configured for process %d on interface %s", this->getProcessID(), interfaceName);

                if(instance == nullptr) {
                    if(strcmp(addressFamily, "IPv4")==0)
                        instance = new OSPFv3Instance(instIdNum, this, IPV4INSTANCE);
                    else
                        instance = new OSPFv3Instance(instIdNum, this, IPV6INSTANCE);

                    EV_DEBUG << "Adding instance " << instIdNum << " to process " << this->processID << endl;
                    this->addInstance(instance);
                }

                //TODO - multiarea configuration??
                cXMLElementList areasList = (*instIt)->getElementsByTagName("Area");
                for(auto areasIt=areasList.begin(); areasIt!=areasList.end(); areasIt++)
                {
                    const char* areaId = (*areasIt)->getNodeValue();
                    Ipv4Address areaIP = Ipv4Address(areaId);
                    const char* areaType = (*areasIt)->getAttribute("type");
                    OSPFv3AreaType type = NORMAL;

                    if(areaType != nullptr){
                        if(strcmp(areaType, "stub") == 0)
                            type = STUB;
                        else if(strcmp(areaType, "nssa") == 0)
                            type = NSSA;
                    }

                    const char* summary = (*areasIt)->getAttribute("summary");

                    if(summary != nullptr){
                        if(strcmp(summary, "no") == 0) {
                            if(type == STUB)
                                type = TOTALLY_STUBBY;
                            else if(type == NSSA)
                                type = NSSA_TOTALLY_STUB;
                        }
                    }

                    //insert area if it's not already there and assign this interface
                    OSPFv3Area* area;
                    if(!(instance->hasArea(areaIP))) {
                        area = new OSPFv3Area(areaIP, instance, type);
                        instance->addArea(area);
                    }
                    else
                        area = instance->getAreaById(areaIP);

                    if(!area->hasInterface(std::string(interfaceName)))
                    {
                        OSPFv3Interface* newInterface = new OSPFv3Interface(interfaceName, this->containingModule, this, interfaceTypeNum, passiveInterface);
                        if(helloInterval!=nullptr)
                            newInterface->setHelloInterval(atoi(helloInterval));

                        if(deadInterval!=nullptr)
                            newInterface->setDeadInterval(atoi(deadInterval));

                        if(interfaceCost!=nullptr)
                            newInterface->setInterfaceCost(atoi(interfaceCost));

                        if(routerPriority!=nullptr) {
                            int rtrPrio = atoi(routerPriority);
                            if(rtrPrio < 0 || rtrPrio > 255)
                                throw cRuntimeError("Router priority out of range on interface %s", interfaceName);

                            newInterface->setRouterPriority(rtrPrio);
                        }

                        newInterface->setArea(area);

                        cXMLElementList ipAddrList = (*interfaceIt)->getElementsByTagName("Ipv6Address");
                        for (auto & ipv6Rec : ipAddrList)
                        {
                            const char * addr6c = ipv6Rec->getNodeValue();
                            std::string add6 = addr6c;
                            std::string prefix6 = add6.substr(0, add6.find("/"));
                            Ipv6InterfaceData * intfData6 = myInterface->ipv6Data();
                            int prefLength;
                            Ipv6Address address6;
                            if (!(address6.tryParseAddrWithPrefix(addr6c, prefLength)))
                                 throw cRuntimeError("Cannot parse Ipv6 address: '%s", addr6c);

                            address6 = Ipv6Address(prefix6.c_str());

                            Ipv6AddressRange ipv6addRange; //add directly networks into addressRange for given area
                            ipv6addRange.prefix = address6; //add only network prefix
                            ipv6addRange.prefixLength = prefLength;
                            area->addAddressRange(ipv6addRange, true); //TODO:  add tag Advertise and exclude link-local (?)
                        }
                        cXMLElementList ipv4AddrList = (*interfaceIt)->getElementsByTagName("IPAddress");
                        if (ipv4AddrList.size() == 1)
                        {
                            Ipv4AddressRange ipv4addRange; // also create addressRange for IPv4
                            for (auto & ipv4Rec : ipv4AddrList)
                            {
                                const char * addr4c = ipv4Rec->getNodeValue(); //from string make ipv4 address and store to interface config
                                ipv4addRange.address = Ipv4Address(addr4c);
                            }

                            cXMLElementList ipv4MaskList = (*interfaceIt)->getElementsByTagName("Mask");
                            if (ipv4MaskList.size() != 1)
                                throw cRuntimeError("Interface %s has more or less than one mask", interfaceName);

                            for (auto & ipv4Rec : ipv4MaskList)
                            {
                                const char * mask4c = ipv4Rec->getNodeValue();
                                ipv4addRange.mask = Ipv4Address(mask4c);
                            }
                            area->addAddressRange(ipv4addRange, true); //TODO:  add tag Advertise and exclude link-local (?)
                        }
                       EV_DEBUG << "I am " << this->getOwner()->getOwner()->getName() << " on int " << newInterface->getInterfaceLLIP() << " with area " << area->getAreaID() <<"\n";
                        area->addInterface(newInterface);
                    }
                }
            }
        }
    }
}//parseConfig


void OSPFv3Process::ageDatabase()
{
    bool shouldRebuildRoutingTable = false;

    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++)
    {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++)
            instances[i]->getArea(j)->ageDatabase();

        if (shouldRebuildRoutingTable)
            rebuildRoutingTable();
    }
} // ageDatabase

// for IPv6 AF
bool OSPFv3Process::hasAddressRange(const Ipv6AddressRange& addressRange) const
{
    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++)
    {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++)
        {
           if(instances[i]->getArea(j)->hasAddressRange(addressRange))
               return true;
        }
    }
    return false;
}
// for IPv4 AF
bool OSPFv3Process::hasAddressRange(const Ipv4AddressRange& addressRange) const
{
    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++)
    {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++)
        {
           if(instances[i]->getArea(j)->hasAddressRange(addressRange))
           {
               return true;
           }
        }
    }
    return false;
}

void OSPFv3Process::handleTimer(cMessage* msg)
{
    switch(msg->getKind())
    {
        case INIT_PROCESS:
            for(auto it=this->instances.begin(); it!=this->instances.end(); it++)
                (*it)->init();

            this->debugDump();
        break;

        case HELLO_TIMER:
        {
            OSPFv3Interface* interface;
            if(!(interface=reinterpret_cast<OSPFv3Interface*>(msg->getContextPointer())))
            {
                //TODO - error
            }
            else {
                EV_DEBUG << "Process received msg, sending event HELLO_TIMER_EVENT\n";
                interface->processEvent(OSPFv3Interface::HELLO_TIMER_EVENT);
            }
        }
        break;

        case WAIT_TIMER:
        {
            OSPFv3Interface* interface;
            if(!(interface=reinterpret_cast<OSPFv3Interface*>(msg->getContextPointer())))
            {
                //TODO - error
            }
            else {
                EV_DEBUG << "Process received msg, sending event WAIT_TIMER_EVENT\n";
                interface->processEvent(OSPFv3Interface::WAIT_TIMER_EVENT);
            }
        }
        break;

        case ACKNOWLEDGEMENT_TIMER: {
            OSPFv3Interface *intf;
            if (!(intf = reinterpret_cast<OSPFv3Interface *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid InterfaceAcknowledgementTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Acknowledgement Timer expired", intf);
                intf->processEvent(OSPFv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT);
            }
        }
        break;

        case NEIGHBOR_INACTIVITY_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Inactivity Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::INACTIVITY_TIMER);
                OSPFv3Interface* intf = neighbor->getInterface();
                int neighborCnt = intf->getNeighborCount();
                for(int i=0; i<neighborCnt; i++){
                    OSPFv3Neighbor* currNei = intf->getNeighbor(i);
                    if(currNei->getNeighborID() == neighbor->getNeighborID()){
//                        neighbor->processEvent(OSPFv3Neighbor::INACTIVITY_TIMER);
                        intf->removeNeighborByID(neighbor->getNeighborID());
                        break;
                    }
                }

                intf->processEvent(OSPFv3Interface::NEIGHBOR_CHANGE_EVENT);
            }
        }
        break;

        case NEIGHBOR_POLL_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Poll Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::POLL_TIMER);
            }
        }
        break;

        case NEIGHBOR_DD_RETRANSMISSION_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborDDRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Database Description Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::DD_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_UPDATE_RETRANSMISSION_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Update Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::UPDATE_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_REQUEST_RETRANSMISSION_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Request Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::REQUEST_RETRANSMISSION_TIMER);
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

void OSPFv3Process::setTimer(cMessage* msg, double delay = 0)
{
    scheduleAt(simTime()+delay, msg);
}

void OSPFv3Process::activateProcess()
{
    Enter_Method_Silent();
    this->isActive=true;
    cMessage* init = new cMessage();
    init->setKind(HELLO_TIMER);
    scheduleAt(simTime(), init);
}//activateProcess

void OSPFv3Process::debugDump()
{
    EV_DEBUG << "Process " << this->getProcessID() << "\n";
    for(auto it=this->instances.begin(); it!=this->instances.end(); it++)
        (*it)->debugDump();
}//debugDump

OSPFv3Instance* OSPFv3Process::getInstanceById(int instanceId)
{
    std::map<int, OSPFv3Instance*>::iterator instIt = this->instancesById.find(instanceId);
    if(instIt == this->instancesById.end())
        return nullptr;

    return instIt->second;
}

void OSPFv3Process::addInstance(OSPFv3Instance* newInstance)
{
    OSPFv3Instance* check = this->getInstanceById(newInstance->getInstanceID());
    if(check==nullptr){
        this->instances.push_back(newInstance);
        this->instancesById[newInstance->getInstanceID()]=newInstance;
    }
}

void OSPFv3Process::sendPacket(Packet *packet, Ipv6Address destination, const char* ifName, short hopLimit)
{
    InterfaceEntry *ie = this->ift->getInterfaceByName(ifName);
    Ipv6InterfaceData *ipv6int = ie->ipv6Data();

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ospfv3);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    packet->addTagIfAbsent<L3AddressReq>()->setDestAddress(destination);
    packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(ipv6int->getLinkLocalAddress());
    packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(hopLimit);
    const auto& ospfPacket = packet->peekAtFront<OSPFv3Packet>();

    switch (ospfPacket->getType())
    {
        case HELLO_PACKET:
        {
            packet->setName("OSPFv3_HelloPacket");
//            const auto& helloPacket = packet->peekAtFront<OSPFv3HelloPacket>();
//            printHelloPacket(helloPacket.get(), destination, outputIfIndex);
        }
        break;
        case DATABASE_DESCRIPTION:
        {
            packet->setName("OSPFv3_DDPacket");
//            const auto& ddPacket = packet->peekAtFront<OSPFv3DatabaseDescription>();
//            printDatabaseDescriptionPacket(ddPacket.get(), destination, outputIfIndex);
        }
        break;

        case LSR:
        {
            packet->setName("OSPFv3_LSRPacket");
//            const auto& requestPacket = packet->peekAtFront<OSPFv3LinkStateRequest>();
//            printLinkStateRequestPacket(requestPacket.get(), destination, outputIfIndex);
        }
        break;

        case LSU:
        {
            packet->setName("OSPFv3_LSUPacket");
//            const auto& updatePacket = packet->peekAtFront<OSPFv3LSUpdate>();
//            printLinkStateUpdatePacket(updatePacket.get(), destination, outputIfIndex);
        }
        break;

        case LS_ACK:
        {
            packet->setName("OSPFv3_LSAckPacket");
//            const auto& ackPacket = packet->peekAtFront<OSPFv3LSAck>();
//            printLinkStateAcknowledgementPacket(ackPacket.get(), destination, outputIfIndex);
        }
        break;

        default:
            break;
    }
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    this->send(packet, "splitterOut");
}//sendPacket

OSPFv3LSA* OSPFv3Process::findLSA(LSAKeyType lsaKey, Ipv4Address areaID, int instanceID)
{
    OSPFv3Instance* instance = this->getInstanceById(instanceID);
    OSPFv3Area* area = instance->getAreaById(areaID);
    return area->getLSAbyKey(lsaKey);
}

bool OSPFv3Process::floodLSA(const OSPFv3LSA* lsa, Ipv4Address areaID, OSPFv3Interface* interface, OSPFv3Neighbor* neighbor)
{
    EV_DEBUG << "Flooding LSA from router " << lsa->getHeader().getAdvertisingRouter() << " with ID " << lsa->getHeader().getLinkStateID() << "\n";
    bool floodedBackOut = false;

    if (lsa != nullptr) {
        OSPFv3Instance* instance = interface->getArea()->getInstance();
        if (lsa->getHeader().getLsaType() == AS_EXTERNAL_LSA) {
            long areaCount = instance->getAreaCount();
            for (long i = 0; i < areaCount; i++) {
                OSPFv3Area* area = instance->getArea(i);
                if (area->getExternalRoutingCapability()) {
                    if (area->floodLSA(lsa, interface, neighbor)) {
                        floodedBackOut = true;
                    }
                }
            }
        }
        else {
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area != nullptr) {
                floodedBackOut = area->floodLSA(lsa, interface, neighbor);
            }
        }
    }

    return floodedBackOut;
}

bool OSPFv3Process::installLSA(const OSPFv3LSA *lsaC, int instanceID, Ipv4Address areaID    /*= BACKBONE_AREAID*/, OSPFv3Interface* intf)
{
    auto lsa = lsaC->dup(); // make editable copy of lsa
    EV_DEBUG << "OSPFv3Process::installLSA\n";
    switch (lsa->getHeader().getLsaType()) {
        case ROUTER_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3RouterLSA *ospfRouterLSA = check_and_cast<OSPFv3RouterLSA *>(lsa);
                return area->installRouterLSA(ospfRouterLSA);
            }
        }
        break;

        case NETWORK_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3NetworkLSA *ospfNetworkLSA = check_and_cast<OSPFv3NetworkLSA *>(lsa);
                return area->installNetworkLSA(ospfNetworkLSA);
            }
        }
        break;

        case INTER_AREA_PREFIX_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3InterAreaPrefixLSA *ospfInterAreaLSA = check_and_cast<OSPFv3InterAreaPrefixLSA *>(lsa);
                return area->installInterAreaPrefixLSA(ospfInterAreaLSA);
            }
        }
        break;

        case LINK_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3LinkLSA *ospfLinkLSA = check_and_cast<OSPFv3LinkLSA *>(lsa);
                return intf->installLinkLSA(ospfLinkLSA);
            }
        }
        break;

        case INTRA_AREA_PREFIX_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if(area!=nullptr) {
                OSPFv3IntraAreaPrefixLSA* intraLSA = check_and_cast<OSPFv3IntraAreaPrefixLSA *>(lsa);
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

void OSPFv3Process::calculateASExternalRoutes(std::vector<OSPFv3RoutingTableEntry* > newTableIPv6, std::vector<OSPFv3IPv4RoutingTableEntry* > newTableIPv4)
{
    EV_DEBUG << "Calculating AS External Routes (Not ymplemented yet)\n";
}

void OSPFv3Process::rebuildRoutingTable()
{
    unsigned long instanceCount = this->instances.size();
    std::vector<OSPFv3RoutingTableEntry *> newTableIPv6;
    std::vector<OSPFv3IPv4RoutingTableEntry *> newTableIPv4;

    for(unsigned int k=0; k<instanceCount; k++) {
        OSPFv3Instance* currInst = this->instances.at(k);
        unsigned long areaCount = currInst->getAreaCount();
        bool hasTransitAreas = false;

        unsigned long i;

        EV_INFO << "Rebuilding routing table for instance " << this->instances.at(k)->getInstanceID() << ":\n";

        //2)Intra area routes are calculated using SPF algo
        for (i = 0; i < areaCount; i++) {
            currInst->getArea(i)->calculateShortestPathTree(newTableIPv6, newTableIPv4);
            if (currInst->getArea(i)->getTransitCapability()) {
                hasTransitAreas = true;
            }
        }
//        3)Inter-area routes are calculated by examining summary-LSAs (on backbone only)
        if (areaCount > 1) {
            OSPFv3Area *backbone = currInst->getAreaById(BACKBONE_AREAID);
            if (backbone != nullptr) {
                backbone->calculateInterAreaRoutes(newTableIPv6, newTableIPv4);
            }
        }
        else {
            if (areaCount == 1) {
                currInst->getArea(0)->calculateInterAreaRoutes(newTableIPv6, newTableIPv4);
            }
        }

        //4)On BDR - Transit area LSAs(summary) are examined - find better paths then in 2) and 3)  TODO
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

        //5) Routes to external destinations are calculated TODO
       // calculateASExternalRoutes(newTableIPv6, newTableIPv4);

        // backup the routing table
        unsigned long routeCount = routingTableIPv6.size();
        std::vector<OSPFv3RoutingTableEntry *> oldTableIPv6;
        std::vector<OSPFv3IPv4RoutingTableEntry *> oldTableIPv4;

        if (currInst->getAddressFamily() == IPV6INSTANCE) // IPv6 AF should not clear IPv4 Routing table and vice versa
        {

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
        else
        {
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
        if (currInst->getAddressFamily() == IPV6INSTANCE)
        {
            routeCount = routingTableIPv6.size();
            EV_DEBUG  << "rebuild , routeCount - " << routeCount << "\n";

            for (i = 0; i < routeCount; i++) {
                if (routingTableIPv6[i]->getDestinationType() == OSPFv3RoutingTableEntry::NETWORK_DESTINATION) {

                    if (routingTableIPv6[i]->getNextHopCount() > 0)
                    {
                        if (routingTableIPv6[i]->getNextHop(0).hopAddress != Ipv6Address::UNSPECIFIED_ADDRESS)
                        {
                            Ipv6Route *route = new Ipv6Route(routingTableIPv6[i]->getDestinationAsGeneric().toIpv6(), routingTableIPv6[i]->getPrefixLength(), routingTableIPv6[i]->getSourceType());
                            route->setNextHop   (routingTableIPv6[i]->getNextHop(0).hopAddress);
                            route->setMetric    (routingTableIPv6[i]->getMetric());
                            route->setInterface (routingTableIPv6[i]->getInterface());
                            route->setExpiryTime(routingTableIPv6[i]->getExpiryTime());
//                            route->setAdminDist (routingTableIPv6[i]->getAdminDist());
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
        else
        {
            routeCount = routingTableIPv4.size();
            for (i = 0; i < routeCount; i++) {
                if (routingTableIPv4[i]->getDestinationType() == OSPFv3IPv4RoutingTableEntry::NETWORK_DESTINATION)
                {
                    if (routingTableIPv4[i]->getNextHopCount() > 0)
                    {
                        if (routingTableIPv4[i]->getNextHop(0).hopAddress != Ipv4Address::UNSPECIFIED_ADDRESS)
                        {
                            Ipv4Route *route = new Ipv4Route();
                            route->setDestination   (routingTableIPv4[i]->getDestinationAsGeneric().toIpv4());
                            route->setNetmask       (route->getDestination().makeNetmask(routingTableIPv4[i]->getPrefixLength()));
                            route->setSourceType    (routingTableIPv4[i]->getSourceType());
                            route->setNextHop       (routingTableIPv4[i]->getNextHop(0).hopAddress);
                            route->setMetric        (routingTableIPv4[i]->getMetric());
                            route->setInterface     (routingTableIPv4[i]->getInterface());
//                            route->setAdminDist     (routingTableIPv4[i]->getAdminDist());
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

        if (currInst->getAddressFamily() == IPV6INSTANCE)
        {
            EV_INFO << "Routing table was rebuilt.\n"
                    << "Results (IPv6):\n";

            routeCount = routingTableIPv6.size();
            for (i = 0; i < routeCount; i++)
                EV_INFO << *routingTableIPv6[i] << "\n";

        }
        else //IPV4INSTANCE
        {
            EV_INFO << "Routing table was rebuilt.\n"
                    << "Results (IPv4):\n";

            routeCount = routingTableIPv6.size();
            for (i = 0; i < routeCount; i++)
                EV_INFO << *routingTableIPv4[i] << "\n";

        }
    }
} // end of rebuildRoutingTable

}//namespace inet


