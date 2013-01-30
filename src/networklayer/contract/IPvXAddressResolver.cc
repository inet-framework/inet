//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2012 OpenSim Ltd.
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
// @author Andras Varga
// @author Zoltan Bojthe
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

    // must be " modulename [ { '%' interfacename | '>' destnode } ] [ '(' protocol ')' ] [ '/' ] " syntax
    // interfacename: existing_interface_of_module | 'routerId'
    // protocol: 'ipv4' | 'ipv6'
    // '/': returns mask instead address
    std::string modname, ifname, protocol, destnodename;
    bool netmask = addrType & ADDR_MASK;
    const char *p = s;
    const char *endp = strchr(p, '\0');
    const char *nextsep = strpbrk(p, "%>(/");
    if (!nextsep)
        nextsep = endp;
    modname.assign(p, nextsep - p);

    char c = *nextsep;

    if (c == '%')
    {
        { p = nextsep + 1; nextsep = strpbrk(p, "(/"); if (!nextsep) nextsep = endp; }
        ifname.assign(p, nextsep - p);
        c = *nextsep;
    }
    else if (c == '>')
    {
        { p = nextsep + 1; nextsep = strpbrk(p, "(/"); if (!nextsep) nextsep = endp; }
        destnodename.assign(p, nextsep - p);
        c = *nextsep;
    }

    if (c == '(')
    {
        { p = nextsep + 1; nextsep = strpbrk(p, ")"); if (!nextsep) nextsep = endp; }
        protocol.assign(p, nextsep - p);
        c = *nextsep;
        if (c == ')')
        {
            { p = nextsep + 1; nextsep = p; }
            c = *nextsep;
        }
    }

    if (c == '/')
    {
        netmask = true;
        { p = nextsep + 1; nextsep = p; }
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
    if (netmask)
        addrType |= ADDR_MASK;

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
    if (ie)
        return getAddressFrom(ie, addrType);

    throw cRuntimeError("IPvXAddressResolver: no interface called `%s' in interface table of `%s'", ifname, host->getFullPath().c_str());
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
    bool netmask = addrType & ADDR_MASK;

    if (addrType & ADDR_IPv6)
    {
        bool exists = getIPv6AddressFrom(ret, ift, netmask);
        if (!exists)
        {
            if(addrType & ADDR_PREFER)
                exists = getIPv4AddressFrom(ret, ift, netmask);
        }
    }
    else if (addrType & ADDR_IPv4)
    {
        bool exists = getIPv4AddressFrom(ret, ift, netmask);
        if (!exists)
        {
            if(addrType & ADDR_PREFER)
                exists = getIPv6AddressFrom(ret, ift, netmask);
        }
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
    bool mask = addrType & ADDR_MASK;

    if (addrType & ADDR_IPv6)
    {
        if (!getInterfaceIPv6Address(ret, ie, mask))
            if (addrType & ADDR_PREFER)
                getInterfaceIPv4Address(ret, ie, mask);
    }
    else if (addrType & ADDR_IPv4)
    {
        if (!getInterfaceIPv4Address(ret, ie, mask))
            if (addrType & ADDR_PREFER)
                getInterfaceIPv6Address(ret, ie, mask);
    }
    else
    {
        throw cRuntimeError("IPvXAddressResolver: unknown addrType %d", addrType);
    }

    return ret;
}

bool IPvXAddressResolver::getIPv4AddressFrom(IPvXAddress& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("IPvXAddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifdef WITH_IPv4
    // choose first usable interface address (configured for IPv4, non-loopback if, addr non-null)
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (!ie->ipv4Data() || ie->isLoopback())
            continue;
        if (getInterfaceIPv4Address(retAddr, ie, netmask))
            return true;
    }
#endif
    return false;
}

bool IPvXAddressResolver::getIPv6AddressFrom(IPvXAddress& retAddr, IInterfaceTable *ift, bool netmask)
{
    // browse interfaces and pick a globally routable address
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("IPvXAddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifndef WITH_IPv6
    return false;
#else
    if (netmask)
        return false;   // IPv6 netmask not supported yet

    bool ret = false;
    IPv6Address::Scope retScope = IPv6Address::UNSPECIFIED;

    for (int i=0; i < ift->getNumInterfaces() && retScope != IPv6Address::GLOBAL; i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (!ie->ipv6Data() || ie->isLoopback())
            continue;
        IPv6Address curAddr = ie->ipv6Data()->getPreferredAddress();
        IPv6Address::Scope curScope = curAddr.getScope();
        if (curScope > retScope)
            { retAddr = curAddr; retScope = curScope; ret = true; }
    }
    return ret;
#endif
}

bool IPvXAddressResolver::getInterfaceIPv6Address(IPvXAddress &ret, InterfaceEntry *ie, bool netmask)
{
#ifdef WITH_IPv6
    if (netmask)
        return false;   // IPv6 netmask not supported yet
    if (ie->ipv6Data())
    {
        IPv6Address addr = ie->ipv6Data()->getPreferredAddress();
        if (!addr.isUnspecified())
        {
            ret = addr;
            return true;
        }
    }
#endif
    return false;
}

bool IPvXAddressResolver::getInterfaceIPv4Address(IPvXAddress &ret, InterfaceEntry *ie, bool netmask)
{
#ifdef WITH_IPv4
    if (ie->ipv4Data())
    {
        IPv4Address addr = ie->ipv4Data()->getIPAddress();
        if (!addr.isUnspecified())
        {
            ret = netmask ? ie->ipv4Data()->getNetmask() : addr;
            return true;
        }
    }
#endif
    return false;
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

