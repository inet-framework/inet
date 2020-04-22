//
// Copyright (C) 2008
// DSDV simple example for INET (add-on)
// Version 2.0
// Diogo Antio & Pedro Menezes
// Instituto Superior Tecnico
// Lisboa - Portugal
// This version and newer version can be found at http://dsdv.8rf.com
// This code was written while assisting the course "Redes moveis e sem fios" http://comp.ist.utl.pt/ec-cm
// Autorization to use and modify this code not needed :P
// The authors hope it will be useful to help understand how
// INET and OMNET++ works(more specifically INET 20061020 and omnet++ 3.3).
// Also we hope it will help in the developing of routing protocols using INET.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/dsdv/Dsdv.h"

namespace inet {

Define_Module(Dsdv);

Dsdv::ForwardEntry::~ForwardEntry()
{
    if (this->event!=nullptr) delete this->event;
    if (this->hello!=nullptr) delete this->hello;
}

Dsdv::Dsdv()
{
}

Dsdv::~Dsdv()
{
    stop();
    // Dispose of dynamically allocated the objects
    delete event;
    delete forwardList;
    //delete Hello;
}

void Dsdv::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    //reads from omnetpp.ini
    if (stage == INITSTAGE_LOCAL)
    {
        sequencenumber = 0;
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);

        routeLifetime = par("routeLifetime").doubleValue();
        helloInterval = par("helloInterval");
        forwardList = new std::list<ForwardEntry *>();
        event = new cMessage("event");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        registerProtocol(Protocol::manet, gate("ipOut"), gate("ipIn"));
    }
}

void Dsdv::start()
{
    /* Search the 80211 interface */
    int  num_80211 = 0;
    InterfaceEntry *ie;
    InterfaceEntry *i_face;
    const char *name;
    broadcastDelay = &par("broadcastDelay");
    for (int i = 0; i < ift->getNumInterfaces(); i++)
    {
        ie = ift->getInterface(i);
        name = ie->getInterfaceName();
        if (strstr(name, "wlan") != nullptr)
        {
            i_face = ie;
            num_80211++;
            interfaceId = i;
        }
    }

    // One enabled network interface (in total)
    if (num_80211 == 1)
        interface80211ptr = i_face;
    else
        throw cRuntimeError("DSDV has found %i 80211 interfaces", num_80211);
    if (par("manetPurgeRoutingTables").boolValue())
    {
        Ipv4Route *entry;
        // clean the route table wlan interface entry
        for (int i = rt->getNumRoutes() - 1; i >= 0; i--)
        {
            entry = rt->getRoute(i);
            const InterfaceEntry *ie = entry->getInterface();
            if (strstr(ie->getInterfaceName(), "wlan") != nullptr)
                rt->deleteRoute(entry);
        }
    }
    CHK(interface80211ptr->getProtocolData<Ipv4InterfaceData>())->joinMulticastGroup(Ipv4Address::LL_MANET_ROUTERS);

    // schedules a random periodic event: the hello message broadcast from DSDV module

    //reads from omnetpp.ini
    //HelloForward = new DsdvHello("HelloForward");
    // schedules a random periodic event: the hello message broadcast from DSDV module
    scheduleAt(simTime() + uniform(0.0, par("maxVariance").doubleValue()), event);
}

void Dsdv::stop()
{
    cancelEvent(event);
    while (!forwardList->empty())
    {
        ForwardEntry *fh = forwardList->front();
        if (fh->event)
            cancelAndDelete(fh->event);
        if (fh->hello)
            cancelAndDelete(fh->hello);
        fh->event = nullptr;
        fh->hello = nullptr;
        forwardList->pop_front();
        delete fh;
    }
}

