//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <strstream>

#include "RoutingTable.h"
#include "RoutingTableParser.h"


//#define PRINTF  printf
#define PRINTF  ev.printf



InterfaceEntry::InterfaceEntry()
{
    mtu = 0;
    metric = 0;

    broadcast = false;
    multicast = false;
    pointToPoint= false;
    loopback = false;

    multicastGroupCtr = 0;
    multicastGroup = NULL;
}

void InterfaceEntry::info(char *buf)
{
    std::ostrstream out(buf, MAX_OBJECTINFO);
    out << "name:" << (!name.empty() ? name : "*");
    out << "  encap:" << (!encap.empty() ? encap : "*");
    out << "  HWaddr:" << (!hwAddrStr.empty() ? hwAddrStr : "*");
}

opp_string& InterfaceEntry::detailedInfo(opp_string& buf)
{
    std::ostrstream out;
    out << "name:" << (!name.empty() ? name : "*");
    out << "\tencap:" << (!encap.empty() ? encap : "*");
    out << "\tHWaddr:" << (!hwAddrStr.empty() ? hwAddrStr : "*");
    out << "\n";

    if (!inetAddr.isNull())
        out << "inet addr:" << inetAddr.getString();
    if (!bcastAddr.isNull())
        out << "\tBcast: " << bcastAddr.getString();
    if (!mask.isNull())
        out << "\tMask: " << mask.getString();
    out << "\n";

    out << "MTU: " << mtu << " \tMetric: " << metric << "\n";

    out << "Groups:";
    for (int j=0; j<multicastGroupCtr; j++)
        if (!multicastGroup[j].isNull())
            out << "  " << multicastGroup[j].getString();
    out << "\n";

    if (broadcast) out << "BROADCAST ";
    if (multicast) out << "MULTICAST ";
    if (pointToPoint) out << "POINTTOPOINT ";
    if (loopback) out << "LOOPBACK ";
    out << "\n";

    buf = out.str();
    return buf;
}

void InterfaceEntry::print()
{
    opp_string buf;
    PRINTF("%s", detailedInfo(buf).c_str());
}

//================================================================

RoutingEntry::RoutingEntry()
{
    ref = 0;

    interfaceNo = -1;

    metric = 0;
    type = DIRECT;
    source = MANUAL;

    age = -1;
    routeInfo = NULL;
}

void RoutingEntry::info(char *buf)
{
    std::ostrstream out(buf, MAX_OBJECTINFO);
    out << (!host.isNull() ? host.getString() : "*") << "  ";
    out << (!gateway.isNull() ? gateway.getString() : "*") << "  ";
    out << (!netmask.isNull() ? netmask.getString() : "*") << "  ";
    out << ref << "  ";
    out << (!interfaceName.empty() ? interfaceName.c_str() : "*") << "  ";
    out << (type==DIRECT ? "DIRECT" : "REMOTE") << "  ";
    out << (routeInfo ? routeInfo : NULL);
}

opp_string& RoutingEntry::detailedInfo(opp_string& buf)
{
    return buf;
}

void RoutingEntry::print()
{
    opp_string buf;
    PRINTF("%s", detailedInfo(buf).c_str());
}

bool RoutingEntry::correspondTo(const IPAddress& target,
                                const IPAddress& nmask,
                                const IPAddress& gw,
                                int metric,
                                const char *dev)
{
    if (!target.isNull() && !target.isEqualTo(host))
        return false;
    if (!nmask.isNull() && !nmask.isEqualTo(netmask))
        return false;
    if (!gw.isNull() && !gw.isEqualTo(gateway))
        return false;
    if (metric && metric!=metric)
        return false;
    if (dev && strcmp(dev, interfaceName.c_str()))
        return false;

    return true;
}

//==============================================================


Define_Module( RoutingTable );


void RoutingTable::initialize()
{
    IPForward = par("IPForward").boolValue();
    const char *filename = par("routingTableFileName");

    numIntrfaces = 0;
    intrface = new InterfaceEntry *[MAX_INTERFACE_NO];

    route = new cArray("routes");
    mcRoute = new cArray("multicast routes");

    defaultRoute = NULL;
    loopbackInterface = NULL;

    // Read routing table file
    // Abort simulation if no routing file could be read
    RoutingTableParser parser(this);
    if (parser.readRoutingTableFromFile(filename) == -1)
        error("Error reading routing table file %s", filename);

    //printIfconfig();
    //printRoutingTable();
}


void RoutingTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}


// --------------
//  Print tables
// --------------

