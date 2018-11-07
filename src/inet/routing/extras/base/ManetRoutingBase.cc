/***************************************************************************
 *   Copyright (C) 2008 by Alfonso Ariza                                   *
 *   Copyright (C) 2012 by Alfonso Ariza                                   *
 *   aarizaq@uma.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

///

#include "inet/routing/extras/base/ManetRoutingBase.h"

#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

using namespace ieee80211;

namespace inetmanet {

template<class KeyT, class ValueT, class CmpT>
class cStdMultiMapWatcher : public omnetpp::cStdVectorWatcherBase
{
  protected:
    std::multimap<KeyT,ValueT,CmpT>& m;
    mutable typename std::multimap<KeyT,ValueT,CmpT>::iterator it;
    mutable int itPos;
    std::string classname;
  public:
    cStdMultiMapWatcher(const char *name, std::multimap<KeyT,ValueT,CmpT>& var) : cStdVectorWatcherBase(name), m(var) {
        itPos=-1;
        classname = std::string("std::multimap<")+opp_typename(typeid(KeyT))+","+opp_typename(typeid(ValueT))+">";
    }
    const char *getClassName() const override {return classname.c_str();}
    virtual const char *getElemTypeName() const override {return "pair<,>";}
    virtual int size() const override {return m.size();}
    virtual std::string at(int i) const override {
        // std::map doesn't support random access iterator and iteration is slow,
        // so we have to use a trick, knowing that Tkenv will call this function with
        // i=0, i=1, etc...
        if (i==0) {
            it=m.begin(); itPos=0;
        } else if (i==itPos+1 && it!=m.end()) {
            ++it; ++itPos;
        } else {
            it=m.begin();
            for (int k=0; k<i && it!=m.end(); k++) ++it;
            itPos=i;
        }
        if (it==m.end()) {
            return std::string("out of bounds");
        }
        return atIt();
    }
    virtual std::string atIt() const {
        std::stringstream out;
        out << it->first << " ==> " << it->second;
        return out.str();
    }
};


template<class KeyT, class ValueT, class CmpT>
class cStdPointerMultiMapWatcher : public cStdMultiMapWatcher<KeyT,ValueT,CmpT>
{
  public:
    cStdPointerMultiMapWatcher(const char *name, std::multimap<KeyT,ValueT,CmpT>& var) : cStdMultiMapWatcher<KeyT,ValueT,CmpT>(name, var) {}
    virtual std::string atIt() const {
        std::stringstream out;
        out << this->it->first << "  ==>  " << *(this->it->second);
        return out.str();
    }
};

template<class KeyT, class ValueT, class CmpT>
void createStdPointerMultiMapWatcher(const char *varname, std::multimap<KeyT,ValueT,CmpT>& m)
{
    new cStdPointerMultiMapWatcher<KeyT,ValueT,CmpT>(varname, m);
}

#define WATCH_PTRMULTIMAP(m)            createStdPointerMultiMapWatcher(#m,(m))

#define IP_DEF_TTL 32
#define UDP_HDR_LEN 8

simsignal_t ManetRoutingBase::mobilityStateChangedSignal = registerSignal("mobilityStateChanged");

ManetRoutingBase::GlobalRouteMap *ManetRoutingBase::globalRouteMap = nullptr;
bool ManetRoutingBase::createInternalStore = false;

void ManetTimer::removeTimer()
{
    removeQueueTimer();
}

ManetTimer::ManetTimer(ManetRoutingBase* agent) : cOwnedObject("ManetTimer")
{
    agent_ = agent;
}

ManetTimer::~ManetTimer()
{
    removeTimer();
}

ManetTimer::ManetTimer() : cOwnedObject("ManetTimer")
{
    agent_ = dynamic_cast <ManetRoutingBase*> (this->getOwner());
    if (agent_==nullptr)
        throw cRuntimeError("timer ower is bad");
}

void ManetTimer::removeQueueTimer()
{
    TimerMultiMap::iterator it;
    for (it=agent_->getTimerMultimMap()->begin(); it != agent_->getTimerMultimMap()->end();  ) {
        if (it->second == this)
            agent_->getTimerMultimMap()->erase(it++);
        else
            ++it;
    }
}

void ManetTimer::resched(double time)
{
    removeQueueTimer();
    if (simTime() + time <= simTime())
        throw cRuntimeError("ManetTimer::resched message timer in the past");
    // Search first
    agent_->getTimerMultimMap()->insert(std::pair<simtime_t, ManetTimer *>(simTime()+time, this));
    agent_->scheduleEvent();
}

void ManetTimer::resched(simtime_t time)
{
    removeQueueTimer();
    if (time <= simTime())
        throw cRuntimeError("ManetTimer::resched message timer in the past");
    agent_->getTimerMultimMap()->insert(std::pair<simtime_t, ManetTimer *>(time, this));
}

bool ManetTimer::isScheduled()
{
    TimerMultiMap::iterator it;
    for (it=agent_->getTimerMultimMap()->begin() ; it != agent_->getTimerMultimMap()->end(); ++it ) {
        if (it->second==this) {
            return true;
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const ManetTimer& e)
{
    os << e.getClassName() << " ";
    return os;
};


L3Address ManetRoutingBase::getAddress() const {
    if (mac_layer_)
        return L3Address(interfaceVector->front().interfacePtr->getMacAddress());
    auto addr = interfaceVector->front().interfacePtr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
    return L3Address(addr);
}

L3Address ManetRoutingBase::getRouterId() const {
    if (inet_rt)
        return inet_rt->getRouterIdAsGeneric();
    return L3Address();
}

ManetRoutingBase::ManetRoutingBase()
{
    isRegistered = false;
    regPosition = false;
    mac_layer_ = false;
    timerMessagePtr = nullptr;
    commonPtr = nullptr;
    routesVector = nullptr;
    interfaceVector = new InterfaceVector;
    staticNode = false;
    collaborativeProtocol = nullptr;
    arp = nullptr;
    isGateway = false;
    proxyAddress.clear();
    addressGroupVector.clear();
    inAddressGroup.clear();
    addressSizeBytes = 4;
}


bool ManetRoutingBase::isThisInterfaceRegistered(InterfaceEntry * ie)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    for (auto & elem : *interfaceVector)
    {
        if (elem.interfacePtr==ie)
            return true;
    }
    return false;
}

void ManetRoutingBase::registerRoutingModule()
{
    InterfaceEntry *ie;
    const char *name;
    /* Set host parameters */
    isRegistered = true;
    int  num_80211 = 0;

    createTimerQueue();

    inet_rt = findModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
    inet_ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    hostModule = getContainingNode(this);
    networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);

    if (routesVector)
        routesVector->clear();

    if (par("useICMP"))
    {
        icmpModule = findModuleFromPar<Icmp>(par("icmpModule"), this);
    }
    sendToICMP = false;

    cProperties *props = getParentModule()->getProperties();
    bool hasMacRouting = props && props->getAsBool("macRouting");
    bool hasNic = props && props->getAsBool("nic");
    mac_layer_ = hasMacRouting || hasNic;
    useManetLabelRouting = par("useManetLabelRouting");

    const char *interfaces = par("interfaces");
    cStringTokenizer tokenizerInterfaces(interfaces);
    const char *token;
    const char * prefixName;

    if (!mac_layer_)
         setStaticNode(par("isStaticNode").boolValue());

    if (!mac_layer_)
    {
        while ((token = tokenizerInterfaces.nextToken()) != nullptr)
        {
            if ((prefixName = strstr(token, "prefix")) != nullptr)
            {
                const char *leftparenp = strchr(prefixName, '(');
                const char *rightparenp = strchr(prefixName, ')');
                std::string interfacePrefix;
                interfacePrefix.assign(leftparenp + 1, rightparenp - leftparenp - 1);
                for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
                {
                    ie = inet_ift->getInterface(i);
                    name = ie->getInterfaceName();
                    if ((strstr(name, interfacePrefix.c_str()) != nullptr) && !isThisInterfaceRegistered(ie))
                    {
                        InterfaceIdentification interface;
                        interface.interfacePtr = ie;
                        interface.index = i;
                        num_80211++;
                        interfaceVector->push_back(interface);
                    }
                }
            }
            else
            {
                for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
                {
                    ie = inet_ift->getInterface(i);
                    name = ie->getInterfaceName();
                    if (strcmp(name, token) == 0 && !isThisInterfaceRegistered(ie))
                    {
                        InterfaceIdentification interface;
                        interface.interfacePtr = ie;
                        interface.index = i;
                        num_80211++;
                        interfaceVector->push_back(interface);
                    }
                }
            }
        }
    }
    else
    {
        cModule *mod = nullptr;
        if (hasNic)
            mod = getParentModule();
        else if (hasMacRouting)
            mod = getParentModule()->getParentModule();
        else
            throw cRuntimeError("Manet routing protocol in mac layer but no nic");
        char *interfaceName = new char[strlen(mod->getFullName()) + 1];
        char *d = interfaceName;
        for (const char *s = mod->getFullName(); *s; s++)
            if (isalnum(*s))
                *d++ = *s;
        *d = '\0';

        for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
        {
            ie = inet_ift->getInterface(i);
            name = ie->getInterfaceName();
            if (strcmp(name, interfaceName) == 0 && !isThisInterfaceRegistered(ie))
            {
                InterfaceIdentification interface;
                interface.interfacePtr = ie;
                interface.index = i;
                num_80211++;
                interfaceVector->push_back(interface);
            }
        }
        delete [] interfaceName;
    }
    const char *exclInterfaces = par("excludedInterfaces");
    cStringTokenizer tokenizerExcluded(exclInterfaces);
    if (tokenizerExcluded.hasMoreTokens())
    {
        while ((token = tokenizerExcluded.nextToken())!=nullptr)
        {
            for (unsigned int i = 0; i<interfaceVector->size(); i++)
            {
                name = (*interfaceVector)[i].interfacePtr->getInterfaceName();
                if (strcmp(token, name)==0)
                {
                    interfaceVector->erase(interfaceVector->begin()+i);
                    break;
                }
            }
        }
    }


    if (interfaceVector->size()==0)
        throw cRuntimeError("Manet routing protocol has found no interfaces that can be used for routing.");

    // One enabled network interface (in total)
    // clear routing entries related to wlan interfaces and autoassign ip adresses
    bool manetPurgeRoutingTables = (bool) par("manetPurgeRoutingTables");
    if (manetPurgeRoutingTables && !mac_layer_)
    {
        IRoute *entry;
        // clean the route table wlan interface entry
        for (int i=inet_rt->getNumRoutes()-1; i>=0; i--)
        {
            entry = inet_rt->getRoute(i);
            const InterfaceEntry *ie = entry->getInterface();
            if (strstr(ie->getInterfaceName(), "wlan")!=nullptr)
            {
                inet_rt->deleteRoute(entry);
            }
        }
    }
    if (par("autoassignAddress") && !mac_layer_)
    {
        Ipv4Address AUTOASSIGN_ADDRESS_BASE(par("autoassignAddressBase").stringValue());
        if (AUTOASSIGN_ADDRESS_BASE.getInt() == 0)
            throw cRuntimeError("Auto assignment need autoassignAddressBase to be set");
        Ipv4Address myAddr(AUTOASSIGN_ADDRESS_BASE.getInt() + uint32(getParentModule()->getId()));
        for (int k=0; k<inet_ift->getNumInterfaces(); k++)
        {
            InterfaceEntry *ie = inet_ift->getInterface(k);
            if (strstr(ie->getInterfaceName(), "wlan")!=nullptr)
            {
                ie->getProtocolData<Ipv4InterfaceData>()->setIPAddress(myAddr);
                ie->getProtocolData<Ipv4InterfaceData>()->setNetmask(Ipv4Address::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }
    }
    // register LL-MANET-Routers
    if (!mac_layer_)
    {
        for (auto & elem : *interfaceVector)
        {
            elem.interfacePtr->getProtocolData<Ipv4InterfaceData>()->joinMulticastGroup(Ipv4Address::LL_MANET_ROUTERS);
        }
        arp = getModuleFromPar<IArp>(par("arpModule"), this);

        hostModule->subscribe(interfaceConfigChangedSignal, this);
        hostModule->subscribe(interfaceIpv4ConfigChangedSignal, this);
        hostModule->subscribe(interfaceIpv6ConfigChangedSignal, this);
    }

    isOperational = true;
    WATCH_PTRMULTIMAP(timerMultiMap);
 //   WATCH_MAP(*routesVector);
    if (!mac_layer_)
    {
        socket.setOutputGate(gate("socketOut"));
        // IPSocket socket(gate("ipOut"));
        socket.bind(par("UdpPort"));
        socket.setCallback(this);
        socket.setBroadcast(true);
    }
}

ManetRoutingBase::~ManetRoutingBase()
{
    // socket.close();
    delete interfaceVector;
    if (timerMessagePtr)
    {
        cancelAndDelete(timerMessagePtr);
        timerMessagePtr = nullptr;
    }

    while (timerMultiMapPtr->size()>0)
    {
        ManetTimer * timer = timerMultiMapPtr->begin()->second;
        timerMultiMapPtr->erase(timerMultiMapPtr->begin());
        delete timer;
    }

    if (routesVector)
    {
        delete routesVector;
        routesVector = nullptr;
    }
    proxyAddress.clear();
    addressGroupVector.clear();
    inAddressGroup.clear();

    if (globalRouteMap)
    {
        auto it = globalRouteMap->find(getAddress());
        if (it != globalRouteMap->end())
            globalRouteMap->erase(it);
        if (globalRouteMap->empty())
        {
            delete globalRouteMap;
            globalRouteMap = nullptr;
        }
    }
}

bool ManetRoutingBase::isIpLocalAddress(const Ipv4Address& dest) const
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    return inet_rt->isLocalAddress(dest);
}



