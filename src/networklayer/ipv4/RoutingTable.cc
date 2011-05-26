//
// Copyright (C) 2004-2006 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


//  Cleanup and rewrite: Andras Varga, 2004

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>

#include "RoutingTable.h"
#include "RoutingTableParser.h"
#include "IPv4Route.h"
#include "IPv4InterfaceData.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "NotifierConsts.h"


Define_Module(RoutingTable);


std::ostream& operator<<(std::ostream& os, const IPv4Route& e)
{
    os << e.info();
    return os;
};


RoutingTable::RoutingTable()
{
 // DSDV
    timetolive_routing_entry = timetolive_routing_entry.getMaxTime();
}

RoutingTable::~RoutingTable()
{
    for (unsigned int i=0; i<routes.size(); i++)
        delete routes[i];
    for (unsigned int i=0; i<multicastRoutes.size(); i++)
        delete multicastRoutes[i];
    while (!inputRules.empty())
    {
        delete inputRules.back();
        inputRules.pop_back();
    }
    while (!outputRules.empty())
    {
        delete outputRules.back();
        outputRules.pop_back();
    }
}

void RoutingTable::initialize(int stage)
{
    if (stage==0)
    {
        // get a pointer to the NotificationBoard module and IInterfaceTable
        nb = NotificationBoardAccess().get();
        ift = InterfaceTableAccess().get();

        IPForward = par("IPForward").boolValue();

        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_INTERFACE_IPv4CONFIG_CHANGED);

        WATCH_PTRVECTOR(routes);
        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(IPForward);
        WATCH(routerId);
    }
    else if (stage==1)
    {
        // L2 modules register themselves in stage 0, so we can only configure
        // the interfaces in stage 1.
        const char *filename = par("routingFile");

        // At this point, all L2 modules have registered themselves (added their
        // interface entries). Create the per-interface IPv4 data structures.
        IInterfaceTable *interfaceTable = InterfaceTableAccess().get();
        for (int i=0; i<interfaceTable->getNumInterfaces(); ++i)
            configureInterfaceForIPv4(interfaceTable->getInterface(i));
        configureLoopbackForIPv4();

        // read routing table file (and interface configuration)
        RoutingTableParser parser(ift, this);
        if (*filename && parser.readRoutingTableFromFile(filename)==-1)
            error("Error reading routing table file %s", filename);

        // set routerId if param is not "" (==no routerId) or "auto" (in which case we'll
        // do it later in stage 3, after network configurators configured the interfaces)
        const char *routerIdStr = par("routerId").stringValue();
        if (strcmp(routerIdStr, "") && strcmp(routerIdStr, "auto"))
            routerId = IPv4Address(routerIdStr);
    }
    else if (stage==3)
    {
        // routerID selection must be after stage==2 when network autoconfiguration
        // assigns interface addresses
        configureRouterId();

        // we don't use notifications during initialize(), so we do it manually.
        // Should be in stage=3 because autoconfigurator runs in stage=2.
        updateNetmaskRoutes();

        //printRoutingTable();
    }
}

void RoutingTable::configureRouterId()
{
    if (routerId.isUnspecified())  // not yet configured
    {
        const char *routerIdStr = par("routerId").stringValue();
        if (!strcmp(routerIdStr, "auto"))  // non-"auto" cases already handled in stage 1
        {
            // choose highest interface address as routerId
            for (int i=0; i<ift->getNumInterfaces(); ++i)
            {
                InterfaceEntry *ie = ift->getInterface(i);
                if (!ie->isLoopback() && ie->ipv4Data()->getIPAddress().getInt() > routerId.getInt())
                    routerId = ie->ipv4Data()->getIPAddress();
            }
        }
    }
    else // already configured
    {
        // if there is no interface with routerId yet, assign it to the loopback address;
        // TODO find out if this is a good practice, in which situations it is useful etc.
        if (getInterfaceByAddress(routerId)==NULL)
        {
            InterfaceEntry *lo0 = ift->getFirstLoopbackInterface();
            lo0->ipv4Data()->setIPAddress(routerId);
            lo0->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);
        }
    }
}

void RoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    if (routerId.isUnspecified())
        sprintf(buf, "%d+%d routes", (int)routes.size(), (int)multicastRoutes.size());
    else
        sprintf(buf, "routerId: %s\n%d+%d routes", routerId.str().c_str(), (int)routes.size(), (int)multicastRoutes.size());
    getDisplayString().setTagArg("t",0,buf);
}

void RoutingTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError(this, "This module doesn't process messages");
}

void RoutingTable::receiveChangeNotification(int category, const cPolymorphic *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category==NF_INTERFACE_CREATED)
    {
        // add netmask route for the new interface
        updateNetmaskRoutes();
    }
    else if (category==NF_INTERFACE_DELETED)
    {
        // remove all routes that point to that interface
        InterfaceEntry *entry = check_and_cast<InterfaceEntry*>(details);
        deleteInterfaceRoutes(entry);
    }
    else if (category==NF_INTERFACE_STATE_CHANGED)
    {
        invalidateCache();
    }
    else if (category==NF_INTERFACE_CONFIG_CHANGED)
    {
        invalidateCache();
    }
    else if (category==NF_INTERFACE_IPv4CONFIG_CHANGED)
    {
        // if anything IPv4-related changes in the interfaces, interface netmask
        // based routes have to be re-built.
        updateNetmaskRoutes();
    }
}

void RoutingTable::deleteInterfaceRoutes(InterfaceEntry *entry)
{
    RouteVector::iterator it = routes.begin();
    while (it != routes.end())
    {
        IPv4Route *route = *it;
        if (route->getInterface() == entry)
        {
            deleteRoute(route);
            it = routes.begin();  // iterator became invalid -- start over
        }
        else
        {
            ++it;
        }
    }
}

void RoutingTable::invalidateCache()
{
    routingCache.clear();
    localAddresses.clear();
}

void RoutingTable::printRoutingTable() const
{
    EV << "-- Routing table --\n";
    ev.printf("%-16s %-16s %-16s %-3s %s\n",
              "Destination", "Gateway", "Netmask", "Iface");

    for (int i=0; i<getNumRoutes(); i++)
        EV << getRoute(i)->detailedInfo() << "\n";
    EV << "\n";
}

std::vector<IPv4Address> RoutingTable::gatherAddresses() const
{
    std::vector<IPv4Address> addressvector;

    for (int i=0; i<ift->getNumInterfaces(); ++i)
        addressvector.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
    return addressvector;
}

//---

void RoutingTable::configureInterfaceForIPv4(InterfaceEntry *ie)
{
    IPv4InterfaceData *d = new IPv4InterfaceData();
    ie->setIPv4Data(d);

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    d->setMetric((int)ceil(2e9/ie->getDatarate())); // use OSPF cost as default
}

InterfaceEntry *RoutingTable::getInterfaceByAddress(const IPv4Address& addr) const
{
    Enter_Method("getInterfaceByAddress(%u.%u.%u.%u)", addr.getDByte(0), addr.getDByte(1), addr.getDByte(2), addr.getDByte(3)); // note: str().c_str() too slow here

    if (addr.isUnspecified())
        return NULL;
    for (int i=0; i<ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->getIPAddress()==addr)
            return ie;
    }
    return NULL;
}


void RoutingTable::configureLoopbackForIPv4()
{
    InterfaceEntry *ie = ift->getFirstLoopbackInterface();

    // add IPv4 info. Set 127.0.0.1/8 as address by default --
    // we may reconfigure later it to be the routerId
    IPv4InterfaceData *d = new IPv4InterfaceData();
    d->setIPAddress(IPv4Address::LOOPBACK_ADDRESS);
    d->setNetmask(IPv4Address::LOOPBACK_NETMASK);
    d->setMetric(1);
    ie->setIPv4Data(d);
}

//---

bool RoutingTable::isLocalAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    if (localAddresses.empty())
    {
        // collect interface addresses if not yet done
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            IPv4Address interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            localAddresses.insert(interfaceAddr);
        }
    }

    AddressSet::iterator it = localAddresses.find(dest);
    return it!=localAddresses.end();
}