void RoutingTable::printIfconfig()
{
    InterfaceEntry *e;

    PRINTF("\n---- IF config ----");
    for (int i = -1; i < numIntrfaces; i++) {
        // trick to add loopback interface in front
        e = (i == -1) ? loopbackInterface : (InterfaceEntry*)(intrface[i]);
        PRINTF("\n%d\t", i);
        e->print();
    }
    PRINTF("\n");
}

void RoutingTable::printRoutingTable()
{
    int i;

    PRINTF("\n-- Routing table --");
    PRINTF("\n%-16s %-16s %-16s %-3s %s",
           "Destination", "Gateway", "Genmask", "Ref", "Iface");
    for (i = 0; i < route->items(); i++) {
        if (route->get(i)) {
            ((RoutingEntry*)route->get(i))->print();
        }
    }

    if (defaultRoute) {
        defaultRoute->print();
    }

    PRINTF("\n");

    for (i = 0; i < mcRoute->items(); i++) {
        if (mcRoute->get(i)) {
            ((RoutingEntry*)mcRoute->get(i))->print();
        }
    }
    PRINTF("\n");
}



// -----------------------------------
//  Access of interface/routing table
// -----------------------------------

bool RoutingTable::localDeliver(const IPAddress& dest)
{
    Enter_Method("localDeliver(%s) y/n", dest.getString());

    for (int i = 0; i < numIntrfaces; i++) {
        if (dest.isEqualTo(intrface[i]->inetAddr)) {
            return true;
        }
    }

    if (dest.isEqualTo(loopbackInterface->inetAddr)) {
        ev << "LOCAL LOOPBACK INVOKED\n";
        return true;
    }

    return false;
}

bool RoutingTable::multicastLocalDeliver(const IPAddress& dest)
{
    Enter_Method("multicastLocalDeliver(%s) y/n", dest.getString());

    for (int i = 0; i < numIntrfaces; i++) {
        for (int j = 0; j < intrface[i]->multicastGroupCtr; j++) {
            if (dest.isEqualTo(intrface[i]->multicastGroup[j])) {
                return true;
            }
        }
    }

    return false;
}


int RoutingTable::outputPortNo(const IPAddress& dest)
{
    Enter_Method("outputPortNo(%s)=?", dest.getString());

    RoutingEntry *e;
    for (int i = 0; i < route->items(); i++) {
        if (route->get(i)) {
            // The destination in the datagram should /always/ be
            // compared against the destination-address of the interface,
            // and the gateway will be used on Layer 2.
            // Theoretically, there should be a differentiation between
            // Host and Network (rather than between Host and Gateway),
            // but none is made here.
            // -- Jochen Reber, 27.10.00
            e = (RoutingEntry*)route->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
                return findInterfaceByName(e->interfaceName.c_str());
            }
        }
    }

    // Is it gateway here?
    if (defaultRoute) {
        return findInterfaceByName(defaultRoute->interfaceName.c_str());
    }

    return -1;
}


int RoutingTable::multicastOutputPortNo(const IPAddress& dest, int index)
{
    Enter_Method("multicastOutputPortNo(%s, %d)=?", dest.getString(), index);

    if (index >= mcRoute->items())
        opp_error("wrong multicast port index");

    int mcDestCtr = 0;
    int i = -1;
    RoutingEntry *e = NULL;
    while (mcDestCtr < index + 1) {
        i++;

        if (i == mcRoute->items())
            opp_error("wrong multicast port index");

        if (mcRoute->get(i)) {
            e = (RoutingEntry*)mcRoute->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask))
                mcDestCtr++;
        }
    }

    // FIXME what if e==NULL?
    return findInterfaceByName(e->interfaceName.c_str());
}

int RoutingTable::numMulticastDestinations(const IPAddress& dest)
{
    int mcDestCtr = 0;
    RoutingEntry *e;

    for (int i = 0; i < mcRoute->items(); i++) {
        if (mcRoute->get(i)) {
            e = (RoutingEntry*)mcRoute->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
                mcDestCtr++;
            }
        }
    }
    return mcDestCtr;
}


InterfaceEntry *RoutingTable::getInterfaceByIndex(int index)
{
    if (index<0 || index>=numIntrfaces)
        opp_error("getInterfaceByIndex(): nonexistent interface %d", index);

    InterfaceEntry *interf = intrface[index];
    return interf;
}


int RoutingTable::findInterfaceByName(const char *name)
{
    Enter_Method("findInterfaceByName(%s)=?", name);
    if (!name)
        return -1;

    for (int i=0; i<numIntrfaces; i++)
        if (!strcmp(name, intrface[i]->name.c_str()))
            return i;

    // loopback interface has no number
    return -1;
}