bool ManetRoutingBase::isLocalAddress(const L3Address& dest) const
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    if (dest.getType() == L3Address::IPv4)
        return inet_rt->isLocalAddress(dest.toIpv4());
    InterfaceEntry *ie;
    for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
    {
        ie = inet_ift->getInterface(i);
        L3Address add(ie->getMacAddress());
        if (add==dest) return true;
    }
    return false;
}

bool ManetRoutingBase::isMulticastAddress(const L3Address& dest) const
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    return dest.isBroadcast();
}

void ManetRoutingBase::linkLayerFeeback()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    hostModule->subscribe(linkBrokenSignal, this);
}

void ManetRoutingBase::registerPosition()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    regPosition = true;
    cModule *mod = getContainingNode(this);
    mod->subscribe(mobilityStateChangedSignal, this);

    IMobility *imod = check_and_cast<IMobility *>(mod->getSubmodule("mobility"));

    curPosition = imod->getCurrentPosition();
    curSpeed = imod->getCurrentVelocity();
}

void ManetRoutingBase::processLinkBreak(const Packet *details) {return;}
void ManetRoutingBase::processLinkBreakManagement(const Packet *details) {return;}
void ManetRoutingBase::processLocatorAssoc(const Packet *details) {return;}
void ManetRoutingBase::processLocatorDisAssoc(const Packet *details) {return;}