// JcM add: check if the dest addr is local network broadcast
bool RoutingTable::isLocalBroadcastAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalBroadcastAddress(%x)", dest.getInt()); // note: str().c_str() too slow here

    if (localBroadcastAddresses.empty())
    {
        // collect interface addresses if not yet done
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            IPv4Address interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            IPv4Address broadcastAddr = interfaceAddr.getBroadcastAddress(ift->getInterface(i)->ipv4Data()->getNetmask());
            if (!broadcastAddr.isUnspecified())
            {
                 localBroadcastAddresses.insert(broadcastAddr);
            }
        }
    }

    AddressSet::iterator it = localBroadcastAddresses.find(dest);
    return it!=localBroadcastAddresses.end();
}

bool RoutingTable::isLocalMulticastAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalMulticastAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->isMemberOfMulticastGroup(dest))
            return true;
    }
    return false;
}

void RoutingTable::dsdvTestAndDelete()
{
     if (timetolive_routing_entry==timetolive_routing_entry.getMaxTime())
           return;
     for (RouteVector::iterator i=routes.begin(); i!=routes.end(); ++i)
     {

           IPv4Route *e = *i;
           if (this->isLocalAddress(e->getHost()))
                     continue;
           if (((e->getHost()).str() != "*") && ((e->getHost()).str() != "<unspec>") && ((e->getHost()).str() != "127.0.0.1") && (simTime()-(e->getInstallTime()))>timetolive_routing_entry){
                //EV << "Routes ends at" << routes.end() <<"\n";
                deleteRoute(e);
                //EV << "After deleting Routes ends at" << routes.end() <<"\n";
                EV << "Deleting entry ip=" << e->getHost().str() <<"\n";
                i--;
           }
      }

}

const bool RoutingTable::testValidity(const IPv4Route *entry) const
{
     if (timetolive_routing_entry==timetolive_routing_entry.getMaxTime())
           return true;
     if (this->isLocalAddress(entry->getHost()))
           return true;
     if (((entry->getHost()).str() != "*") && ((entry->getHost()).str() != "<unspec>") && ((entry->getHost()).str() != "127.0.0.1") && (simTime()-(entry->getInstallTime()))>timetolive_routing_entry){
                return false;
      }
      return true;

}

