/***************************************************************************
 *   Copyright (C) 2008 by Alfonso Ariza                                   *
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

#include "ManetRoutingBase.h"
#include "UDPPacket.h"
#include "IPv4Datagram.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPv6ControlInfo.h"
#include "Ieee802Ctrl_m.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "Coord.h"
#include "ControlInfoBreakLink_m.h"
#include "Ieee80211Frame_m.h"
#include "ICMPAccess.h"
#include "IMobility.h"
#include "Ieee80211MgmtAP.h"

#define IP_DEF_TTL 32
#define UDP_HDR_LEN 8

simsignal_t ManetRoutingBase::mobilityStateChangedSignal = SIMSIGNAL_NULL;
ManetRoutingBase::GlobalRouteMap *ManetRoutingBase::globalRouteMap = NULL;
bool ManetRoutingBase::createInternalStore = false;


ManetRoutingBase::ManetRoutingBase()
{
#ifdef WITH_80211MESH
    locator = NULL;
#endif
    isRegistered = false;
    regPosition = false;
    mac_layer_ = false;
    commonPtr = NULL;
    routesVector = NULL;
    interfaceVector = new InterfaceVector;
    staticNode = false;
    colaborativeProtocol = NULL;
    arp = NULL;
    isGateway = false;
    proxyAddress.clear();
    addressGroupVector.clear();
    inAddressGroup.clear();

}


bool ManetRoutingBase::isThisInterfaceRegistered(InterfaceEntry *ie)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
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
    inet_rt = RoutingTableAccess().getIfExists();
    inet_ift = InterfaceTableAccess().get();
    nb = NotificationBoardAccess().get();

    if (routesVector)
        routesVector->clear();

    if (par("useICMP"))
    {
        icmpModule = ICMPAccess().getIfExists();
    }
    sendToICMP = false;

    cProperties *props = getParentModule()->getProperties();
    mac_layer_ = props && props->getAsBool("macRouting");
    usetManetLabelRouting = par("usetManetLabelRouting");

    const char *interfaces = par("interfaces");
    cStringTokenizer tokenizerInterfaces(interfaces);
    const char *token;
    const char *prefixName;
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
        routerId = ManetAddress(inet_rt->getRouterId());

    if (interfaceVector->size()==0)
        opp_error("Manet routing protocol has found no interfaces that can be used for routing.");
    if (mac_layer_)
        hostAddress = ManetAddress(interfaceVector->front().interfacePtr->getMacAddress());
    else
        hostAddress = ManetAddress(interfaceVector->front().interfacePtr->ipv4Data()->getIPAddress());
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
            opp_error("Auto assignment need autoassignAddressBase to be set");
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
        arp = ArpAccess().get();
    }
    nb->subscribe(this,NF_L2_AP_DISASSOCIATED);
    nb->subscribe(this,NF_L2_AP_ASSOCIATED);

#ifdef WITH_80211MESH
    locator = LocatorModuleAccess().getIfExists();
    if (locator)
    {
        InterfaceEntry *ie = getInterfaceWlanByAddress();
        if (locator->getIpAddress().isUnspecified() && ie->ipv4Data())
            locator->setIpAddress(ie->ipv4Data()->getIPAddress());
        if (locator->getMacAddress().isUnspecified())
            locator->setMacAddress(ie->getMacAddress());
        nb->subscribe(this,NF_LOCATOR_ASSOC);
        nb->subscribe(this,NF_LOCATOR_DISASSOC);
    }
#endif

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
            globalRouteMap->insert(std::make_pair<ManetAddress,ProtocolsRoutes>(getAddress(),vect));
        }
        else
        {
            ProtocolRoutingData data;
            data.isProactive = isProactive();
            data.routesVector = routesVector;
            it->second.push_back(data);
        }
    }


 //   WATCH_MAP(*routesVector);
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

bool ManetRoutingBase::isLocalAddress(const ManetAddress& dest) const
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    if (!mac_layer_)
        return inet_rt->isLocalAddress(dest.getIPv4());
    InterfaceEntry *ie;
    for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
    {
        ie = inet_ift->getInterface(i);
        ManetAddress add(ie->getMacAddress());
        if (add==dest) return true;
    }
    return false;
}

bool ManetRoutingBase::isMulticastAddress(const ManetAddress& dest) const
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    if (mac_layer_)
        return dest.getMAC() == MACAddress::BROADCAST_ADDRESS;
    else
        return dest.getIPv4() == IPv4Address::ALLONES_ADDRESS;
}

void ManetRoutingBase::linkLayerFeeback()
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    nb->subscribe(this, NF_LINK_BREAK);
}

void ManetRoutingBase::linkPromiscuous()
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    nb->subscribe(this, NF_LINK_PROMISCUOUS);
}

void ManetRoutingBase::linkFullPromiscuous()
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    nb->subscribe(this, NF_LINK_FULL_PROMISCUOUS);
}

void ManetRoutingBase::registerPosition()
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    regPosition = true;
    mobilityStateChangedSignal = registerSignal("mobilityStateChanged");
    cModule *mod;
    for (mod = getParentModule(); mod != 0; mod = mod->getParentModule()) {
            cProperties *properties = mod->getProperties();
            if (properties && properties->getAsBool("node"))
                break;
    }
    if (mod)
        mod->subscribe(mobilityStateChangedSignal, this);
    else
        getParentModule()->subscribe(mobilityStateChangedSignal, this);
}

void ManetRoutingBase::processLinkBreak(const cObject *details) {return;}
void ManetRoutingBase::processLinkBreakManagement(const cObject *details) {return;}
void ManetRoutingBase::processPromiscuous(const cObject *details) {return;}
void ManetRoutingBase::processFullPromiscuous(const cObject *details) {return;}
void ManetRoutingBase::processLocatorAssoc(const cObject *details) {return;}
void ManetRoutingBase::processLocatorDisAssoc(const cObject *details) {return;}


void ManetRoutingBase::sendToIpOnIface(cPacket *msg, int srcPort, const ManetAddress& destAddr, int destPort, int ttl, double delay, InterfaceEntry  *ie)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    if (destAddr.getType() == ManetAddress::MAC_ADDRESS)
    {
        Ieee802Ctrl *ctrl = new Ieee802Ctrl;
        //TODO ctrl->setEtherType(...);
        MACAddress macadd = destAddr.getMAC();
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
                    ctrlAux->setInputPort(ie->getInterfaceId());
                msgAux->setControlInfo(ctrlAux);
                sendDelayed(msgAux, delay, "to_ip");

            }
            ie = interfaceVector->back().interfacePtr;
        }

        if (ie)
            ctrl->setInputPort(ie->getInterfaceId());
        msg->setControlInfo(ctrl);
        sendDelayed(msg, delay, "to_ip");
        return;
    }

    UDPPacket *udpPacket = new UDPPacket(msg->getName());
    udpPacket->setByteLength(UDP_HDR_LEN);
    udpPacket->encapsulate(msg);
    //IPvXAddress srcAddr = interfaceWlanptr->ipv4Data()->getIPAddress();

    if (ttl==0)
    {
        // delete and return
        delete msg;
        return;
    }
    // set source and destination port
    udpPacket->setSourcePort(srcPort);
    udpPacket->setDestinationPort(destPort);

    if (destAddr.getType() == ManetAddress::IPv4_ADDRESS)
    {
        // send to IPv4
        IPv4Address add(destAddr.getIPv4());
        IPv4Address  srcadd;

// If found interface We use the address of interface
        if (ie)
            srcadd = ie->ipv4Data()->getIPAddress();
        else
            srcadd = hostAddress.getIPv4();

        EV << "Sending app packet " << msg->getName() << " over IPv4." << " from " <<
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
                if (ipControlInfoAux->getOrigDatagram())
                    delete ipControlInfoAux->removeOrigDatagram();
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
    else if (destAddr.getType() == ManetAddress::IPv6_ADDRESS)
    {
        // send to IPv6
        EV << "Sending app packet " << msg->getName() << " over IPv6.\n";
        IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
        // ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setProtocol(IP_PROT_MANET);
        ipControlInfo->setSrcAddr(hostAddress.getIPv6());
        ipControlInfo->setDestAddr(destAddr.getIPv6());
        ipControlInfo->setHopLimit(ttl);
        // ipControlInfo->setInterfaceId(udpCtrl->InterfaceId()); FIXME extend IPv6 with this!!!
        udpPacket->setControlInfo(ipControlInfo);
        sendDelayed(udpPacket, delay, "to_ip");
    }
    else
    {
        throw cRuntimeError("Unaccepted ManetAddress type: %d", destAddr.getType());
    }
    // totalSend++;
}

void ManetRoutingBase::sendToIp(cPacket *msg, int srcPort, const ManetAddress& destAddr, int destPort, int ttl, double delay, const ManetAddress &iface)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    InterfaceEntry  *ie = NULL;
    if (!iface.isUnspecified())
        ie = getInterfaceWlanByAddress(iface); // The user want to use a pre-defined interface

    sendToIpOnIface(msg, srcPort, destAddr, destPort, ttl, delay, ie);
}

void ManetRoutingBase::sendToIp(cPacket *msg, int srcPort, const ManetAddress& destAddr, int destPort, int ttl, double delay, int index)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

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
    ManetAddress add = omnet_exist_rte(dst.s_addr);
    if (add.isUnspecified()) return false;
    else if (add.getIPv4() == IPv4Address::ALLONES_ADDRESS) return false;
    else return true;
}

void ManetRoutingBase::omnet_chg_rte(const ManetAddress &dst, const ManetAddress &gtwy, const ManetAddress &netm, short int hops, bool del_entry, const ManetAddress &iface)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    IPv4Address desAddress(dst.getIPv4());
    if (!createInternalStore && routesVector)
    {
        delete routesVector;
        routesVector = NULL;
    }
    else if (createInternalStore && routesVector)
    {
        RouteMap::iterator it = routesVector->find(dst);
        if (it != routesVector->end())
            routesVector->erase(it);
        if (!del_entry)
        {
            /*
            ManetAddress dest=dst;
            ManetAddress next=gtwy;
            if (mac_layer_)
            {
                dest.setAddresType(ManetAddress::MAC);
                next.setAddresType(ManetAddress::MAC);
            }
            else
            {
                dest.setAddresType(ManetAddress::IPV4);
                next.setAddresType(ManetAddress::IPV4);
            }*/
            routesVector->insert(std::make_pair<ManetAddress,ManetAddress>(dst, gtwy));
        }
    }

    if (mac_layer_)
        return;

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
                    opp_error("Aodv omnet_chg_rte can't delete route entry");
            }
            else
            {
                found = true;
                oldentry = e;
            }
        }
    }

