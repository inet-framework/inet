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
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/ipv6/IPv6AddressType.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/arp/IARP.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/routing/extras/base/ControlInfoBreakLink_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAP.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

using namespace ieee80211;

namespace inetmanet {

#define IP_DEF_TTL 32
#define UDP_HDR_LEN 8

simsignal_t ManetRoutingBase::mobilityStateChangedSignal = registerSignal("mobilityStateChanged");

ManetRoutingBase::GlobalRouteMap *ManetRoutingBase::globalRouteMap = NULL;
bool ManetRoutingBase::createInternalStore = false;

ManetRoutingBase::ManetRoutingBase()
{
    isRegistered = false;
    regPosition = false;
    mac_layer_ = false;
    commonPtr = NULL;
    routesVector = NULL;
    interfaceVector = new InterfaceVector;
    staticNode = false;
    collaborativeProtocol = NULL;
    arp = NULL;
    isGateway = false;
    proxyAddress.clear();
    addressGroupVector.clear();
    inAddressGroup.clear();

}


bool ManetRoutingBase::isThisInterfaceRegistered(InterfaceEntry * ie)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    for (unsigned int i=0; i<interfaceVector->size(); i++)
    {
        if ((*interfaceVector)[i].interfacePtr==ie)
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
    inet_rt = findModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
    inet_ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    hostModule = getContainingNode(this);

    if (routesVector)
        routesVector->clear();

    if (par("useICMP"))
    {
        icmpModule = findModuleFromPar<ICMP>(par("icmpModule"), this);
    }
    sendToICMP = false;

    cProperties *props = getParentModule()->getProperties();
    mac_layer_ = props && props->getAsBool("macRouting");
    useManetLabelRouting = par("useManetLabelRouting");

    const char *interfaces = par("interfaces");
    cStringTokenizer tokenizerInterfaces(interfaces);
    const char *token;
    const char * prefixName;

    if (!mac_layer_)
         setStaticNode(par("isStaticNode").boolValue());

    if (!mac_layer_)
    {
        while ((token = tokenizerInterfaces.nextToken()) != NULL)
        {
            if ((prefixName = strstr(token, "prefix")) != NULL)
            {
                const char *leftparenp = strchr(prefixName, '(');
                const char *rightparenp = strchr(prefixName, ')');
                std::string interfacePrefix;
                interfacePrefix.assign(leftparenp + 1, rightparenp - leftparenp - 1);
                for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
                {
                    ie = inet_ift->getInterface(i);
                    name = ie->getName();
                    if ((strstr(name, interfacePrefix.c_str()) != NULL) && !isThisInterfaceRegistered(ie))
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
                    name = ie->getName();
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
        cModule *mod = getParentModule()->getParentModule();
        char *interfaceName = new char[strlen(mod->getFullName()) + 1];
        char *d = interfaceName;
        for (const char *s = mod->getFullName(); *s; s++)
            if (isalnum(*s))
                *d++ = *s;
        *d = '\0';

        for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
        {
            ie = inet_ift->getInterface(i);
            name = ie->getName();
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
        while ((token = tokenizerExcluded.nextToken())!=NULL)
        {
            for (unsigned int i = 0; i<interfaceVector->size(); i++)
            {
                name = (*interfaceVector)[i].interfacePtr->getName();
                if (strcmp(token, name)==0)
                {
                    interfaceVector->erase(interfaceVector->begin()+i);
                    break;
                }
            }
        }
    }

    if (inet_rt)
        routerId = L3Address(inet_rt->getRouterId());

    if (interfaceVector->size()==0)
        throw cRuntimeError("Manet routing protocol has found no interfaces that can be used for routing.");
    if (mac_layer_)
        hostAddress = L3Address(interfaceVector->front().interfacePtr->getMacAddress());
    else
        hostAddress = L3Address(interfaceVector->front().interfacePtr->ipv4Data()->getIPAddress());
    // One enabled network interface (in total)
    // clear routing entries related to wlan interfaces and autoassign ip adresses
    bool manetPurgeRoutingTables = (bool) par("manetPurgeRoutingTables");
    if (manetPurgeRoutingTables && !mac_layer_)
    {
        IPv4Route *entry;
        // clean the route table wlan interface entry
        for (int i=inet_rt->getNumRoutes()-1; i>=0; i--)
        {
            entry = inet_rt->getRoute(i);
            const InterfaceEntry *ie = entry->getInterface();
            if (strstr(ie->getName(), "wlan")!=NULL)
            {
                inet_rt->deleteRoute(entry);
            }
        }
    }
    if (par("autoassignAddress") && !mac_layer_)
    {
        IPv4Address AUTOASSIGN_ADDRESS_BASE(par("autoassignAddressBase").stringValue());
        if (AUTOASSIGN_ADDRESS_BASE.getInt() == 0)
            throw cRuntimeError("Auto assignment need autoassignAddressBase to be set");
        IPv4Address myAddr(AUTOASSIGN_ADDRESS_BASE.getInt() + uint32(getParentModule()->getId()));
        for (int k=0; k<inet_ift->getNumInterfaces(); k++)
        {
            InterfaceEntry *ie = inet_ift->getInterface(k);
            if (strstr(ie->getName(), "wlan")!=NULL)
            {
                ie->ipv4Data()->setIPAddress(myAddr);
                ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }
    }
    // register LL-MANET-Routers
    if (!mac_layer_)
    {
        for (unsigned int i = 0; i<interfaceVector->size(); i++)
        {
            (*interfaceVector)[i].interfacePtr->ipv4Data()->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);
        }
        arp = getModuleFromPar<IARP>(par("arpModule"), this);
    }
    hostModule->subscribe(NF_L2_AP_DISASSOCIATED, this);
    hostModule->subscribe(NF_L2_AP_ASSOCIATED, this);

    if (par("PublicRoutingTables").boolValue())
    {
        setInternalStore(true);
        if (globalRouteMap == NULL)
        {
            globalRouteMap = new GlobalRouteMap;
        }

        GlobalRouteMap::iterator it = globalRouteMap->find(getAddress());
        if (it == globalRouteMap->end())
        {
            ProtocolRoutingData data;
            ProtocolsRoutes vect;
            data.isProactive = isProactive();
            data.routesVector = routesVector;
            vect.push_back(data);
            globalRouteMap->insert(std::pair<L3Address,ProtocolsRoutes>(getAddress(),vect));
        }
        else
        {
            ProtocolRoutingData data;
            data.isProactive = isProactive();
            data.routesVector = routesVector;
            it->second.push_back(data);
        }
    }
    ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
    initHook(this);

 //   WATCH_MAP(*routesVector);
    IPSocket socket(gate("to_ip"));
    socket.registerProtocol(IP_PROT_MANET);
}

ManetRoutingBase::~ManetRoutingBase()
{
    delete interfaceVector;
    if (routesVector)
    {
        delete routesVector;
        routesVector = NULL;
    }
    proxyAddress.clear();
    addressGroupVector.clear();
    inAddressGroup.clear();

    if (globalRouteMap)
    {
        GlobalRouteMap::iterator it = globalRouteMap->find(getAddress());
        if (it != globalRouteMap->end())
            globalRouteMap->erase(it);
        if (globalRouteMap->empty())
        {
            delete globalRouteMap;
            globalRouteMap = NULL;
        }
    }
}

bool ManetRoutingBase::isIpLocalAddress(const IPv4Address& dest) const
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
        return inet_rt->isLocalAddress(dest.toIPv4());
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
    hostModule->subscribe(NF_LINK_BREAK, this);
}

void ManetRoutingBase::linkPromiscuous()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    hostModule->subscribe(NF_LINK_PROMISCUOUS, this);
}

void ManetRoutingBase::linkFullPromiscuous()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    hostModule->subscribe(NF_LINK_FULL_PROMISCUOUS, this);
}

void ManetRoutingBase::registerPosition()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    regPosition = true;
    cModule *mod = findContainingNode(getParentModule());
    if (!mod)
        mod = getParentModule();
    mod->subscribe(mobilityStateChangedSignal, this);
}

void ManetRoutingBase::processLinkBreak(const cObject *details) {return;}
void ManetRoutingBase::processLinkBreakManagement(const cObject *details) {return;}
void ManetRoutingBase::processPromiscuous(const cObject *details) {return;}
void ManetRoutingBase::processFullPromiscuous(const cObject *details) {return;}
void ManetRoutingBase::processLocatorAssoc(const cObject *details) {return;}
void ManetRoutingBase::processLocatorDisAssoc(const cObject *details) {return;}


void ManetRoutingBase::sendToIpOnIface(cPacket *msg, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, InterfaceEntry  *ie)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    if (destAddr.getType() == L3Address::MAC)
    {
        Ieee802Ctrl *ctrl = new Ieee802Ctrl;
        //TODO ctrl->setEtherType(...);
        MACAddress macadd = destAddr.toMAC();
        ctrl->setDest(macadd);

        if (ie == NULL)
            ie = interfaceVector->back().interfacePtr;

        if (macadd == MACAddress::BROADCAST_ADDRESS)
        {
            for (unsigned int i = 0; i<interfaceVector->size()-1; i++)
            {
// It's necessary to duplicate the the control info message and include the information relative to the interface
                Ieee802Ctrl *ctrlAux = ctrl->dup();
                ie = (*interfaceVector)[i].interfacePtr;
                cPacket *msgAux = msg->dup();
// Set the control info to the duplicate packet
                if (ie)
                    ctrlAux->setInterfaceId(ie->getInterfaceId());
                msgAux->setControlInfo(ctrlAux);
                sendDelayed(msgAux, delay, "to_ip");

            }
            ie = interfaceVector->back().interfacePtr;
        }

        if (ie)
            ctrl->setInterfaceId(ie->getInterfaceId());
        msg->setControlInfo(ctrl);
        sendDelayed(msg, delay, "to_ip");
        return;
    }