void Dsdv::handleSelfMessage(cMessage *msg)
{
    // When DSDV module receives selfmessage (scheduled event)
    // it means that it's time for Hello message broadcast event
    // i.e. Brodcast Hello messages to other nodes when selfmessage=event
    // But if selmessage!=event it means that it is time to forward useful Hello message to othert nodes
    if (msg == event)
    {
        auto hello = makeShared<DsdvHello>();

        rt->purge();

        // count non-loopback interfaces
        // int numIntf = 0;
        // InterfaceEntry *ie = nullptr;
        //for (int k=0; k<ift->getNumInterfaces(); k++)
        //  if (!ift->getInterface(k)->isLoopback())
        //  {ie = ift->getInterface(k); numIntf++;}

        // Filling the DsdvHello fields
        // Ipv4Address source = (ie->ipv4()->getIPAddress());
        Ipv4Address source = (interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        hello->setChunkLength(b(128)); ///size of Hello message in bits
        hello->setSrcAddress(source);
        sequencenumber += 2;
        hello->setSequencenumber(sequencenumber);
        hello->setNextAddress(source);
        hello->setHopdistance(1);

        /*http://www.cs.ucsb.edu/~ebelding/txt/bc.txt
        The IPv4 address for "limited broadcast" is 255.255.255.255, and is not supposed to be forwarded.
        Since the nodes in an ad hoc network are asked to forward the flooded packets, the limited broadcast
        address is a poor choice.  The other available choice, the "directed broadcast" address, would presume a
        choice of routing prefix for the ad hoc network and thus is not a reasonable choice.
        (...)
        Limited Broadcast - Sent to all NICs on the some network segment as the source NIC. It is represented with
        the 255.255.255.255 TCP/IP address. This broadcast is not forwarded by routers so will only appear on one
        network segment.
        Direct broadcast - Sent to all hosts on a network. Routers may be configured to forward directed broadcasts
        on large networks. For network 192.168.0.0, the broadcast is 192.168.255.255.
        */
        //new control info for DsdvHello
        auto packet = new Packet("Hello", hello);
        auto addressReq = packet->addTag<L3AddressReq>();
        addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255)); //let's try the limited broadcast 255.255.255.255 but multicast goes from 224.0.0.0 to 239.255.255.255
        addressReq->setSrcAddress(source); //let's try the limited broadcast
        packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

        //broadcast to other nodes the hello message
        send(packet, "ipOut");
        packet = nullptr;
        hello = nullptr;

        //schedule new brodcast hello message event
        scheduleAt(simTime()+helloInterval+broadcastDelay->doubleValue(), event);
        bubble("Sending new hello message");
    }
    else
    {
        for (auto it = forwardList->begin(); it != forwardList->end(); it++)
        {
            if ( (*it)->event == msg )
            {
                EV << "Vou mandar forward do " << (*it)->hello->peekData<DsdvHello>()->getSrcAddress() << endl; // todo
                send((*it)->hello, "ipOut");
                (*it)->hello = nullptr;
                delete (*it);
                forwardList->erase(it);
                break;
            }
        }
        delete msg;
    }
}