#ifdef WITH_80211MESH
    if (locator && locator->isApIp(desAddress) && del_entry)
    {
        std::vector<IPv4Address> list;
        locator->getApListIp(desAddress,list);
        for (unsigned int i = 0; i<list.size(); i++)
        {
            for (int i=inet_rt->getNumRoutes(); i>0; --i)
            {
                IPv4Route *e = inet_rt->getRoute(i-1);
                if (list[i] == e->getDestination())
                {
                    if (!inet_rt->deleteRoute(e))
                        opp_error("Aodv omnet_chg_rte can't delete route entry");
                    else
                        break;
                }
            }
        }
    }
#endif
    if (del_entry)
        return;

    IPv4Address netmask(netm.getIPv4());
    IPv4Address gateway(gtwy.getIPv4());

    // The default mask is for manet routing is  IPv4Address::ALLONES_ADDRESS
    if (netm.isUnspecified())
        netmask = IPv4Address::ALLONES_ADDRESS;

    InterfaceEntry *ie = getInterfaceWlanByAddress(iface);
    IPv4Route::RouteSource routeSource = usetManetLabelRouting ? IPv4Route::MANET : IPv4Route::MANET2;

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSource() == routeSource)
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
    entry->setSource(routeSource);
    inet_rt->addRoute(entry);