void ManetRoutingBase::processChangeInterface(simsignal_t signalID,const cObject *details)
{
    IRoute *entry;
    // clean the route table wlan interface entry
    for (int i=inet_rt->getNumRoutes()-1; i>=0; i--)
    {
        entry = inet_rt->getRoute(i);
        const InterfaceEntry *ie = entry->getInterface();
        if (strstr(ie->getInterfaceName(), "wlan")!=nullptr)
        {
            inet_rt->deleteRoute(entry);
        }
    }
    handleNodeShutdown(nullptr);
    handleNodeStart(nullptr);
}



void ManetRoutingBase::sendToIpOnIface(Packet *msg, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, InterfaceEntry  *ie)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    if (!isOperational)
    {
        delete msg;
        return;
    }

    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    // if delay
    if (delay > 0) {
        DelayTimer * delayTimer =  new DelayTimer(msg, destAddr, destPort, this);
        if (timerMultiMapPtr == nullptr)
            throw cRuntimeError("timerMultiMapPtr == nullptr");
        delayTimer->resched(delay);
    }
    else {
        emit(packetSentSignal, msg);
        socket.sendTo(msg, destAddr, destPort);
    }
    // totalSend++;
}

void ManetRoutingBase::sendToIp(Packet *msg, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, const L3Address &iface)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    InterfaceEntry  *ie = nullptr;
    if (!iface.isUnspecified())
        ie = getInterfaceWlanByAddress(iface); // The user want to use a pre-defined interface
    else {
        // select the first interface.
        if (interfaceVector->empty())
            throw cRuntimeError("not valid wireless interface present");
        ie = interfaceVector->front().interfacePtr;
    }


    sendToIpOnIface(msg, srcPort, destAddr, destPort, ttl, delay, ie);
}