    UDPPacket *udpPacket = new UDPPacket(msg->getName());
    udpPacket->setByteLength(UDP_HDR_LEN);
    udpPacket->encapsulate(msg);
    //Address srcAddr = interfaceWlanptr->ipv4Data()->getIPAddress();

    if (ttl==0)
    {
        // delete and return
        delete msg;
        return;
    }
    // set source and destination port
    udpPacket->setSourcePort(srcPort);
    udpPacket->setDestinationPort(destPort);

    if (destAddr.getType() == L3Address::IPv4)
    {
        // send to IPv4
        IPv4Address add(destAddr.toIPv4());
        IPv4Address  srcadd;

// If found interface We use the address of interface
        if (ie)
            srcadd = ie->ipv4Data()->getIPAddress();
        else
            srcadd = hostAddress.toIPv4();

        EV_INFO << "Sending app packet " << msg->getName() << " over IPv4." << " from " <<
        srcadd.str() << " to " << add.str() << "\n";
        IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
        ipControlInfo->setDestAddr(add);
        //ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setProtocol(IP_PROT_MANET);

        ipControlInfo->setTimeToLive(ttl);
        udpPacket->setControlInfo(ipControlInfo);

        if (ie!=NULL)
            ipControlInfo->setInterfaceId(ie->getInterfaceId());

        if ((add == IPv4Address::ALLONES_ADDRESS || add == IPv4Address::LL_MANET_ROUTERS) && ie == NULL)
        {
// In this case we send a broadcast packet per interface
            for (unsigned int i = 0; i<interfaceVector->size()-1; i++)
            {
                ie = (*interfaceVector)[i].interfacePtr;
                srcadd = ie->ipv4Data()->getIPAddress();
// It's necessary to duplicate the the control info message and include the information relative to the interface
                IPv4ControlInfo *ipControlInfoAux = new IPv4ControlInfo(*ipControlInfo);
                ipControlInfoAux->setInterfaceId(ie->getInterfaceId());
                ipControlInfoAux->setSrcAddr(srcadd);
                UDPPacket *udpPacketAux = udpPacket->dup();
// Set the control info to the duplicate udp packet
                udpPacketAux->setControlInfo(ipControlInfoAux);
                sendDelayed(udpPacketAux, delay, "to_ip");
            }
            ie = interfaceVector->back().interfacePtr;
            srcadd = ie->ipv4Data()->getIPAddress();
            ipControlInfo->setInterfaceId(ie->getInterfaceId());
        }
        ipControlInfo->setSrcAddr(srcadd);
        sendDelayed(udpPacket, delay, "to_ip");
    }
    else if (destAddr.getType() == L3Address::IPv6)
    {
        // send to IPv6
        EV_INFO << "Sending app packet " << msg->getName() << " over IPv6.\n";
        INetworkProtocolControlInfo *ipControlInfo = IPv6AddressType::INSTANCE.createNetworkProtocolControlInfo();
        // ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setTransportProtocol(IP_PROT_MANET);
        ipControlInfo->setSourceAddress(hostAddress);
        ipControlInfo->setDestinationAddress(destAddr);
        ipControlInfo->setHopLimit(ttl);
        //ipControlInfo->setInterfaceId(udpCtrl->getInterfaceId()); FIXME extend IPv6 with this!!!
        udpPacket->setControlInfo(check_and_cast<cObject *>(ipControlInfo));
        sendDelayed(udpPacket, delay, "to_ip");
    }
    else
    {
        throw cRuntimeError("Unaccepted Address type: %d", destAddr.getType());
    }
    // totalSend++;
}