#ifdef WITH_80211MESH
    if (locator && locator->isApIp(desAddress))
    {
        std::vector<IPv4Address> list;
        locator->getApListIp(desAddress,list);
        for (unsigned int i = 0; i<list.size(); i++)
        {
            IPv4Route *e = inet_rt->findBestMatchingRoute(list[i]);
            if (e && e->getDestination() == list[i])
            {
                if (e->getGateway() == gateway && e->getMetric() == hops && e->getInterface() == ie)
                    continue;
            }

            IPv4Route *entry = new IPv4Route();

            /// Destination
            entry->setDestination(list[i]);
            /// Route mask
            entry->setNetmask(netmask);
            /// Next hop
            entry->setGateway(gateway);
            /// Metric ("cost" to reach the destination)
            entry->setMetric(hops);
            /// Interface name and pointer

            entry->setInterface(ie);

            /// Route type: Direct or Remote
            entry->setType(routeType);
            /// Source of route, MANUAL by reading a file,
            /// routing protocol name otherwise
            entry->setSource(routeSource);
            inet_rt->addRoute(entry);
        }
    }
#endif
}

// This methods use the nic index to identify the output nic.
void ManetRoutingBase::omnet_chg_rte(const struct in_addr &dst, const struct in_addr &gtwy, const struct in_addr &netm,
                                      short int hops, bool del_entry, int index)
{
    omnet_chg_rte(dst.s_addr, gtwy.s_addr, netm.s_addr, hops, del_entry, index);
}