void Dsdv::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleSelfMessage(msg);
    }
    else if (check_and_cast<Packet *>(msg)->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::manet)
    {
        auto packet = new Packet("Hello");

        // When DSDV module receives DsdvHello from other host
        // it adds/replaces the information in routing table for the one contained in the message
        // but only if it's useful/up-to-date. If not the DSDV module ignores the message.
        auto addressReq = packet->addTag<L3AddressReq>();
        addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255)); //let's try the limited broadcast 255.255.255.255 but multicast goes from 224.0.0.0 to 239.255.255.255
        addressReq->setSrcAddress(interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        ForwardEntry *fhp = nullptr;
        auto recHello = !isForwardHello ? staticPtrCast<DsdvHello>(check_and_cast<Packet *>(msg)->peekData<DsdvHello>()->dupShared()) : nullptr;
        if (isForwardHello) {
            fhp = new ForwardEntry();
            fhp->hello = check_and_cast<Packet *>(msg->dup());
        }

        if (msg->arrivedOn("ipIn")) {
            ASSERT((!isForwardHello && recHello) || (isForwardHello && fhp->hello));
            bubble("Received hello message");
            Ipv4Address source = interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();

            //reads DSDV hello message fields
            Ipv4Address src;
            unsigned int msgsequencenumber;
            int numHops;
            Ipv4Address next;
            if (!isForwardHello) {
                src = recHello->getSrcAddress();
                msgsequencenumber = recHello->getSequencenumber();
                next = recHello->getNextAddress();
                numHops = recHello->getHopdistance();

                if (src == source) {
                    EV_INFO << "Hello msg dropped. This message returned to the original creator.\n";
                    delete packet;
                    delete fhp;
                    delete msg;
                    return;
                }
            }
            else {
                auto fhpHello = fhp->hello->peekData<DsdvHello>();
                src = fhpHello->getSrcAddress();
                msgsequencenumber = fhpHello->getSequencenumber();
                next = fhpHello->getNextAddress();
                numHops = fhpHello->getHopdistance();

                if (src == source) {
                    EV_INFO << "Hello msg dropped. This message returned to the original creator.\n";
                    delete packet;
                    delete fhp;
                    delete msg;
                    return;
                }
            }

            Ipv4Route *_entrada_routing = rt->findBestMatchingRoute(src);
            DsdvIpv4Route *entrada_routing = dynamic_cast<DsdvIpv4Route *>(_entrada_routing);

            //Tests if the DSDV hello message that arrived is useful
            if (_entrada_routing == nullptr
                    || (_entrada_routing != nullptr && _entrada_routing->getNetmask() != Ipv4Address::ALLONES_ADDRESS)
                    || (entrada_routing != nullptr && (msgsequencenumber>(entrada_routing->getSequencenumber()) || (msgsequencenumber == (entrada_routing->getSequencenumber()) && numHops < (entrada_routing->getMetric())))))
            {

                //remove old entry
                if (entrada_routing != nullptr)
                    rt->deleteRoute(entrada_routing);

                //adds new information to routing table according to information in hello message
                {
                    Ipv4Address netmask = Ipv4Address::ALLONES_ADDRESS; // Ipv4Address(par("netmask").stringValue());
                    DsdvIpv4Route *e = new DsdvIpv4Route();
                    e->setDestination(src);
                    e->setNetmask(netmask);
                    e->setGateway(next);
                    e->setInterface(interface80211ptr);
                    e->setSourceType(IRoute::MANET);
                    e->setMetric(numHops);
                    e->setSequencenumber(msgsequencenumber);
                    e->setExpiryTime(simTime()+routeLifetime);
                    rt->addRoute(e);
                }
                if (!isForwardHello) {
                    recHello->setNextAddress(source);
                    numHops++;
                    recHello->setHopdistance(numHops);
                    double waitTime = intuniform(1, 50);
                    waitTime = waitTime/100;
                    EV_DETAIL << "waitime for forward before was " << waitTime <<" And host is " << source << "\n";
                    //waitTime= SIMTIME_DBL (simTime())+waitTime;
                    EV_DETAIL << "waitime for forward is " << waitTime <<" And host is " << source << "\n";     //FIXME unchanged waitTime showed twice!!!
                    packet->insertAtBack(recHello);
                    sendDelayed(packet, waitTime, "ipOut");
                    packet = nullptr;
                }
                else {
                    auto forwardHello = staticPtrCast<DsdvHello>(fhp->hello->peekData<DsdvHello>()->dupShared());
                    //forward useful message to other nodes
                    forwardHello->setNextAddress(source);
                    numHops++;
                    forwardHello->setHopdistance(numHops);
                    //send(HelloForward, "ipOut");//
                    //HelloForward=nullptr;//
                    double waitTime = intuniform(1, 50);
                    waitTime = waitTime/100;
                    EV_DETAIL << "waitime for forward before was " << waitTime <<" And host is " << source << "\n";
                    waitTime = SIMTIME_DBL(simTime())+waitTime;
                    EV_DETAIL << "waitime for forward is " << waitTime <<" And host is " << source << "\n";
                    fhp->event = new cMessage("event2");
                    scheduleAt(waitTime, fhp->event);
                    forwardList->push_back(fhp);
                    fhp = nullptr;
                }
            }
            delete packet;
            delete fhp;
            delete msg;
        }
        else
            throw cRuntimeError("Message arrived on unknown gate %s", msg->getArrivalGate()->getName());
    }
    else
        throw cRuntimeError("Message not supported %s", msg->getName());
}


} // namespace inet