void ManetRoutingBase::sendToIp(Packet *msg, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, int index)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    InterfaceEntry  *ie = nullptr;
    if (index!=-1)
        ie = getInterfaceEntry(index); // The user want to use a pre-defined interface

    sendToIpOnIface(msg, srcPort, destAddr, destPort, ttl, delay, ie);
}


void ManetRoutingBase::omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm,
                                      short int hops, bool del_entry)
{
    omnet_chg_rte(dst.s_addr, gtwy.s_addr, netm.s_addr, hops, del_entry);
}

void ManetRoutingBase::omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm,
                                      short int hops, bool del_entry, const struct in_addr &iface)
{
    omnet_chg_rte(dst.s_addr, gtwy.s_addr, netm.s_addr, hops, del_entry, iface.s_addr);
}

bool ManetRoutingBase::omnet_exist_rte(struct in_addr dst)
{
    L3Address add = omnet_exist_rte(dst.s_addr);
    if (add.isUnspecified()) return false;
    else if (add.getType() == L3Address::IPv4 && add.toIpv4() == Ipv4Address::ALLONES_ADDRESS) return false;
    else if (add.getType() == L3Address::IPv6 && add.toIpv6() == Ipv6Address::ALL_NODES_1) return false;
    else return true;
}

void ManetRoutingBase::omnet_chg_rte(const L3Address &dst,
        const L3Address &gtwy, const L3Address &netm, short int hops,
        bool del_entry, const L3Address &iface) {
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    setRouteInternalStorege(dst, gtwy, del_entry);

    if (mac_layer_)
        return;
    bool found = false;
    IRoute *oldentry = nullptr;
    for (int i = inet_rt->getNumRoutes(); i > 0; --i) {
        IRoute *e = inet_rt->getRoute(i - 1);
        if (dst == e->getDestinationAsGeneric()) {
            if (del_entry && !found) {
                if (!inet_rt->deleteRoute(e))
                    throw cRuntimeError(
                            "Aodv omnet_chg_rte can't delete route entry");
            }
            else {
                found = true;
                oldentry = e;
            }
        }
    }

    if (del_entry)
        return;

    // The default mask is for manet routing is  Ipv4Address::ALLONES_ADDRESS
    int netMaskLeng = dst.getAddressType()->getMaxPrefixLength();
    if (!netm.isUnspecified()) {
        if (netm.getType() == L3Address::IPv4) {
            int i = 0;
            uint32_t low = netm.toIpv4().getInt();
            for (i = 0; i < 32; i++) {
                if (low & (1 << i))
                    break;
            }
            netMaskLeng = 32 - i;
        }
        else if (netm.getType() == L3Address::IPv6) {
            unsigned int i = 0;
            auto add = netm.toIpv6();
            uint32 *w = add.words();
            for (i = 0; i < 128; i++) {
                uint32_t val = w[(unsigned int) (i / 32)];
                if (val & (1 << i % 32))
                    break;
            }
            netMaskLeng = 128 - i;
        }
    }

    InterfaceEntry *ie = getInterfaceWlanByAddress(iface);
    IRoute::SourceType sourceType =
            useManetLabelRouting ? IRoute::MANET : IRoute::MANET2;

    if (found) {
        if (oldentry->getDestinationAsGeneric() == dst
                && oldentry->getPrefixLength() == netMaskLeng
                && oldentry->getNextHopAsGeneric() == gtwy
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSourceType() == sourceType)
            return;
        inet_rt->deleteRoute(oldentry);
    }

    IRoute *entry = inet_rt->createRoute();

    /// Destination
    entry->setDestination(dst);
    /// Route mask
    entry->setPrefixLength(netMaskLeng);
    /// Next hop
    entry->setNextHop(gtwy);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(ie);

    entry->setSource(this);

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise
    entry->setSourceType(sourceType);
    inet_rt->addRoute(entry);
}

// This methods use the nic index to identify the output nic.
void ManetRoutingBase::omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm,
                                      short int hops, bool del_entry, int index)
{
    omnet_chg_rte(dst.s_addr, gtwy.s_addr, netm.s_addr, hops, del_entry, index);
}


