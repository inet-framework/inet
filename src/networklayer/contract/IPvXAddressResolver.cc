//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "IPvXAddressResolver.h"
#include "IInterfaceTable.h"
#include "NotificationBoard.h"

#ifdef WITH_IPv4
#include "IRoutingTable.h"
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#include "RoutingTable6.h"
#endif


IPvXAddress IPvXAddressResolver::resolve(const char *s, int addrType)
{
    IPvXAddress addr;
    if (!tryResolve(s, addr, addrType))
        throw cRuntimeError("IPvXAddressResolver: address `%s' not configured (yet?)", s);

    return addr;
}

std::vector<IPvXAddress> IPvXAddressResolver::resolve(std::vector<std::string> strs, int addrType)
{
    std::vector<IPvXAddress> result;
    int n = strs.size();
    result.reserve(n);
    for (int i=0; i<n; i++)
        result.push_back(resolve(strs[i].c_str(), addrType));
    return result;
}

bool IPvXAddressResolver::tryResolve(const char *s, IPvXAddress& result, int addrType)
{
    // empty address
    result = IPvXAddress();
    if (!s || !*s)
        return true;

    // handle address literal
    if (result.tryParse(s))
        return true;

    // must be " modulename[ { '%' interfacename | '>' destnode } ] [ '(' protocol ')' ]" syntax
    // must be "modulename%interfacename(protocol)" syntax then,
    // "%interfacename" and "(protocol)" being optional
    std::string modname, ifname, protocol, destnodename;
    const char *p = s;
    const char *endp = strchr(p, '\0');
    const char *nextsep = strpbrk(p,"%>()");
    if (!nextsep)
        nextsep = endp;
    modname.assign(p, nextsep - p);

    char c = *nextsep;

    if (c) { p = nextsep + 1; nextsep = strpbrk(p, "%>()"); if (!nextsep) nextsep = endp; }

    if (c == '%')
    {
        ifname.assign(p, nextsep - p);
        c = *nextsep;
        if (c) { p = nextsep + 1; nextsep = strpbrk(p, "%>()"); if (!nextsep) nextsep = endp; }
    }
    else if (c == '>')
    {
        destnodename.assign(p, nextsep - p);
        c = *nextsep;
        if (c) { p = nextsep + 1; nextsep = strpbrk(p, "%>()"); if (!nextsep) nextsep = endp; }
    }

    if (c == '(' && *nextsep == ')')
    {
        protocol.assign(p, nextsep - p);
        p = nextsep + 1;
        nextsep++;
        c = *nextsep;
    }

    if (c)
        throw cRuntimeError("IPvXAddressResolver: syntax error parsing address spec `%s'", s);

    // find module
    cModule *mod = simulation.getModuleByPath(modname.c_str());
    if (!mod)
        throw cRuntimeError("IPvXAddressResolver: module `%s' not found", modname.c_str());


    // check protocol
    if (!protocol.empty())
    {
        if (protocol == "ipv4")
            addrType = ADDR_IPv4;
        else if (protocol == "ipv6")
            addrType = ADDR_IPv6;
        else
            throw cRuntimeError("IPvXAddressResolver: error parsing address spec `%s': address type must be `(ipv4)' or `(ipv6)'", s);
    }

    // find interface for dest node
    // get address from the given module/interface
    if (!destnodename.empty())
    {
        cModule *destnode = simulation.getModuleByPath(destnodename.c_str());
        if (!destnode)
            throw cRuntimeError("IPvXAddressResolver: destination module `%s' not found", destnodename.c_str());
        result = addressOf(mod, destnode, addrType);
    }
    else if (ifname.empty())
        result = addressOf(mod, addrType);
    else if (ifname == "routerId")
        result = IPvXAddress(routerIdOf(mod)); // addrType is meaningless here, routerId is protocol independent
    else
        result = addressOf(mod, ifname.c_str(), addrType);
    return !result.isUnspecified();
}

IPv4Address IPvXAddressResolver::routerIdOf(cModule *host)
{
#ifdef WITH_IPv4
    IRoutingTable *rt = routingTableOf(host);
    return rt->getRouterId();
#else
    throw cRuntimeError("INET was compiled without IPv4 support");
#endif
}

IPvXAddress IPvXAddressResolver::addressOf(cModule *host, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    return getAddressFrom(ift, addrType);
}

IPvXAddress IPvXAddressResolver::addressOf(cModule *host, const char *ifname, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    InterfaceEntry *ie = ift->getInterfaceByName(ifname);
    if (!ie)
        throw cRuntimeError("IPvXAddressResolver: no interface called `%s' in interface table of `%s'", ifname, host->getFullPath().c_str());

    return getAddressFrom(ie, addrType);
}

IPvXAddress IPvXAddressResolver::addressOf(cModule *host, cModule *destmod, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie)
        {
            int gateId = ie->getNodeOutputGateId();
            if (gateId != -1)
                if (host->gate(gateId)->pathContains(destmod))
                    return getAddressFrom(ie, addrType);
        }
    }
    throw cRuntimeError("IPvXAddressResolver: no interface connected to `%s' module in interface table of `%s'", destmod->getFullPath().c_str(), host->getFullPath().c_str());
}

IPvXAddress IPvXAddressResolver::getAddressFrom(IInterfaceTable *ift, int addrType)
{
    IPvXAddress ret;
    if (addrType==ADDR_IPv6 || addrType==ADDR_PREFER_IPv6)
    {
        ret = getIPv6AddressFrom(ift);
        if (ret.isUnspecified() && addrType==ADDR_PREFER_IPv6)
            ret = getIPv4AddressFrom(ift);
    }
    else if (addrType==ADDR_IPv4 || addrType==ADDR_PREFER_IPv4)
    {
        ret = getIPv4AddressFrom(ift);
        if (ret.isUnspecified() && addrType==ADDR_PREFER_IPv4)
            ret = getIPv6AddressFrom(ift);
    }
    else
    {
        throw cRuntimeError("IPvXAddressResolver: unknown addrType %d", addrType);
    }

    return ret;
}