void ManetRoutingBase::sendToIp(cPacket *msg, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, const L3Address &iface)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    InterfaceEntry  *ie = NULL;
    if (!iface.isUnspecified())
        ie = getInterfaceWlanByAddress(iface); // The user want to use a pre-defined interface

    sendToIpOnIface(msg, srcPort, destAddr, destPort, ttl, delay, ie);
}

void ManetRoutingBase::sendToIp(cPacket *msg, int srcPort, const L3Address& destAddr, int destPort, int ttl, double delay, int index)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    InterfaceEntry  *ie = NULL;
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
    else if (add.toIPv4() == IPv4Address::ALLONES_ADDRESS) return false;
    else return true;
}

void ManetRoutingBase::omnet_chg_rte(const L3Address &dst, const L3Address &gtwy, const L3Address &netm, short int hops, bool del_entry, const L3Address &iface)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    setRouteInternalStorege(dst, gtwy, del_entry);

    if (mac_layer_)
        return;
    IPv4Address desAddress(dst.toIPv4());

    bool found = false;
    IPv4Route *oldentry = NULL;
    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        IPv4Route *e = inet_rt->getRoute(i-1);
        if (desAddress == e->getDestination())
        {
            if (del_entry && !found)
            {
                if (!inet_rt->deleteRoute(e))
                    throw cRuntimeError("Aodv omnet_chg_rte can't delete route entry");
            }
            else
            {
                found = true;
                oldentry = e;
            }
        }
    }

    if (del_entry)
        return;

    IPv4Address netmask(netm.toIPv4());
    IPv4Address gateway(gtwy.toIPv4());

    // The default mask is for manet routing is  IPv4Address::ALLONES_ADDRESS
    if (netm.isUnspecified())
        netmask = IPv4Address::ALLONES_ADDRESS;

    InterfaceEntry *ie = getInterfaceWlanByAddress(iface);
    IRoute::SourceType sourceType = useManetLabelRouting ? IRoute::MANET : IRoute::MANET2;

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSourceType() == sourceType)
            return;
        inet_rt->deleteRoute(oldentry);
    }

    IPv4Route *entry = new IPv4Route();

    /// Destination
    entry->setDestination(desAddress);
    /// Route mask
    entry->setNetmask(netmask);
    /// Next hop
    entry->setGateway(gateway);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(ie);

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