void ManetRoutingBase::omnet_chg_rte(const ManetAddress &dst, const ManetAddress &gtwy, const ManetAddress &netm, short int hops, bool del_entry, int index)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    IPv4Address desAddress(dst.getIPv4());
    if (!createInternalStore && routesVector)
    {
         delete routesVector;
         routesVector = NULL;
    }
    else if (createInternalStore && routesVector)
    {
         RouteMap::iterator it = routesVector->find(dst);
         if (it != routesVector->end())
             routesVector->erase(it);
         if (!del_entry)
         {
             routesVector->insert(std::make_pair<ManetAddress,ManetAddress>(dst, gtwy));
         }
    }
    if (mac_layer_)
        return;
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
                    opp_error("Aodv omnet_chg_rte can't delete route entry");
            }
            else
            {
                found = true;
                oldentry = e;
            }
        }
    }

#ifdef WITH_80211MESH
    if (locator && locator->isApIp(desAddress) && del_entry)
    {
        std::vector<IPv4Address> list;
        locator->getApListIp(desAddress, list);
        for (unsigned int i = 0; i < list.size(); i++)
        {
            for (int i = inet_rt->getNumRoutes(); i > 0; --i)
            {
                IPv4Route *e = inet_rt->getRoute(i - 1);
                if (list[i] == e->getDestination())
                {
                    if (!inet_rt->deleteRoute(e))
                        opp_error("Aodv omnet_chg_rte can't delete route entry");
                    else
                        break;
                }
            }
        }
    }
#endif
    if (del_entry)
        return;

    IPv4Address netmask(netm.getIPv4());
    IPv4Address gateway(gtwy.getIPv4());
    if (netm.isUnspecified())
        netmask = IPv4Address::ALLONES_ADDRESS;

    InterfaceEntry *ie = getInterfaceEntry(index);
    IPv4Route::RouteSource routeSource = usetManetLabelRouting ? IPv4Route::MANET : IPv4Route::MANET2;

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == ie
                && oldentry->getSource() == routeSource)
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

    if (usetManetLabelRouting)
        entry->setSource(IPv4Route::MANET);
    else
        entry->setSource(IPv4Route::MANET2);

        inet_rt->addRoute(entry);

#ifdef WITH_80211MESH
    if (locator && locator->isApIp(desAddress))
    {
        std::vector<IPv4Address> list;
        locator->getApListIp(desAddress, list);
        for (unsigned int i = 0; i < list.size(); i++)
        {
            IPv4Route *e = inet_rt->findBestMatchingRoute(list[i]);
            if (e && e->getDestination() == list[i])
            {
                if (e->getGateway() == gateway && e->getMetric() == hops && e->getInterface() == ie)
                    continue;
            }

            e = new IPv4Route();

            /// Destination
            e->setDestination(list[i]);
            /// Route mask
            e->setNetmask(netmask);
            /// Next hop
            e->setGateway(gateway);
            /// Metric ("cost" to reach the destination)
            e->setMetric(hops);
            /// Interface name and pointer

            e->setInterface(ie);

            /// Route type: Direct or Remote
            e->setType(entry->getType());
            /// Source of route, MANUAL by reading a file,
            /// routing protocol name otherwise
            e->setSource(entry->getSource());
            inet_rt->addRoute(e);
        }
    }
#endif
}


//
// Check if it exists in the ip4 routing table the address dst
// if it doesn't exist return ALLONES_ADDRESS
//
ManetAddress ManetRoutingBase::omnet_exist_rte(ManetAddress dst)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    IPv4Address desAddress(dst.getIPv4());
    const IPv4Route *e = NULL;
    if (mac_layer_)
        return ManetAddress::ZERO;
    for (int i=inet_rt->getNumRoutes(); i>0; --i)
    {
        e = inet_rt->getRoute(i-1);
        if (desAddress == e->getDestination())
            return ManetAddress(e->getGateway());
    }
    return ManetAddress(IPv4Address::ALLONES_ADDRESS);
}