const IPv4Route *RoutingTable::findBestMatchingRoute(const IPv4Address& dest) const
{
    Enter_Method("findBestMatchingRoute(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    RoutingCache::iterator it = routingCache.find(dest);
    if (it != routingCache.end())
    {
        if (it->second==NULL)
        {
              routingCache.clear();
              localAddresses.clear();
        }
        else if (testValidity(it->second))
        {
            if (it->second->getSource()==IPv4Route::MANET)
            {
                if (IPv4Address::maskedAddrAreEqual(dest, it->second->getHost(), IPv4Address::ALLONES_ADDRESS))
        return it->second;
            }
            else
                return it->second;
        }
    }
    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    const IPv4Route *bestRoute = NULL;
    uint32 longestNetmask = 0;
    for (RouteVector::const_iterator i=routes.begin(); i!=routes.end(); ++i)
    {
        IPv4Route *e = *i;
        if (testValidity(e))
        {
            if (IPv4Address::maskedAddrAreEqual(dest, e->getHost(), e->getNetmask()) &&  // match
                (!bestRoute || e->getNetmask().getInt() > longestNetmask))  // longest so far
            {
                bestRoute = e;
                longestNetmask = e->getNetmask().getInt();
            }
    }
    }

    if (bestRoute && bestRoute->getSource()==IPv4Route::MANET && bestRoute->getHost()!=dest)
    {
        bestRoute=NULL;
        /* in this case we must find the mask must be 255.255.255.255 route */
        for (RouteVector::const_iterator i=routes.begin(); i!=routes.end(); ++i)
        {
           IPv4Route *e = *i;
           if (testValidity(e))
           {
              if (IPv4Address::maskedAddrAreEqual(dest, e->getHost(), IPv4Address::ALLONES_ADDRESS) &&  // match
               (!bestRoute || e->getNetmask().getInt()>longestNetmask))  // longest so far
              {
                 bestRoute = e;
                 longestNetmask = e->getNetmask().getInt();
              }
           }
        }
    }

    routingCache[dest] = bestRoute;
    return bestRoute;
}

InterfaceEntry *RoutingTable::getInterfaceForDestAddr(const IPv4Address& dest) const
{
    Enter_Method("getInterfaceForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getInterface() : NULL;
}

IPv4Address RoutingTable::getGatewayForDestAddr(const IPv4Address& dest) const
{
    Enter_Method("getGatewayForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getGateway() : IPv4Address();
}


MulticastRoutes RoutingTable::getMulticastRoutesFor(const IPv4Address& dest) const
{
    Enter_Method("getMulticastRoutesFor(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here here

    MulticastRoutes res;
    res.reserve(16);
    for (RouteVector::const_iterator i=multicastRoutes.begin(); i!=multicastRoutes.end(); ++i)
    {
        const IPv4Route *e = *i;
        if (IPv4Address::maskedAddrAreEqual(dest, e->getHost(), e->getNetmask()))
        {
            MulticastRoute r;
            r.interf = e->getInterface();
            r.gateway = e->getGateway();
            res.push_back(r);
        }
    }
    return res;
}


int RoutingTable::getNumRoutes() const
{
    return routes.size()+multicastRoutes.size();
}

const IPv4Route *RoutingTable::getRoute(int k) const
{
    if (k < (int)routes.size())
        return routes[k];
    k -= routes.size();
    if (k < (int)multicastRoutes.size())
        return multicastRoutes[k];
    return NULL;
}

const IPv4Route *RoutingTable::getDefaultRoute() const
{
    int n = (int)routes.size();
    for (int i=0; i<n; i++)
        if (routes[i]->getNetmask().isUnspecified())
            return routes[i];
    return NULL;
}

const IPv4Route *RoutingTable::findRoute(const IPv4Address& target, const IPv4Address& netmask,
    const IPv4Address& gw, int metric, const char *dev) const
{
    int n = getNumRoutes();
    for (int i=0; i<n; i++)
        if (routeMatches(getRoute(i), target, netmask, gw, metric, dev))
            return getRoute(i);
    return NULL;
}

void RoutingTable::addRoute(const IPv4Route *entry)
{
    Enter_Method("addRoute(...)");

    // check for null address and default route
    if (entry->getHost().isUnspecified() != entry->getNetmask().isUnspecified())
        error("addRoute(): to add a default route, set both host and netmask to zero");

    if (entry->getHost().doAnd(entry->getNetmask().isUnspecified()).getInt() != 0)
        error("addRoute(): suspicious route: host %s has 1-bits outside netmask %s",
              entry->getHost().str().c_str(), entry->getNetmask().str().c_str());

    // check that the interface exists
    if (!entry->getInterface())
        error("addRoute(): interface cannot be NULL");

    // if this is a default route, remove old default route (we're replacing it)
    if (entry->getNetmask().isUnspecified() && getDefaultRoute()!=NULL)
        deleteRoute(getDefaultRoute());

    // add to tables
    if (!entry->getHost().isMulticast())
        routes.push_back(const_cast<IPv4Route*>(entry));
    else
        multicastRoutes.push_back(const_cast<IPv4Route*>(entry));

    invalidateCache();
    updateDisplayString();

    nb->fireChangeNotification(NF_IPv4_ROUTE_ADDED, entry);
}


bool RoutingTable::deleteRoute(const IPv4Route *entry)
{
    Enter_Method("deleteRoute(...)");

    RouteVector::iterator i = std::find(routes.begin(), routes.end(), entry);
    if (i!=routes.end())
    {
        nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, entry); // rather: going to be deleted
        routes.erase(i);
        delete entry;
        invalidateCache();
        updateDisplayString();
        return true;
    }
    i = std::find(multicastRoutes.begin(), multicastRoutes.end(), entry);
    if (i!=multicastRoutes.end())
    {
        nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, entry); // rather: going to be deleted
        multicastRoutes.erase(i);
        delete entry;
        invalidateCache();
        updateDisplayString();
        return true;
    }
    return false;
}


bool RoutingTable::routeMatches(const IPv4Route *entry,
    const IPv4Address& target, const IPv4Address& nmask,
    const IPv4Address& gw, int metric, const char *dev) const
{
    if (!target.isUnspecified() && !target.equals(entry->getHost()))
        return false;
    if (!nmask.isUnspecified() && !nmask.equals(entry->getNetmask()))
        return false;
    if (!gw.isUnspecified() && !gw.equals(entry->getGateway()))
        return false;
    if (metric && metric!=entry->getMetric())
        return false;
    if (dev && strcmp(dev, entry->getInterfaceName()))
        return false;

    return true;
}

void RoutingTable::updateNetmaskRoutes()
{
    // first, delete all routes with src=IFACENETMASK
    for (unsigned int k=0; k<routes.size(); k++)
        if (routes[k]->getSource()==IPv4Route::IFACENETMASK)
            routes.erase(routes.begin()+(k--));  // '--' is necessary because indices shift down

    // then re-add them, according to actual interface configuration
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->getNetmask()!=IPv4Address::ALLONES_ADDRESS)
        {
            IPv4Route *route = new IPv4Route();
            route->setType(IPv4Route::DIRECT);
            route->setSource(IPv4Route::IFACENETMASK);
            route->setHost(ie->ipv4Data()->getIPAddress().doAnd(ie->ipv4Data()->getNetmask()));
            route->setNetmask(ie->ipv4Data()->getNetmask());
            route->setGateway(IPv4Address());
            route->setMetric(ie->ipv4Data()->getMetric());
            route->setInterface(ie);
            routes.push_back(route);
        }
    }

    invalidateCache();
    updateDisplayString();
}