void ManetRoutingBase::omnet_chg_rte(const L3Address &dst, const L3Address &gtwy, const L3Address &netm, short int hops, bool del_entry, int index)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    setRouteInternalStorege(dst, gtwy, del_entry);
    if (mac_layer_)
        return;

    IPv4Address desAddress(dst.toIPv4());
    bool found = false;
    IPv4Route *oldentry = NULL;
    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        IPv4Route *e = inet_rt->getRoute(i-1);
        if (desAddress == e->getDestination())
        {
            if (del_entry && !found)
            {
                if (!inet_rt->deleteRoute(e))
                    throw cRuntimeError("Aodv omnet_chg_rte can't delete route entry");
            }
            else
            {
                found = true;
                oldentry = e;
            }
        }
    }

    if (del_entry)
        return;

    IPv4Address netmask(netm.toIPv4());
    IPv4Address gateway(gtwy.toIPv4());
    if (netm.isUnspecified())
        netmask = IPv4Address::ALLONES_ADDRESS;

    InterfaceEntry *ie = getInterfaceEntry(index);
    IRoute::SourceType sourceType = useManetLabelRouting ? IRoute::MANET : IRoute::MANET2;

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSourceType() == sourceType)
            return;
        inet_rt->deleteRoute(oldentry);
    }

    IPv4Route *entry = new IPv4Route();

    /// Destination
    entry->setDestination(desAddress);
    /// Route mask
    entry->setNetmask(netmask);
    /// Next hop
    entry->setGateway(gateway);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(getInterfaceEntry(index));

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise

    if (useManetLabelRouting)
        entry->setSourceType(IRoute::MANET);
    else
        entry->setSourceType(IRoute::MANET2);

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

    IPv4Address desAddress(dst.toIPv4());
    const IPv4Route *e = NULL;

    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        e = inet_rt->getRoute(i-1);
        if (desAddress == e->getDestination())
            return L3Address(e->getGateway());
    }
    return L3Address(IPv4Address::ALLONES_ADDRESS);
}