int RoutingTable::findInterfaceByAddress(const IPAddress& addr)
{
    Enter_Method("findInterfaceByAddress(%s)=?", addr.getString());
    if (addr.isNull())
        return -1;

    for (int i = 0; i < numIntrfaces; i++)
        if (IPAddress::maskedAddrAreEqual(addr,intrface[i]->inetAddr,intrface[i]->mask))
            return i;

// BCH Andras -- code from UTS MPLS model
    // FIXME this seems to be a bloody hack. Why do we return 0 when intrface[0]
    // is obviously NOT what we were looking for???  --Andras

    // Add number for loopback
    if((loopbackInterface->inetAddr.getInt()) == (addr.getInt()))
        return 0;
// ECH
    return -1;
}


IPAddress RoutingTable::nextGatewayAddress(const IPAddress& dest)
{
    Enter_Method("nextGatewayAddress(%s)=?", dest.getString());

    for (int i = 0; i < route->items(); i++) {
        if (route->get(i)) {
            RoutingEntry *e = (RoutingEntry*)route->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
                return e->gateway;
            }
        }
    }

    if (defaultRoute)
        return defaultRoute->gateway;

    return IPAddress();
}


bool RoutingTable::add(const IPAddress& target,
                       const IPAddress& netmask,
                       const IPAddress& gw,
                       int metric,
                       char *dev)
{
    Enter_Method("add(...)");

    RoutingEntry *e = new RoutingEntry();
    int dev_nb;

    e->host = target;
    // We add at the end, if everything is alright

    if (!netmask.isNull()) {
        e->netmask = netmask;
    } else {
        // If class D or E, the netmask is NULL here
        e->netmask = target.getNetworkMask();
    }

    if (!gw.isNull()) {
        e->gateway = gw;
        e->type = REMOTE;
    } else {
        e->gateway = IPAddress();
        e->type = DIRECT;
    }

    e->metric = metric;

    if (dev) {
        dev_nb = findInterfaceByName(dev);
        if (dev_nb != -1) {
            // The interface exists
            e->interfaceName = dev;
            e->interfaceNo = dev_nb;
        } else {
            // The interface doesn't exist
            opp_error("Adding a route failed: nonexistent interface");
        }
    } else {
        // Trying to find the interface
        if (e->type == REMOTE) {
            // If it's a gateway entry, looking in the routing table
            // for existing route for the gateway value
            dev_nb = outputPortNo(gw);
            if (dev_nb != -1) {
                // Route for the gateway exists
                e->interfaceNo = dev_nb;
                e->interfaceName = intrface[dev_nb]->name;
            } else {
                // No route for the gateway, looking in the interface table
                dev_nb = findInterfaceByAddress(gw);
                if (dev_nb != -1) {
                    // The interface for the gateway address exists
                    e->interfaceNo = dev_nb;
                    e->interfaceName = intrface[dev_nb]->name;
                } else {
                    // The interface doesn't exist
                    opp_error("Adding a route failed: nonexistent interface");
                }
            }
        } else {
            // If it's a host entry, looking in the interface table
            // for the target address
            dev_nb = findInterfaceByAddress(e->host);
            if (dev_nb != -1) {
                // The interface for the gateway address exists
                e->interfaceNo = dev_nb;
                e->interfaceName = intrface[dev_nb]->name;
            } else {
                // The interface doesn't exist
                opp_error("Adding a route failed: no interface found");
            }
        }
    }

    // Adding...
    // If not the default route
    if (!target.isNull()) {
        // Check if entry is for multicast address
        if (e->host.isMulticast()) {
            route->add(e);
        } else {
            mcRoute->add(e);
        }
    } else {
        defaultRoute = e;
    }

    return true;
}


bool RoutingTable::del(const IPAddress& target,
                       const IPAddress& netmask,
                       const IPAddress& gw,
                       int metric,
                       char *dev)
{
    Enter_Method("del(...)");
    bool res = false;

    if (target) {
        // A target is given => not default route
        RoutingEntry *e;
        for (int i = 0; i < route->items(); i++) {
            if (route->get(i)) {
                e = (RoutingEntry*)route->get(i);
                if (e->correspondTo(target, netmask, gw, metric, dev)) {
                    if (e->host.isMulticast()) {
                        route->remove(e);
                    } else {
                        mcRoute->remove(e);
                    }
                    res = true;
                }
            }
        }
    } else {
        // Delete default entry
        if (defaultRoute->correspondTo(target, netmask, gw, metric, dev)) {
            defaultRoute = NULL;
            res = true;
        }
    }

    return res;
}