//
// Erase all the entries in the routing table
//
void ManetRoutingBase::omnet_clean_rte()
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

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
// generic receiveChangeNotification, the protocols must implemet processLinkBreak and processPromiscuous only
//
void ManetRoutingBase::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method("Manet llf");
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");
    if (category == NF_LINK_BREAK)
    {
        if (details==NULL)
            return;
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(const_cast<cObject*>(details));
        if (frame)
        {
            cPacket *pktAux = frame->getEncapsulatedPacket();
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
                processLinkBreak(details);
        }
        else
        {
            Ieee80211ManagementFrame *frame = dynamic_cast<Ieee80211ManagementFrame *>(const_cast<cObject*>(details));
            if (frame)
                processLinkBreakManagement(details);
        }

    }
    else if (category == NF_LINK_PROMISCUOUS)
    {
        processPromiscuous(details);
    }
    else if (category == NF_LINK_FULL_PROMISCUOUS)
    {
        processFullPromiscuous(details);
    }
    else if(category == NF_L2_AP_DISASSOCIATED || category == NF_L2_AP_ASSOCIATED)
    {
        Ieee80211MgmtAP::NotificationInfoSta *infoSta = dynamic_cast<Ieee80211MgmtAP::NotificationInfoSta *>(const_cast<cObject*> (details));
        if (infoSta)
        {
            ManetAddress addr;
            if (!mac_layer_ && arp)
                addr = ManetAddress(arp->getInverseAddressResolution(infoSta->getStaAddress()));
            else
                addr = ManetAddress(infoSta->getStaAddress());
            // sanity check
            for (unsigned int i = 0; i< proxyAddress.size(); i++)
            {
                 if (proxyAddress[i].address == addr)
                 {
                     proxyAddress.erase(proxyAddress.begin()+i);
                     break;
                 }
            }
            if (category == NF_L2_AP_ASSOCIATED)
            {
                ManetProxyAddress p;
                p.address = addr;
                p.mask = mac_layer_ ? ManetAddress(MACAddress::BROADCAST_ADDRESS) : ManetAddress(IPv4Address::ALLONES_ADDRESS);
                proxyAddress.push_back(p);
            }
        }
    }
#ifdef WITH_80211MESH
    else if(category == NF_LOCATOR_ASSOC)
        processLocatorAssoc(details);
    else if(category == NF_LOCATOR_DISASSOC)
        processLocatorDisAssoc(details);
#endif
}

void ManetRoutingBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if (signalID == mobilityStateChangedSignal)
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
int ManetRoutingBase::getWlanInterfaceIndexByAddress(ManetAddress add)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    if (add.isUnspecified())
        return interfaceVector->front().index;

    for (unsigned int i=0; i<interfaceVector->size(); i++)
    {
        if (mac_layer_)
        {
            if ((*interfaceVector)[i].interfacePtr->getMacAddress() == add.getMAC())
                return (*interfaceVector)[i].index;
        }
        else
        {
            if ((*interfaceVector)[i].interfacePtr->ipv4Data()->getIPAddress() == add.getIPv4())
                return (*interfaceVector)[i].index;
        }
    }
    return -1;
}