void RoutingTable::addRule(bool output,IPv4RouteRule *entry)
{
// first, find the rule if exist
    delRule(entry);
    if (output)
    {
        outputRules.push_back(entry);
    }
    else
    {
        inputRules.push_back(entry);
    }
}

void RoutingTable::delRule(IPv4RouteRule *entry)
{
    for (unsigned int i;i<outputRules.size();i++)
    {
       if (outputRules[i]==entry)
           outputRules.erase(outputRules.begin()+i);

    }
    for (unsigned int i;i<inputRules.size();i++)
    {
        if (inputRules[i]==entry)
            inputRules.erase(inputRules.begin()+i);

    }
}

const IPv4RouteRule * RoutingTable::getRule(bool output,int index) const
{
    if (output)
    {
        if (index < (int)outputRules.size())
            return outputRules[index];
        else
            return NULL;
    }
    else
    {
        if (index < (int)inputRules.size())
            return inputRules[index];
        else
            return NULL;
    }
}

int RoutingTable::getNumRules(bool output)
{
    if (output)
        return outputRules.size();
    else
        return inputRules.size();
}

const IPv4RouteRule * RoutingTable::findRule(bool output,int prot,int sPort,const IPv4Address &srcAddr,int dPort,const IPv4Address &destAddr,const InterfaceEntry *iface) const
{
	std::vector<IPv4RouteRule *>::const_iterator it;
	std::vector<IPv4RouteRule *>::const_iterator endIt;
    if (output)
    {
    	it = outputRules.begin();
    	endIt = outputRules.end();
    }
    else
    {
    	it = inputRules.begin();
    	endIt = inputRules.end();
    }

    while (it!=endIt)
    {
       IPv4RouteRule *e = (*it);
       if (!srcAddr.isUnspecified() && !e->getSrcAddress().isUnspecified())
       {
           if (!IPv4Address::maskedAddrAreEqual(srcAddr,e->getSrcAddress(),e->getSrcNetmask()))
           {
               it++;
               continue;
           }
       }
       if (!destAddr.isUnspecified() && !e->getDestAddress().isUnspecified())
       {
           if (!IPv4Address::maskedAddrAreEqual(destAddr,e->getDestAddress(),e->getDestNetmask()))
           {
               it++;
               continue;
           }
       }
       if ((prot!=IP_PROT_NONE) && (e->getProtocol()!=IP_PROT_NONE) && (prot!=e->getProtocol()))
       {
           it++;
           continue;
       }
       if ((sPort!=-1) && (e->getSrcPort()!=-1) && (sPort!=e->getSrcPort()))
       {
           it++;
           continue;
       }
       if ((dPort!=-1) && (e->getDestPort()!=-1) && (sPort!=e->getDestPort()))
       {
           it++;
           continue;
       }
       if ((iface!=NULL) && (e->getInterface()!=NULL) && (iface!=e->getInterface()))
       {
           it++;
           continue;
       }
       // found valid src address, dest address src port and dest port
       return e;
    }
    return NULL;
}