//
// Erase all the entries in the routing table
//
void ManetRoutingBase::omnet_clean_rte()
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    IPv4Route *entry;
    if (mac_layer_)
        return;
    // clean the route table wlan interface entry
    for (int i=inet_rt->getNumRoutes()-1; i>=0; i--)
    {
        entry = inet_rt->getRoute(i);
        if (strstr(entry->getInterface()->getName(), "wlan")!=NULL)
        {
            inet_rt->deleteRoute(entry);
        }
    }
}

//
// generic receiveChangeNotification, the protocols must implement processLinkBreak and processPromiscuous only
//
void ManetRoutingBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method("Manet llf");
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");
    if (signalID == NF_LINK_BREAK)
    {
        if (obj == NULL)
            return;
        if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(const_cast<cObject*>(obj)))
        {
            Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*>(obj));
            if (frame)
            {
                cPacket * pktAux = frame->getEncapsulatedPacket();
                if (!mac_layer_ && pktAux != NULL)
                {
                    cPacket *pkt = pktAux->dup();
                    ControlInfoBreakLink *add = new ControlInfoBreakLink;
                    add->setDest(frame->getReceiverAddress());
                    pkt->setControlInfo(add);
                    processLinkBreak(pkt);
                    delete pkt;
                }
                else
                    processLinkBreak(obj);
            }
            else
            {
                Ieee80211ManagementFrame *frame =
                        dynamic_cast<Ieee80211ManagementFrame *>(const_cast<cObject*>(obj));
                if (frame)
                    processLinkBreakManagement(obj);
            }
        }
    }
    else if (signalID == NF_LINK_PROMISCUOUS)
    {
        processPromiscuous(obj);
    }
    else if (signalID == NF_LINK_FULL_PROMISCUOUS)
    {
        processFullPromiscuous(obj);
    }
    else if(signalID == NF_L2_AP_DISASSOCIATED || signalID == NF_L2_AP_ASSOCIATED)
    {
        Ieee80211MgmtAP::NotificationInfoSta *infoSta = dynamic_cast<Ieee80211MgmtAP::NotificationInfoSta *>(const_cast<cObject*> (obj));
        if (infoSta)
        {
            L3Address addr;
            if (!mac_layer_ && arp)
                addr = arp->getL3AddressFor(infoSta->getStaAddress());
            else
                addr = L3Address(infoSta->getStaAddress());
            // sanity check
            for (unsigned int i = 0; i< proxyAddress.size(); i++)
            {
                 if (proxyAddress[i].address == addr)
                 {
                     proxyAddress.erase(proxyAddress.begin()+i);
                     break;
                 }
            }
            if (signalID == NF_L2_AP_ASSOCIATED)
            {
                ManetProxyAddress p;
                p.address = addr;
                p.mask = mac_layer_ ? L3Address(MACAddress::BROADCAST_ADDRESS) : L3Address(IPv4Address::ALLONES_ADDRESS);
                proxyAddress.push_back(p);
            }
        }
    }
    else if (signalID == mobilityStateChangedSignal)
    {
        IMobility *mobility = check_and_cast<IMobility*>(obj);
        curPosition = mobility->getCurrentPosition();
        curSpeed = mobility->getCurrentSpeed();
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

    for (unsigned int i=0; i<interfaceVector->size(); i++)
    {
        if (add.getType() == L3Address::MAC)
        {
            if ((*interfaceVector)[i].interfacePtr->getMacAddress() == add.toMAC())
                return (*interfaceVector)[i].index;
        }
        else
        {
            if ((*interfaceVector)[i].interfacePtr->ipv4Data()->getIPAddress() == add.toIPv4())
                return (*interfaceVector)[i].index;
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

    for (unsigned int i=0; i<interfaceVector->size(); i++)
    {
        if (add.getType() == L3Address::MAC)
        {
            if ((*interfaceVector)[i].interfacePtr->getMacAddress() == add.toMAC())
                return (*interfaceVector)[i].interfacePtr;
        }
        else
        {
            if ((*interfaceVector)[i].interfacePtr->ipv4Data()->getIPAddress() == add.toIPv4())
                return (*interfaceVector)[i].interfacePtr;
        }
    }
    return NULL;
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
        return NULL;
}


//////////////////
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
        routesVector = NULL;
    }
    else
    {
        if (routesVector==NULL)
            routesVector = new RouteMap;
    }
}