//
// Get the interface with the same address that add
//
InterfaceEntry *ManetRoutingBase::getInterfaceWlanByAddress(ManetAddress add) const
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    if (add.isUnspecified())
        return interfaceVector->front().interfacePtr;

    for (unsigned int i=0; i<interfaceVector->size(); i++)
    {
        if (mac_layer_)
        {
            if ((*interfaceVector)[i].interfacePtr->getMacAddress() == add.getMAC())
                return (*interfaceVector)[i].interfacePtr;
        }
        else
        {
            if ((*interfaceVector)[i].interfacePtr->ipv4Data()->getIPAddress() == add.getIPv4())
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
        opp_error("Manet routing protocol is not register");

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
        opp_error("Manet routing protocol is not register");

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
        error("this node doesn't have activated the register position");
    return curPosition;
}

double ManetRoutingBase::getSpeed()
{
    if (!regPosition)
        error("this node doesn't have activated the register position");
    return curSpeed.length();
}

const Coord& ManetRoutingBase::getDirection()
{
    if (!regPosition)
        error("this node doesn't have activated the register position");
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


ManetAddress ManetRoutingBase::getNextHopInternal(const ManetAddress &dest)
{
    if (routesVector==NULL)
        return ManetAddress::ZERO;
    if (routesVector->empty())
        return ManetAddress::ZERO;
    RouteMap::iterator it = routesVector->find(dest);
    if (it!=routesVector->end())
        return it->second;
    return ManetAddress::ZERO;
}

bool ManetRoutingBase::setRoute(const ManetAddress & destination, const ManetAddress &nextHop, const int &ifaceIndex, const int &hops, const ManetAddress &mask)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

    /* Add route to kernel routing table ... */
    IPv4Address desAddress(destination.getIPv4());
    bool del_entry = (nextHop.isUnspecified());

    if (!createInternalStore && routesVector)
    {
         delete routesVector;
         routesVector = NULL;
    }
    else if (createInternalStore && routesVector)
    {
         //FIXME netmask not stored in internal routesVector, only stored in inet routing table
         // Is the netmask always ALLONES? If yes, do remove mask parameter...
         RouteMap::iterator it = routesVector->find(destination);
         if (it != routesVector->end())
             routesVector->erase(it);
         if (!del_entry)
         {
             routesVector->insert(std::make_pair<ManetAddress,ManetAddress>(destination, nextHop));
         }
    }

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
                    opp_error("ManetRoutingBase::setRoute can't delete route entry");
            }
            else
            {
                found = true;
                oldentry = e;
            }
        }
    }

#ifdef WITH_80211MESH
    if (locator && locator->isApIp(desAddress) && del_entry)
    {
        std::vector<IPv4Address> list;
        locator->getApListIp(desAddress, list);
        for (unsigned int i = 0; i < list.size(); i++)
        {
            for (int i = inet_rt->getNumRoutes(); i > 0; --i)
            {
                IPv4Route *e = inet_rt->getRoute(i - 1);
                if (list[i] == e->getDestination())
                {
                    if (!inet_rt->deleteRoute(e))
                        opp_error("Aodv omnet_chg_rte can't delete route entry");
                    else
                        break;
                }
            }
        }
    }
#endif
    if (del_entry)
        return true;

    IPv4Address netmask(mask.getIPv4());
    IPv4Address gateway(nextHop.getIPv4());
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
                && oldentry->getSource() == IPv4Route::MANUAL)
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
    IPv4Route::RouteSource routeSource = usetManetLabelRouting ? IPv4Route::MANET : IPv4Route::MANET2;
    entry->setSource(routeSource);

    inet_rt->addRoute(entry);

#ifdef WITH_80211MESH
    if (locator && locator->isApIp(desAddress))
    {
        std::vector<IPv4Address> list;
        locator->getApListIp(desAddress, list);
        for (unsigned int i = 0; i < list.size(); i++)
        {
            IPv4Route *e = inet_rt->findBestMatchingRoute(list[i]);
            if (e && e->getDestination() == list[i])
            {
                if (e->getGateway() == gateway && e->getMetric() == hops && e->getInterface() == ie)
                    continue;
            }

            e = new IPv4Route();

            /// Destination
            e->setDestination(list[i]);
            /// Route mask
            e->setNetmask(netmask);
            /// Next hop
            e->setGateway(gateway);
            /// Metric ("cost" to reach the destination)
            e->setMetric(hops);
            /// Interface name and pointer

            e->setInterface(ie);

            /// Route type: Direct or Remote
            e->setType(entry->getType());
            /// Source of route, MANUAL by reading a file,
            /// routing protocol name otherwise
            e->setSource(entry->getSource());
            inet_rt->addRoute(e);
        }
    }
#endif
    return true;

}

bool ManetRoutingBase::setRoute(const ManetAddress & destination, const ManetAddress &nextHop, const char *ifaceName, const int &hops, const ManetAddress &mask)
{
    if (!isRegistered)
        opp_error("Manet routing protocol is not register");

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
        EV << "won't send ICMP error messages for multicast message " << datagram << endl;
        delete pkt;
        return;
    }
    // check source address
    if (datagram->getSrcAddress().isUnspecified() && par("setICMPSourceAddress"))
        datagram->setSrcAddress(inet_ift->getInterface(0)->ipv4Data()->getIPAddress());
    EV << "issuing ICMP Destination Unreachable for packets waiting in queue for failed route discovery.\n";
    icmpModule->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
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

void ManetRoutingBase::addInAddressGroup(const ManetAddress& addr, int group)
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