IPvXAddress IPvXAddressResolver::getAddressFrom(InterfaceEntry *ie, int addrType)
{
    IPvXAddress ret;

    if (addrType==ADDR_IPv6 || addrType==ADDR_PREFER_IPv6)
    {
        ret = getInterfaceIPv6Address(ie);
        if (ret.isUnspecified() && addrType==ADDR_PREFER_IPv6)
            ret = getInterfaceIPv4Address(ie);
    }
    else if (addrType==ADDR_IPv4 || addrType==ADDR_PREFER_IPv4)
    {
        ret = getInterfaceIPv4Address(ie);
        if (ret.isUnspecified() && addrType==ADDR_PREFER_IPv4)
            ret = getInterfaceIPv6Address(ie);
    }
    else
    {
        throw cRuntimeError("IPvXAddressResolver: unknown addrType %d", addrType);
    }

    return ret;
}

IPv4Address IPvXAddressResolver::getIPv4AddressFrom(IInterfaceTable *ift)
{
    IPv4Address addr;

    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("IPvXAddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifdef WITH_IPv4
    // choose first usable interface address (configured for IPv4, non-loopback if, addr non-null)
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data() && !ie->ipv4Data()->getIPAddress().isUnspecified() && !ie->isLoopback())
        {
            addr = ie->ipv4Data()->getIPAddress();
            break;
        }
    }
#endif
    return addr;
}

IPv6Address IPvXAddressResolver::getIPv6AddressFrom(IInterfaceTable *ift)
{
    IPv6Address addr;

    // browse interfaces and pick a globally routable address
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("IPvXAddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifdef WITH_IPv6
    for (int i=0; i<ift->getNumInterfaces() && addr.isUnspecified(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (!ie->ipv6Data() || ie->isLoopback())
            continue;
        IPv6Address ifAddr = ie->ipv6Data()->getPreferredAddress();
        if (addr.isGlobal() && ifAddr.isGlobal() && addr!=ifAddr)
            EV << ift->getFullPath() << " has at least two globally routable addresses on different interfaces\n";
        if (ifAddr.isGlobal())
            addr = ifAddr;
    }
#endif
    return addr;
}

IPv6Address IPvXAddressResolver::getInterfaceIPv6Address(InterfaceEntry *ie)
{
#ifdef WITH_IPv6
    if (ie->ipv6Data())
        return ie->ipv6Data()->getPreferredAddress();
#endif
    return IPv6Address();
}

IPv4Address IPvXAddressResolver::getInterfaceIPv4Address(InterfaceEntry *ie)
{
#ifdef WITH_IPv4
    if (ie->ipv4Data())
        return ie->ipv4Data()->getIPAddress();
#endif
    return IPv4Address();
}

IInterfaceTable *IPvXAddressResolver::interfaceTableOf(cModule *host)
{
    // find IInterfaceTable
    cModule *mod = host->getSubmodule("interfaceTable");
    if (!mod)
        throw cRuntimeError("IPvXAddressResolver: IInterfaceTable not found as submodule "
                  " `interfaceTable' in host/router `%s'", host->getFullPath().c_str());

    return check_and_cast<IInterfaceTable *>(mod);
}

IRoutingTable *IPvXAddressResolver::routingTableOf(cModule *host)
{
    IRoutingTable *mod = findRoutingTableOf(host);
    if (!mod)
        throw cRuntimeError("IPvXAddressResolver: IRoutingTable not found as submodule "
                  " `routingTable' in host/router `%s'", host->getFullPath().c_str());
    return mod;
}

RoutingTable6 *IPvXAddressResolver::routingTable6Of(cModule *host)
{
    // find RoutingTable6
    RoutingTable6 *mod = findRoutingTable6Of(host);
    if (!mod)
        throw cRuntimeError("IPvXAddressResolver: RoutingTable6 not found as submodule "
                  " `routingTable6' in host/router `%s'", host->getFullPath().c_str());
    return mod;
}

NotificationBoard *IPvXAddressResolver::notificationBoardOf(cModule *host)
{
    // find NotificationBoard
    cModule *mod = host->getSubmodule("notificationBoard");
    if (!mod)
        throw cRuntimeError("IPvXAddressResolver: NotificationBoard not found as submodule "
                  " notificationBoard' in host/router `%s'", host->getFullPath().c_str());

    return check_and_cast<NotificationBoard *>(mod);
}

IInterfaceTable *IPvXAddressResolver::findInterfaceTableOf(cModule *host)
{
    cModule *mod = host->getSubmodule("interfaceTable");
    return dynamic_cast<IInterfaceTable *>(mod);
}

IRoutingTable *IPvXAddressResolver::findRoutingTableOf(cModule *host)
{
#ifdef WITH_IPv4
    cModule *mod = host->getSubmodule("routingTable");
    return dynamic_cast<IRoutingTable *>(mod);
#else
    return NULL;
#endif
}

RoutingTable6 *IPvXAddressResolver::findRoutingTable6Of(cModule *host)
{
#ifdef WITH_IPv6
    cModule *mod = host->getSubmodule("routingTable6");
    return dynamic_cast<RoutingTable6 *>(mod);
#else
    return NULL;
#endif
}

NotificationBoard *IPvXAddressResolver::findNotificationBoardOf(cModule *host)
{
    cModule *mod = host->getSubmodule("notificationBoard");
    return dynamic_cast<NotificationBoard *>(mod);
}