L3Address ManetRoutingBase::getNextHopInternal(const L3Address &dest)
{
    if (routesVector==NULL)
        return L3Address();
    if (routesVector->empty())
        return L3Address();
    RouteMap::iterator it = routesVector->find(dest);
    if (it!=routesVector->end())
        return it->second;
    return L3Address();
}

bool ManetRoutingBase::setRoute(const L3Address & destination, const L3Address &nextHop, const int &ifaceIndex, const int &hops, const L3Address &mask)
{
    if (!isRegistered)
        throw cRuntimeError("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    IPv4Address desAddress(destination.toIPv4());
    bool del_entry = (nextHop.isUnspecified());

    setRouteInternalStorege(destination, nextHop, del_entry);

    if (mac_layer_)
        return true;

    if (ifaceIndex>=getNumInterfaces())
        return false;

    bool found = false;
    IPv4Route *oldentry = NULL;

    //TODO the entries with ALLONES netmasks stored at the begin of inet route entry vector,
    // let optimise next search!
    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        IPv4Route *e = inet_rt->getRoute(i-1);
        if (desAddress == e->getDestination())     // FIXME netmask checking?
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

    IPv4Address netmask(mask.toIPv4());
    IPv4Address gateway(nextHop.toIPv4());
    if (mask.isUnspecified())
        netmask = IPv4Address::ALLONES_ADDRESS;
    InterfaceEntry *ie = getInterfaceEntry(ifaceIndex);

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSourceType() == IRoute::MANUAL)
            return true;
        inet_rt->deleteRoute(oldentry);
    }

    IPv4Route *entry = new IPv4Route();

    /// Destination
    entry->setDestination(desAddress);
    /// Route mask
    entry->setNetmask(netmask);
    /// Next hop
    entry->setGateway(gateway);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(ie);

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
        if (strcmp(ifaceName, getInterfaceEntry(index)->getName())==0) break;
    }
    if (index>=getNumInterfaces())
        return false;
    return setRoute(destination, nextHop, index, hops, mask);
};

