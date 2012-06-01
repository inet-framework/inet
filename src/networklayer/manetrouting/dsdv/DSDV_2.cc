/*
 * Copyright (C) 2008
 * DSDV simple example for INET (add-on)
 * Version 2.0
 * Diogo Ant�o & Pedro Menezes
 * Instituto Superior T�cnico
 * Lisboa - Portugal
 * This version and newer version can be found at http://dsdv.8rf.com
 * This code was written while assisting the course "Redes m�veis e sem fios" http://comp.ist.utl.pt/ec-cm
 * Autorization to use and modify this code not needed :P
 * The authors hope it will be useful to help understand how
 * INET and OMNET++ works(more specifically INET 20061020 and omnet++ 3.3).
 * Also we hope it will help in the developing of routing protocols using INET.
*/

#include "DSDVhello_m.h"//created by opp_msgc 3.3 from DSDVhello.msg
#include "DSDV_2.h"

#define NOforwardHello
Define_Module(DSDV_2);

void DSDV_2::initialize(int stage)
{
    //reads from omnetpp.ini
    if (stage==4)
    {
        sequencenumber = 0;
        ift = NULL;
        rt = NULL;
        ift = InterfaceTableAccess().get();
        /* Search the 80211 interface */
        int  num_80211 = 0;
        InterfaceEntry *   ie;
        InterfaceEntry *   i_face;
        const char *name;
        for (int i = 0; i < ift->getNumInterfaces(); i++)
        {
            ie = ift->getInterface(i);
            name = ie->getName();
            if (strstr(name, "wlan")!=NULL)
            {
                i_face = ie;
                num_80211++;
                interfaceId = i;
            }
        }

        // One enabled network interface (in total)
        if (num_80211==1)
            interface80211ptr = i_face;
        else
            opp_error("DSDV has found %i 80211 interfaces", num_80211);
        rt = RoutingTableAccess().get();
        if (par("manetPurgeRoutingTables").boolValue())
        {
            IPv4Route *entry;
            // clean the route table wlan interface entry
            for (int i=rt->getNumRoutes()-1; i>=0; i--)
            {
                entry = rt->getRoute(i);
                const InterfaceEntry *ie = entry->getInterface();
                if (strstr(ie->getName(), "wlan")!=NULL)
                {
                    rt->deleteRoute(entry);
                }
            }
        }
        interface80211ptr->ipv4Data()->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);

        // schedules a random periodic event: the hello message broadcast from DSDV module

        routeLifetime = par("routeLifetime").doubleValue();

        //reads from omnetpp.ini
        hellomsgperiod_DSDV = (simtime_t) par("hellomsgperiod_DSDV");
        //HelloForward = new DSDV_HelloMessage("HelloForward");
        // schedules a random periodic event: the hello message broadcast from DSDV module
        forwardList = new list<forwardHello*>;
        event = new cMessage("event");
        scheduleAt( uniform(0, par("MaxVariance_DSDV").doubleValue(), par("RNGseed_DSDV").doubleValue()), event);

    }
}


DSDV_2::forwardHello::~forwardHello()
{
    if (this->event!=NULL) delete this->event;
    if (this->hello!=NULL) delete this->hello;
}

DSDV_2::forwardHello::forwardHello()
{
    this->event = NULL;
    this->hello = NULL;
}


DSDV_2::DSDV_2()
{
    // Set the pointer to NULL, so that the destructor won't crash
    // even if initialize() doesn't get called because of a runtime
    // error or user cancellation during the startup process.
    event = NULL;
}

DSDV_2::~DSDV_2()
{
    // Dispose of dynamically allocated the objects
    cancelAndDelete(event);
    while (!forwardList->empty())
    {
        forwardHello *fh = forwardList->front();
        if (fh->event)
            cancelAndDelete(fh->event);
        if (fh->hello)
            cancelAndDelete(fh->hello);
        fh->event = NULL;
        fh->hello = NULL;
        forwardList->pop_front();
        delete fh;
    }
    delete forwardList;
    //delete Hello;
}