void ManetRoutingBase::omnet_chg_rte(const L3Address &dst,
        const L3Address &gtwy, const L3Address &netm, short int hops,
        bool del_entry, int index) {
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    setRouteInternalStorege(dst, gtwy, del_entry);
    if (mac_layer_)
        return;

    bool found = false;
    IRoute *oldentry = nullptr;
    for (int i = inet_rt->getNumRoutes(); i > 0; --i) {
        IRoute *e = inet_rt->getRoute(i - 1);
        if (dst == e->getDestinationAsGeneric()) {
            if (del_entry && !found) {
                if (!inet_rt->deleteRoute(e))
                    throw cRuntimeError(
                            "Aodv omnet_chg_rte can't delete route entry");
            } else {
                found = true;
                oldentry = e;
            }
        }
    }

    if (del_entry)
        return;

    // The default mask is for manet routing is  Ipv4Address::ALLONES_ADDRESS
    int netMaskLeng = dst.getAddressType()->getMaxPrefixLength();
    if (!netm.isUnspecified()) {
        if (netm.getType() == L3Address::IPv4) {
            int i = 0;
            uint32_t low = netm.toIpv4().getInt();
            for (i = 0; i < 32; i++) {
                if (low & (1 << i))
                    break;
            }
            netMaskLeng = 32 - i;
        }
        else if (netm.getType() == L3Address::IPv6) {
            unsigned int i = 0;
            auto add = netm.toIpv6();
            uint32 *w = add.words();
            for (i = 0; i < 128; i++) {
                uint32_t val = w[(unsigned int) (i / 32)];
                if (val & (1 << i % 32))
                    break;
            }
            netMaskLeng = 128 - i;
        }
    }

    InterfaceEntry *ie = getInterfaceEntry(index);
    IRoute::SourceType sourceType = useManetLabelRouting ? IRoute::MANET : IRoute::MANET2;

    if (found) {
        if (oldentry->getDestinationAsGeneric() == dst
                && oldentry->getPrefixLength() == netMaskLeng
                && oldentry->getNextHopAsGeneric() == gtwy
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSourceType() == sourceType)
            return;
        inet_rt->deleteRoute(oldentry);
    }

    IRoute *entry = inet_rt->createRoute();

    /// Destination
    entry->setDestination(dst);
    /// Route mask
    entry->setPrefixLength(netMaskLeng);
    /// Next hop
    entry->setNextHop(gtwy);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(ie);

    entry->setSource(this);

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise
    entry->setSourceType(sourceType);

    inet_rt->addRoute(entry);

}


//
// Check if it exists in the ip4 routing table the address dst
// if it doesn't exist return ALLONES_ADDRESS
//
L3Address ManetRoutingBase::omnet_exist_rte(L3Address dst)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    if (mac_layer_)
        return L3Address();

    const IRoute *e = nullptr;

    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        e = inet_rt->getRoute(i-1);
        if (dst == e->getDestinationAsGeneric())
            return e->getNextHopAsGeneric();
    }
    return L3Address(Ipv4Address::ALLONES_ADDRESS);
}

//
// Erase all the entries in the routing table
//
void ManetRoutingBase::omnet_clean_rte()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    IRoute *entry;
    if (mac_layer_)
        return;
    // clean the route table wlan interface entry
    for (int i=inet_rt->getNumRoutes()-1; i>=0; i--)
    {
        entry = inet_rt->getRoute(i);
        if (strstr(entry->getInterface()->getInterfaceName(), "wlan")!=nullptr)
        {
            inet_rt->deleteRoute(entry);
        }
    }
}

//
// generic receiveChangeNotification, the protocols must implement processLinkBreak and processPromiscuous only
//
void ManetRoutingBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("Manet llf");
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    if (signalID == interfaceConfigChangedSignal || signalID == interfaceIpv4ConfigChangedSignal || signalID == interfaceIpv6ConfigChangedSignal)
    {
        if (simTime() > 0)
            processChangeInterface(signalID,obj);

    }
    else if (signalID == linkBrokenSignal)
    {
        if (obj == nullptr)
            return;
        Packet *datagram = check_and_cast<Packet *>(obj);

        const auto dataHeader80211 = datagram->peekAtFront<Ieee80211DataOrMgmtHeader>();
        const auto dataOrMfmHeader80211 = datagram->peekAtFront<Ieee80211DataHeader>();

        if (dataHeader80211 != nullptr)
            processLinkBreak(datagram);
        else if (dataOrMfmHeader80211 != nullptr)
            processLinkBreakManagement(datagram);

    }
    else if(signalID == l2ApDisassociatedSignal || signalID == l2ApAssociatedSignal)
    {
    }
    else if (signalID == mobilityStateChangedSignal)
    {
        IMobility *mobility = check_and_cast<IMobility*>(obj);
        curPosition = mobility->getCurrentPosition();
        curSpeed = mobility->getCurrentVelocity();
        posTimer = simTime();
    }
}


/*
  Replacement for gettimeofday(), used for timers.
  The timeval should only be interpreted as number of seconds and
  fractions of seconds since the start of the simulation.
*/
int ManetRoutingBase::gettimeofday(struct timeval *tv, struct timezone *tz)
{
    double current_time;
    double tmp;
    /* Timeval is required, timezone is ignored */
    if (!tv)
        return -1;
    current_time = SIMTIME_DBL(simTime());
    tv->tv_sec = (long)current_time; /* Remove decimal part */
    tmp = (current_time - tv->tv_sec) * 1000000;
    tv->tv_usec = (long)(tmp+0.5);
    if (tv->tv_usec>1000000)
    {
        tv->tv_sec++;
        tv->tv_usec -= 1000000;
    }
    return 0;
}

//
// Get the index of interface with the same address that add
//
int ManetRoutingBase::getWlanInterfaceIndexByAddress(L3Address add)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    if (add.isUnspecified())
        return interfaceVector->front().index;

    for (auto & elem : *interfaceVector)
    {
        if (add.getType() == L3Address::MAC)
        {
            if (elem.interfacePtr->getMacAddress() == add.toMac())
                return elem.index;
        }
        else
        {
            if (elem.interfacePtr->getProtocolData<Ipv4InterfaceData>()->getIPAddress() == add.toIpv4())
                return elem.index;
        }
    }
    return -1;
}

//
// Get the interface with the same address that add
//
InterfaceEntry *ManetRoutingBase::getInterfaceWlanByAddress(L3Address add) const
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    if (add.isUnspecified())
        return interfaceVector->front().interfacePtr;

    for (auto & elem : *interfaceVector)
    {
        if (add.getType() == L3Address::MAC)
        {
            if (elem.interfacePtr->getMacAddress() == add.toMac())
                return elem.interfacePtr;
        }
        else
        {
            if (elem.interfacePtr->getProtocolData<Ipv4InterfaceData>()->getIPAddress() == add.toIpv4())
                return elem.interfacePtr;
        }
    }
    return nullptr;
}