void ManetRoutingBase::sendICMP(cPacket *pkt)
{
    if (pkt==NULL)
        return;

    if (icmpModule==NULL || !sendToICMP)
    {
        delete pkt;
        return;
    }

    if (mac_layer_)
    {
        // The packet is encapsulated in a Ieee802 frame
        cPacket *pktAux = pkt->decapsulate();
        if (pktAux)
        {
            delete pkt;
            pkt = pktAux;
        }
    }

    IPv4Datagram *datagram = dynamic_cast<IPv4Datagram*>(pkt);
    if (datagram==NULL)
    {
        delete pkt;
        return;
    }
    // don't send ICMP error messages for multicast messages
    if (datagram->getDestAddress().isMulticast())
    {
        EV_INFO << "won't send ICMP error messages for multicast message " << datagram << endl;
        delete pkt;
        return;
    }
    // check source address
    if (datagram->getSrcAddress().isUnspecified() && par("setICMPSourceAddress"))
        datagram->setSrcAddress(inet_ift->getInterface(0)->ipv4Data()->getIPAddress());
    EV_DETAIL << "issuing ICMP Destination Unreachable for packets waiting in queue for failed route discovery.\n";
    icmpModule->sendErrorMessage(datagram, -1 /*TODO*/, ICMP_DESTINATION_UNREACHABLE, 0);
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
        for (unsigned int i=0; i<inAddressGroup.size(); i++)
        {
            if (inAddressGroup[i]==group)
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
        for (AddressGroupIterator it = addressGroupVector[group].begin(); it!=addressGroupVector[group].end(); it++)
            if (isLocalAddress(*it)) return true;
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
    for (unsigned int i=0; i<inAddressGroup.size(); i++)
        if (group==inAddressGroup[i])
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
    for (AddressGroupIterator it=addressGroupVector[group].begin(); it!=addressGroupVector[group].end(); it++)
        addressGroup.push_back(*it);
    return true;
}


bool ManetRoutingBase::isAddressInProxyList(const L3Address & addr)
{
    if (proxyAddress.empty())
        return false;
    for (unsigned int i = 0; i < proxyAddress.size(); i++)
    {
        //if ((addr & proxyAddress[i].mask) == proxyAddress[i].address)
        if (addr == proxyAddress[i].address)   //FIXME
            return true;
    }
    return false;
}

void ManetRoutingBase::setAddressInProxyList(const L3Address & addr,const L3Address & mask)
{
    // search if exist
    for (unsigned int i = 0; i < proxyAddress.size(); i++)
    {
        if ((addr == proxyAddress[i].address) && (mask == proxyAddress[i].mask))
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
    for (unsigned int i = 0; i < proxyAddress.size(); i++)
    {
        //if ((addr & proxyAddress[i].mask) == proxyAddress[i].address)
        if (addr == proxyAddress[i].address)   //FIXME
            return true;
    }
    return false;
}

bool ManetRoutingBase::getAp(const L3Address &destination, L3Address& accesPointAddr) const
{
    return false;
}

void ManetRoutingBase::getApList(const MACAddress & dest,std::vector<MACAddress>& list)
{
    list.clear();
    list.push_back(dest);
}

void ManetRoutingBase::getApListIp(const IPv4Address &dest,std::vector<IPv4Address>& list)
{
    list.clear();
    list.push_back(dest);
}

void ManetRoutingBase::getListRelatedAp(const L3Address & add, std::vector<L3Address>& list)
{
    if (add.getType() == L3Address::MAC)
    {
        std::vector<MACAddress> listAux;
        getApList(add.toMAC(), listAux);
        list.clear();
        for (unsigned int i = 0; i < listAux.size(); i++)
        {
            list.push_back(L3Address(listAux[i]));
        }
    }
    else
    {
        std::vector<IPv4Address> listAux;
        getApListIp(add.toIPv4(), listAux);
        list.clear();
        for (unsigned int i = 0; i < listAux.size(); i++)
        {
            list.push_back(L3Address(listAux[i]));
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
         routesVector = NULL;
     }
     else if (createInternalStore && routesVector)
     {
         RouteMap::iterator it = routesVector->find(dest);
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
    if (!createInternalStore || globalRouteMap == NULL)
        return false;
    L3Address next = src;
    route.clear();
    route.push_back(src);
    while (1)
    {
        GlobalRouteMap::iterator it = globalRouteMap->find(next);
        if (it==globalRouteMap->end())
            return false;
        if (it->second.empty())
            return false;

        if (it->second.size() == 1)
        {
            RouteMap *rt = it->second[0].routesVector;
            RouteMap::iterator it2 = rt->find(dest);
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
            RouteMap::iterator it2 = rt->find(dest);
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
    throw cRuntimeError("model error");
}


// Auxiliary function that return a string with the address
std::string ManetRoutingBase::convertAddressToString(const L3Address& add)
{
    return add.str();
}

} // namespace inetmanet

} // namespace inet