void DSDV_2::handleMessage(cMessage *msg)
{

    DSDV_HelloMessage * recHello = NULL;
    // When DSDV module receives selfmessage (scheduled event)
    // it means that it's time for Hello message broadcast event
    // i.e. Brodcast Hello messages to other nodes when selfmessage=event
    // But if selmessage!=event it means that it is time to forward useful Hello message to othert nodes
    if (msg->isSelfMessage())
    {
        if (msg==event)
        {
            //new hello message

            DSDV_HelloMessage * Hello = new DSDV_HelloMessage("Hello");

            //pointer to interface and routing table
            if (!ift)
                ift = InterfaceTableAccess().get();
            if (!rt)
                rt = RoutingTableAccess().get();

            rt->purge();

            // count non-loopback interfaces
            // int numIntf = 0;
            // InterfaceEntry *ie = NULL;
            //for (int k=0; k<ift->getNumInterfaces(); k++)
            //  if (!ift->getInterface(k)->isLoopback())
            //  {ie = ift->getInterface(k); numIntf++;}

            // Filling the DSDV_HelloMessage fields
            // IPv4Address source = (ie->ipv4()->getIPAddress());
            IPv4Address source = (interface80211ptr->ipv4Data()->getIPAddress());
            Hello->setBitLength(128); ///size of Hello message in bits
            Hello->setSrcIPAddress(source);
            sequencenumber += 2;
            Hello->setSequencenumber(sequencenumber);
            Hello->setNextIPAddress(source);
            Hello->setHopdistance(1);

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
            //new control info for DSDV_HelloMessage
            IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
            controlInfo->setDestAddr(IPv4Address(255, 255, 255, 255)); //let's try the limited broadcast 255.255.255.255 but multicast goes from 224.0.0.0 to 239.255.255.255
            controlInfo->setSrcAddr(source); //let's try the limited broadcast
            controlInfo->setProtocol(IP_PROT_MANET);
            controlInfo->setInterfaceId(interface80211ptr->getInterfaceId());
            Hello->setControlInfo(controlInfo);

            //broadcast to other nodes the hello message
            send(Hello, "to_ip");
            Hello = NULL;

            //schedule new brodcast hello message event
            scheduleAt(simTime()+hellomsgperiod_DSDV+uniform(0, 0.01), event);
            bubble("Sending new hello message");
        }
        else
        {
            for (list<forwardHello*>::iterator it = forwardList->begin(); it != forwardList->end(); it++)
            {
                if ( (*it)->event == msg )
                {
                    try
                    {
                        EV << "Vou mandar forward do " << (*it)->hello->getSrcIPAddress() << endl;
                        if ( (*it)->hello->getControlInfo() == NULL )
                            error("Apanhei-o a nulo no for");
                        send((*it)->hello, "to_ip");
                        (*it)->hello = NULL;
                        delete (*it)->event;
                        (*it)->event = NULL;
                        delete (*it);
                        forwardList->erase(it);
                    }
                    catch (exception &e)
                    {
                        error(e.what());
                    }
                    break;
                }
            }
        }
    }
    else if (dynamic_cast<DSDV_HelloMessage *>(msg))
    {

        // When DSDV module receives DSDV_HelloMessage from other host
        // it adds/replaces the information in routing table for the one contained in the message
        // but only if it's useful/up-to-date. If not the DSDV module ignores the message.
        try
        {
            if (msg->getControlInfo() != NULL )
                delete msg->removeControlInfo();
            IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
            controlInfo->setDestAddr(IPv4Address(255, 255, 255, 255)); //let's try the limited broadcast 255.255.255.255 but multicast goes from 224.0.0.0 to 239.255.255.255

            // int numIntf = 0;
            if (ift!=NULL)
                ift = InterfaceTableAccess().get();
            //InterfaceEntry *ie = NULL;
            //for (int k=0; k<ift->getNumInterfaces(); k++)
            //  if (!ift->getInterface(k)->isLoopback())
            //  {ie = ift->getInterface(k); numIntf++;}

            //controlInfo->setSrcAddr(ie->ipv4()->getIPAddress());
            controlInfo->setSrcAddr(interface80211ptr->ipv4Data()->getIPAddress());
            controlInfo->setProtocol(IP_PROT_MANET);
            controlInfo->setInterfaceId(interface80211ptr->getInterfaceId());
#ifdef NOforwardHello
            recHello = dynamic_cast<DSDV_HelloMessage *>(msg);
            recHello->setControlInfo(controlInfo);
#else
            forwardHello *fhp;
            DSDV_HelloMessage *helloFor;
            fhp = new forwardHello();
            fhp->hello = (DSDV_HelloMessage *) (dynamic_cast<DSDV_HelloMessage *>(msg))->dup();
            fhp->hello = dynamic_cast<DSDV_HelloMessage *>(msg);
            fhp->hello->setControlInfo(controlInfo);
            if ( fhp->hello->getControlInfo() == NULL )
                error("Nulo quando copiei");
#endif
        }
        catch (exception &e)
        {
            error(e.what());
        }
#ifdef  NOforwardHello
        if (msg->arrivedOn("from_ip") && recHello)
        {
#else
        if (msg->arrivedOn("from_ip") && fhp->hello)
        {
#endif


            bubble("Received hello message");

            IPv4Address source = interface80211ptr->ipv4Data()->getIPAddress();

            //pointer to interface and routing table
            //rt = RoutingTableAccess_DSDV().get(); // RoutingTable *rt = nodeInfo[i].rt;
            //ift = InterfaceTableAccess().get();//InterfaceTable *ift = nodeInfo[i].ift;


            //reads DSDV hello message fields
#ifdef      NOforwardHello

            IPv4Address src = recHello->getSrcIPAddress();
            unsigned int msgsequencenumber = recHello->getSequencenumber();
            IPv4Address next = recHello->getNextIPAddress();
            int numHops = recHello->getHopdistance();

            if (src==source)
            {
                EV << "Hello msg dropped. This message returned to the original creator.\n";
                try
                {
                    delete recHello;
                }
                catch (exception &e)
                {
                    error(e.what());
                }
                return;
            }
#else
            IPv4Address src = fhp->hello->getSrcIPAddress();
            unsigned int msgsequencenumber = fhp->hello->getSequencenumber();
            IPv4Address next = fhp->hello->getNextIPAddress();
            int numHops = fhp->hello->getHopdistance();

            if (src==source)
            {
                EV << "Hello msg dropped. This message returned to the original creator.\n";
                try
                {
                    delete fhp;
                }
                catch (exception &e)
                {
                    error(e.what());
                }
                return;
            }
#endif
            // count non-loopback interfaces
            //int numIntf = 0;
            //InterfaceEntry *ie = NULL;
            //for (int k=0; k<ift->getNumInterfaces(); k++)
            //  if (!ift->getInterface(k)->isLoopback())
            //  {ie = ift->getInterface(k); numIntf++;}
            //
            //Tests if the DSDV hello message that arrived is originally from another node
            //IPv4Address source = (ie->ipv4()->getIPAddress());



            IPv4Route *_entrada_routing = rt->findBestMatchingRoute(src);
            DSDVIPv4Route *entrada_routing = dynamic_cast<DSDVIPv4Route *>(_entrada_routing);

            //Tests if the DSDV hello message that arrived is useful
            if (_entrada_routing == NULL
                    || (_entrada_routing != NULL && _entrada_routing->getNetmask() != IPv4Address::ALLONES_ADDRESS)
                    || (entrada_routing != NULL && (msgsequencenumber>(entrada_routing->getSequencenumber()) || (msgsequencenumber == (entrada_routing->getSequencenumber()) && numHops < (entrada_routing->getMetric())))))
            {

                //remove old entry
                if (entrada_routing != NULL)
                    rt->deleteRoute(entrada_routing);

                //adds new information to routing table according to information in hello message
                {
                    IPv4Address netmask = IPv4Address::ALLONES_ADDRESS; // IPv4Address(par("netmask").stringValue());
                    DSDVIPv4Route *e = new DSDVIPv4Route();
                    e->setDestination(src);
                    e->setNetmask(netmask);
                    e->setGateway(next);
                    e->setInterface(interface80211ptr);
                    e->setSource(IPv4Route::MANET);
                    e->setMetric(numHops);
                    e->setSequencenumber(msgsequencenumber);
                    e->setExpiryTime(simTime()+routeLifetime);
                    rt->addRoute(e);
                }
#ifdef      NOforwardHello
                recHello->setNextIPAddress(source);
                numHops++;
                recHello->setHopdistance(numHops);
                //send(HelloForward, "to_ip");//
                //HelloForward=NULL;//
                double waitTime = intuniform(1, 50);
                waitTime = waitTime/100;
                EV << "waitime for forward before was " << waitTime <<" And host is " << source << "\n";
                //waitTime= SIMTIME_DBL (simTime())+waitTime;
                EV << "waitime for forward is " << waitTime <<" And host is " << source << "\n";
                sendDelayed(recHello, waitTime, "to_ip");
#else
                try
                {
                    //forward useful message to other nodes
                    fhp->hello->setNextIPAddress(source);
                    numHops++;
                    fhp->hello->setHopdistance(numHops);
                    //send(HelloForward, "to_ip");//
                    //HelloForward=NULL;//
                    double waitTime = intuniform(1, 50);
                    waitTime = waitTime/100;
                    EV << "waitime for forward before was " << waitTime <<" And host is " << source << "\n";
                    waitTime = SIMTIME_DBL(simTime())+waitTime;
                    EV << "waitime for forward is " << waitTime <<" And host is " << source << "\n";
                    fhp->event = new cMessage("event2");
                    scheduleAt(waitTime, fhp->event);
                    forwardList->push_back(fhp);
                }
                catch (exception &e)
                {
                    error(e.what());
                }
#endif
            }
            else
            {
#ifdef      NOforwardHello
                delete msg;
#else
                delete fhp;
#endif
            }
            //delete msg; ?
        }
        else
        {
            delete msg;
        }
    }
    else
    {
        error("Message not supported %s", msg->getName());
    }
}