//
// Get the index used in the general interface table
//
int ManetRoutingBase::getWlanInterfaceIndex(int i) const
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    if (i >= 0 && i < (int) interfaceVector->size())
        return (*interfaceVector)[i].index;
    else
        return -1;
}

//
// Get the i-esime wlan interface
//
InterfaceEntry *ManetRoutingBase::getWlanInterfaceEntry(int i) const
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    if (i >= 0 && i < (int)interfaceVector->size())
        return (*interfaceVector)[i].interfacePtr;
    else
        return nullptr;
}


//////////////////
void ManetRoutingBase::createTimerQueue()
{
    if (timerMessagePtr == nullptr)
        timerMessagePtr = new cMessage("ManetRoutingBase TimerQueue");
    if (timerMultiMapPtr == nullptr)
        timerMultiMapPtr = new TimerMultiMap;
}


void ManetRoutingBase::scheduleEvent()
{
    if (!timerMessagePtr)
        return;
    if (!timerMultiMapPtr)
        return;

    if (timerMultiMapPtr->empty()) { // nothing to do
        if (timerMessagePtr->isScheduled())
            cancelEvent(timerMessagePtr);
        return;
    }

    simtime_t now = simTime();
    while (!timerMultiMapPtr->empty() && timerMultiMapPtr->begin()->first <= now) {
        auto e = timerMultiMapPtr->begin();
        ManetTimer * timer = e->second;
        timerMultiMapPtr->erase(e);
        timer->expire();
    }

    auto e = timerMultiMapPtr->begin();
    if (timerMessagePtr->isScheduled()) {
        if (e->first < timerMessagePtr->getArrivalTime())  {
            cancelEvent(timerMessagePtr);
            scheduleAt(e->first, timerMessagePtr);
        }
        else if (e->first>timerMessagePtr->getArrivalTime()) { // Possible throw cRuntimeError, or the first event has been canceled
            cancelEvent(timerMessagePtr);
            scheduleAt(e->first, timerMessagePtr);
            EV << "timer Queue problem";
            // throw cRuntimeError("timer Queue problem");
        }
    }
    else {
        scheduleAt(e->first, timerMessagePtr);
    }
}

bool ManetRoutingBase::checkTimer(cMessage *msg)
{
    if (msg != timerMessagePtr)
        return false;
    if (timerMessagePtr == nullptr)
        throw cRuntimeError("ManetRoutingBase::checkTimer throw cRuntimeError timerMessagePtr doens't exist");
    if (timerMultiMapPtr->empty())
        return true;

    while (timerMultiMapPtr->begin()->first <= simTime())
    {
        auto it = timerMultiMapPtr->begin();
        ManetTimer * timer = it->second;
        if (timer == nullptr)
            throw cRuntimeError ("timer owner is bad");
        timerMultiMapPtr->erase(it);
        timer->expire();
        if (timerMultiMapPtr->empty())
            break;
    }
    return true;
}

//
// Access to node position
//
///////////////////
const Coord& ManetRoutingBase::getPosition()
{
    if (!regPosition)
        throw cRuntimeError("this node doesn't have activated the register position");
    return curPosition;
}

double ManetRoutingBase::getSpeed()
{
    if (!regPosition)
        throw cRuntimeError("this node doesn't have activated the register position");
    return curSpeed.length();
}

const Coord& ManetRoutingBase::getDirection()
{
    if (!regPosition)
        throw cRuntimeError("this node doesn't have activated the register position");
    return curSpeed;
}

void ManetRoutingBase::setInternalStore(bool i)
{
    createInternalStore = i;
    if (!createInternalStore)
    {
        delete routesVector;
        routesVector = nullptr;
    }
    else
    {
        if (routesVector==nullptr)
            routesVector = new RouteMap;
    }
}


L3Address ManetRoutingBase::getNextHopInternal(const L3Address &dest)
{
    if (routesVector==nullptr)
        return L3Address();
    if (routesVector->empty())
        return L3Address();
    auto it = routesVector->find(dest);
    if (it!=routesVector->end())
        return it->second;
    return L3Address();
}

bool ManetRoutingBase::setRoute(const L3Address & destination, const L3Address &nextHop, const int &ifaceIndex, const int &hops, const L3Address &mask)
{

    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");




    /* Add route to kernel routing table ... */
    bool del_entry = (nextHop.isUnspecified());

    setRouteInternalStorege(destination, nextHop, del_entry);

    if (mac_layer_)
        return true;

    if (ifaceIndex>=getNumInterfaces())
        return false;

    bool found = false;
    IRoute *oldentry = nullptr;

    //TODO the entries with ALLONES netmasks stored at the begin of inet route entry vector,
    // let optimise next search!
    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        IRoute *e = inet_rt->getRoute(i-1);
        if (destination == e->getDestinationAsGeneric())     // FIXME netmask checking?
        {
            if (del_entry && !found)    // FIXME The 'found' never set to true when 'del_entry' is true
            {
                if (!inet_rt->deleteRoute(e))
                    throw cRuntimeError("ManetRoutingBase::setRoute can't delete route entry");
            }
            else
            {
                found = true;
                oldentry = e;
            }
        }
    }
    if (del_entry)
        return true;


    // The default mask is for manet routing is  Ipv4Address::ALLONES_ADDRESS
    int netMaskLeng = destination.getAddressType()->getMaxPrefixLength();
    if (!mask.isUnspecified()) {
        if (mask.getType() == L3Address::IPv4) {
            int i = 0;
            uint32_t low = mask.toIpv4().getInt();
            for (i = 0; i < 32; i++) {
                if (low & (1 << i))
                    break;
            }
            netMaskLeng = 32 - i;
        }
        else if (mask.getType() == L3Address::IPv6) {
            unsigned int i = 0;
            auto add = mask.toIpv6();
            uint32 *w = add.words();
            for (i = 0; i < 128; i++) {
                uint32_t val = w[(unsigned int) (i / 32)];
                if (val & (1 << i % 32))
                    break;
            }
            netMaskLeng = 128 - i;
        }
    }

    InterfaceEntry *ie = getInterfaceEntry(ifaceIndex);

    if (found)
    {
        if (oldentry->getDestinationAsGeneric() == destination
                && oldentry->getPrefixLength() == netMaskLeng
                && oldentry->getNextHopAsGeneric() == nextHop
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSourceType() == IRoute::MANUAL)
            return true;
        inet_rt->deleteRoute(oldentry);
    }

    IRoute *entry = inet_rt->createRoute();

    entry->setDestination(destination);
    /// Route mask
    entry->setPrefixLength(netMaskLeng);
    /// Next hop
    entry->setNextHop(nextHop);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(ie);

    entry->setSource(this);

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise
    IRoute::SourceType sourceType = useManetLabelRouting ? IRoute::MANET : IRoute::MANET2;
    entry->setSourceType(sourceType);

    inet_rt->addRoute(entry);

    return true;

}