bool ManetRoutingBase::delInAddressGroup(const ManetAddress& addr, int group)
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
        for (AddressGroup::iterator it = addressGroupVector[group].begin(); it!=addressGroupVector[group].end(); it++)
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

bool ManetRoutingBase::findInAddressGroup(const ManetAddress& addr, int group)
{
    if ((int)addressGroupVector.size()<=group)
        return false;
    if (addressGroupVector[group].count(addr)>0)
        return true;
    return false;
}

bool ManetRoutingBase::findAddressAndGroup(const ManetAddress& addr, int &group)
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

bool ManetRoutingBase::getAddressGroup(std::vector<ManetAddress> &addressGroup, int group)
{
    if ((int)addressGroupVector.size()<=group)
        return false;
    addressGroup.clear();
    for (AddressGroup::iterator it=addressGroupVector[group].begin(); it!=addressGroupVector[group].end(); it++)
        addressGroup.push_back(*it);
    return true;
}


bool ManetRoutingBase::isAddressInProxyList(const ManetAddress & addr)
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

void ManetRoutingBase::setAddressInProxyList(const ManetAddress & addr,const ManetAddress & mask)
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

bool ManetRoutingBase::getAddressInProxyList(int i,ManetAddress &addr, ManetAddress &mask)
{
    if (i< 0 || i >= (int)proxyAddress.size())
        return false;
    addr = proxyAddress[i].address;
    mask = proxyAddress[i].mask;
    return true;
}


bool ManetRoutingBase::addressIsForUs(const ManetAddress &addr) const
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

bool ManetRoutingBase::getAp(const ManetAddress &destination, ManetAddress& accesPointAddr) const
{
#ifdef WITH_80211MESH
    if (locator == NULL)
        return false;
    if (isInMacLayer())
    {
        MACAddress macAddr = locator->getLocatorMacToMac(MACAddress(destination.getLo()));
        if (macAddr.isUnspecified())
            return false;
        accesPointAddr = macAddr.getInt();
    }
    else
    {
        IPv4Address ipAddr = locator->getLocatorIpToIp(IPv4Address(destination.getLo()));
        if (ipAddr.isUnspecified())
            return false;
        accesPointAddr = ipAddr.getInt();
    }
    return true;
#else
    return false;
#endif
}

void ManetRoutingBase::getApList(const MACAddress & dest,std::vector<MACAddress>& list)
{
    list.clear();
#ifdef WITH_80211MESH
    if (locator)
    {
        MACAddress ap = locator->getLocatorMacToMac(dest);
        if (!ap.isUnspecified())
            locator->getApList(ap,list);
        else
            locator->getApList(dest,list);
    }
#endif
    list.push_back(dest);
}

void ManetRoutingBase::getApListIp(const IPv4Address &dest,std::vector<IPv4Address>& list)
{
    list.clear();
#ifdef WITH_80211MESH
    if (locator)
    {
        IPv4Address ap = locator->getLocatorIpToIp(dest);
        if (!ap.isUnspecified())
            locator->getApListIp(ap,list);
        else
            locator->getApListIp(dest,list);
    }
#endif
    list.push_back(dest);
}

void ManetRoutingBase::getListRelatedAp(const ManetAddress & add, std::vector<ManetAddress>& list)
{
    if (mac_layer_)
    {
        std::vector<MACAddress> listAux;
        getApList(add.getMAC(), listAux);
        list.clear();
        for (unsigned int i = 0; i < listAux.size(); i++)
        {
            list.push_back(ManetAddress(listAux[i]));
        }
    }
    else
    {
        std::vector<IPv4Address> listAux;
        getApListIp(add.getIPv4(), listAux);
        list.clear();
        for (unsigned int i = 0; i < listAux.size(); i++)
        {
            list.push_back(ManetAddress(listAux[i]));
        }
    }
}

bool ManetRoutingBase::isAp() const
{
#ifdef WITH_80211MESH
    if (!locator)
        return false;
    if (mac_layer_)
        return locator->isThisAp();
    else
        return locator->isThisApIp();
#else
    return false;
#endif
}

bool ManetRoutingBase::getRouteFromGlobal(const ManetAddress &src, const ManetAddress &dest, std::vector<ManetAddress> &route)
{
    if (!createInternalStore || globalRouteMap == NULL)
        return false;
    ManetAddress next = src;
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
}