bool ManetRoutingBase::setRoute(const L3Address & destination, const L3Address &nextHop, const char *ifaceName, const int &hops, const L3Address &mask)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    int index;
    for (index = 0; index < getNumInterfaces(); index++)
    {
        if (strcmp(ifaceName, getInterfaceEntry(index)->getInterfaceName())==0) break;
    }
    if (index>=getNumInterfaces())
        return false;
    return setRoute(destination, nextHop, index, hops, mask);
};

bool ManetRoutingBase::sendICMP(Packet *pkt)
{
    if (pkt == nullptr)
        return false;

    if (!sendToICMP)
        return false;

    if (icmpModule == nullptr) {
        return false;
    }

    auto pktAux = pkt->dup();

    if (mac_layer_) {
        pktAux->popAtFront<Ieee80211MacHeader>();
        // The packet is encapsulated in a Ieee802 frame
    }

    // TODO: Reinjec the packet, the IP layer should send the notification
    const auto& networkHeader = pktAux->peekAtFront<Ipv4Header>();

    if (networkHeader == nullptr) {
        delete pktAux;
        return false;
    }
    // don't send ICMP error messages for multicast messages
    if (networkHeader->getDestAddress().isMulticast()) {
        delete pktAux;
        return false;
    }
    // check source address
    if (networkHeader->getSourceAddress().isUnspecified() && par("setICMPSourceAddress")) {
        const auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(pktAux);
        ipv4Header->setSourceAddress(inet_ift->getInterface(0)->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        insertNetworkProtocolHeader(pktAux, Protocol::ipv4, ipv4Header);
    }

    EV_DETAIL << "issuing ICMP Destination Unreachable for packets waiting in queue for failed route discovery.\n";
    int interfaceId = pkt->getTag<InterfaceInd>()->getInterfaceId();
    icmpModule->sendErrorMessage(pktAux, interfaceId , ICMP_DESTINATION_UNREACHABLE, 0);

    return true;
}
// The address group allows to implement the anycast. Any address in the group is valid for the route
// Address group methods
//


int  ManetRoutingBase::getNumAddressInAGroups(int group)
{
    if ((int)addressGroupVector.size()<=group)
        return -1;
    return addressGroupVector[group].size();
}

void ManetRoutingBase::addInAddressGroup(const L3Address& addr, int group)
{
    AddressGroup addressGroup;
    if ((int)addressGroupVector.size()<=group)
    {
        while ((int)addressGroupVector.size()<=group)
        {
            AddressGroup addressGroup;
            addressGroupVector.push_back(addressGroup);
        }
    }
    else
    {
        if (addressGroupVector[group].count(addr)>0)
            return;
    }
    addressGroupVector[group].insert(addr);
    // check if the node is already in the group
    if (isLocalAddress(addr))
    {
        for (auto & elem : inAddressGroup)
        {
            if (elem==group)
                return;
        }
        inAddressGroup.push_back(group);
    }
}

bool ManetRoutingBase::delInAddressGroup(const L3Address& addr, int group)
{
    if ((int)addressGroupVector.size()<=group)
        return false;
    if (addressGroupVector[group].count(addr)==0)
        return false;

    addressGroupVector[group].erase(addr);
    // check if the node is already in the group
    if (isLocalAddress(addr))
    {
        // check if other address is in the group
        for (const auto & elem : addressGroupVector[group])
            if (isLocalAddress(elem)) return true;
        for (unsigned int i=0; i<inAddressGroup.size(); i++)
        {
            if (inAddressGroup[i]==group)
            {
                inAddressGroup.erase(inAddressGroup.begin()+i);
                return true;
            }
        }
    }
    return true;
}

bool ManetRoutingBase::findInAddressGroup(const L3Address& addr, int group)
{
    if ((int)addressGroupVector.size()<=group)
        return false;
    if (addressGroupVector[group].count(addr)>0)
        return true;
    return false;
}

bool ManetRoutingBase::findAddressAndGroup(const L3Address& addr, int &group)
{
    if (addressGroupVector.empty())
        return false;
    for (unsigned int i=0; i<addressGroupVector.size(); i++)
    {
        if (findInAddressGroup(addr, i))
        {
            group = i;
            return true;
        }
    }
    return false;
}

bool ManetRoutingBase::isInAddressGroup(int group)
{
    if (inAddressGroup.empty())
        return false;
    for (auto & elem : inAddressGroup)
        if (group==elem)
            return true;
    return false;
}

bool ManetRoutingBase::getAddressGroup(AddressGroup &addressGroup, int group)
{
    if ((int)addressGroupVector.size()<=group)
        return false;
    addressGroup = addressGroupVector[group];
    return true;
}

bool ManetRoutingBase::getAddressGroup(std::vector<L3Address> &addressGroup, int group)
{
    if ((int)addressGroupVector.size()<=group)
        return false;
    addressGroup.clear();
    for (const auto & elem : addressGroupVector[group])
        addressGroup.push_back(elem);
    return true;
}


bool ManetRoutingBase::isAddressInProxyList(const L3Address & addr)
{
    if (proxyAddress.empty())
        return false;
    for (auto & elem : proxyAddress)
    {
        //if ((addr & proxyAddress[i].mask) == proxyAddress[i].address)
        if (addr == elem.address)   //FIXME
            return true;
    }
    return false;
}

void ManetRoutingBase::setAddressInProxyList(const L3Address & addr,const L3Address & mask)
{
    // search if exist
    for (auto & elem : proxyAddress)
    {
        if ((addr == elem.address) && (mask == elem.mask))
            return;
    }
    ManetProxyAddress val;
    val.address = addr;
    val.mask = mask;
    proxyAddress.push_back(val);
}

bool ManetRoutingBase::getAddressInProxyList(int i,L3Address &addr, L3Address &mask)
{
    if (i< 0 || i >= (int)proxyAddress.size())
        return false;
    addr = proxyAddress[i].address;
    mask = proxyAddress[i].mask;
    return true;
}


bool ManetRoutingBase::addressIsForUs(const L3Address &addr) const
{
    if (isLocalAddress(addr))
        return true;
    if (proxyAddress.empty())
        return false;
    for (auto & elem : proxyAddress)
    {
        //if ((addr & proxyAddress[i].mask) == proxyAddress[i].address)
        if (addr == elem.address)   //FIXME
            return true;
    }
    return false;
}

bool ManetRoutingBase::getAp(const L3Address &destination, L3Address& accesPointAddr) const
{
    return false;
}

void ManetRoutingBase::getApList(const MacAddress & dest,std::vector<MacAddress>& list)
{
    list.clear();
    list.push_back(dest);
}

void ManetRoutingBase::getApListIp(const Ipv4Address &dest,std::vector<Ipv4Address>& list)
{
    list.clear();
    list.push_back(dest);
}

void ManetRoutingBase::getListRelatedAp(const L3Address & add, std::vector<L3Address>& list)
{
    if (add.getType() == L3Address::MAC)
    {
        std::vector<MacAddress> listAux;
        getApList(add.toMac(), listAux);
        list.clear();
        for (auto & elem : listAux)
        {
            list.push_back(L3Address(elem));
        }
    }
    else
    {
        std::vector<Ipv4Address> listAux;
        getApListIp(add.toIpv4(), listAux);
        list.clear();
        for (auto & elem : listAux)
        {
            list.push_back(L3Address(elem));
        }
    }
}

bool ManetRoutingBase::isAp() const
{
    return false;
}


void ManetRoutingBase::setRouteInternalStorege(const L3Address &dest, const L3Address &next, const bool &erase)
{
    if (!createInternalStore && routesVector)
     {
         delete routesVector;
         routesVector = nullptr;
     }
     else if (createInternalStore && routesVector)
     {
         auto it = routesVector->find(dest);
         if (it != routesVector->end())
         {
             if (erase)
                 routesVector->erase(it);
             else
                 it->second = next;
         }
         else
             routesVector->insert(std::pair<L3Address,L3Address>(dest, next));
     }
}

bool ManetRoutingBase::getRouteFromGlobal(const L3Address &src, const L3Address &dest, std::vector<L3Address> &route)
{
    if (!createInternalStore || globalRouteMap == nullptr)
        return false;
    L3Address next = src;
    route.clear();
    route.push_back(src);
    while (1)
    {
        auto it = globalRouteMap->find(next);
        if (it==globalRouteMap->end())
            return false;
        if (it->second.empty())
            return false;

        if (it->second.size() == 1)
        {
            RouteMap *rt = it->second[0].routesVector;
            auto it2 = rt->find(dest);
            if (it2 == rt->end())
                return false;
            if (it2->second == dest)
            {
                route.push_back(dest);
                return true;
            }
            else
            {
                route.push_back(it2->second);
                next = it2->second;
            }
        }
        else
        {
            if (it->second.size() > 2)
                throw cRuntimeError("Number of routing protocols bigger that 2");
            // if several protocols, search before in the proactive
            RouteMap *rt;
            if (it->second[0].isProactive)
                rt = it->second[0].routesVector;
            else
                rt = it->second[1].routesVector;
            auto it2 = rt->find(dest);
            if (it2 == rt->end())
            {
                // search in the reactive

                if (it->second[0].isProactive)
                    rt = it->second[1].routesVector;
                else
                    rt = it->second[0].routesVector;
                it2 = rt->find(dest);
                if (it2 == rt->end())
                    return false;
            }
            if (it2->second == dest)
            {
                route.push_back(dest);
                return true;
            }
            else
            {
                route.push_back(it2->second);
                next = it2->second;
            }
        }
    }
}


// Auxiliary function that return a string with the address
std::string ManetRoutingBase::convertAddressToString(const L3Address& add)
{
    return add.str();
}


} // namespace inetmanet

} // namespace inet
