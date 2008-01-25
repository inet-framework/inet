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
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "IPAddressResolver.h"
#include "InterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "RoutingTable.h"
#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#include "RoutingTable6.h"
#endif


IPvXAddress IPAddressResolver::resolve(const char *s, int addrType)
{
    IPvXAddress addr;
    if (!tryResolve(s, addr, addrType))
        opp_error("IPAddressResolver: address `%s' not configured (yet?)", s);
    return addr;
}

bool IPAddressResolver::tryResolve(const char *s, IPvXAddress& result, int addrType)
{
    // empty address
    result = IPvXAddress();
    if (!s || !*s)
        return true;

    // handle address literal
    if (result.tryParse(s))
        return true;

    // must be "modulename/interfacename(protocol)" syntax then,
    // "/interfacename" and "(protocol)" being optional
    const char *slashp = strchr(s,'/');
    const char *leftparenp = strchr(s,'(');
    const char *rightparenp = strchr(s,')');
    const char *endp = s+strlen(s);

    // rudimentary syntax check
    if ((slashp && leftparenp && slashp>leftparenp) ||
        (leftparenp && !rightparenp) ||
        (!leftparenp && rightparenp) ||
        (rightparenp && rightparenp!=endp-1))
    {
        opp_error("IPAddressResolver: syntax error parsing address spec `%s'", s);
    }

    // parse fields: modname, ifname, protocol
    std::string modname, ifname, protocol;
    modname.assign(s, (slashp?slashp:leftparenp?leftparenp:endp)-s);
    if (slashp)
        ifname.assign(slashp+1, (leftparenp?leftparenp:endp)-slashp-1);
    if (leftparenp)
        protocol.assign(leftparenp+1, rightparenp-leftparenp-1);

    // find module and check protocol
    cModule *mod = simulation.moduleByPath(modname.c_str());
    if (!mod)
        opp_error("IPAddressResolver: module `%s' not found", modname.c_str());
    if (!protocol.empty() && protocol!="ipv4" && protocol!="ipv6")
        opp_error("IPAddressResolver: error parsing address spec `%s': address type must be `(ipv4)' or `(ipv6)'", s);
    if (!protocol.empty())
        addrType = protocol=="ipv4" ? ADDR_IPv4 : ADDR_IPv6;

    // get address from the given module/interface
    if (ifname.empty())
        result = addressOf(mod, addrType);
    else if (ifname == "routerId")
        result = IPvXAddress(routerIdOf(mod)); // addrType is meaningless here, routerId is protocol independent
    else
        result = addressOf(mod, ifname.c_str(), addrType);
    return !result.isUnspecified();
}

IPAddress IPAddressResolver::routerIdOf(cModule *host)
{
    RoutingTable *rt = routingTableOf(host);
    return rt->routerId();
}

IPvXAddress IPAddressResolver::addressOf(cModule *host, int addrType)
{
    InterfaceTable *ift = interfaceTableOf(host);
    return getAddressFrom(ift, addrType);
}

IPvXAddress IPAddressResolver::addressOf(cModule *host, const char *ifname, int addrType)
{
    InterfaceTable *ift = interfaceTableOf(host);
    InterfaceEntry *ie = ift->interfaceByName(ifname);
    if (!ie)
        opp_error("IPAddressResolver: no interface called `%s' in `%s'", ifname, ift->fullPath().c_str());
    return getAddressFrom(ie, addrType);
}

IPvXAddress IPAddressResolver::getAddressFrom(InterfaceTable *ift, int addrType)
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
        opp_error("IPAddressResolver: unknown addrType %d", addrType);
    }
    return ret;
}

IPvXAddress IPAddressResolver::getAddressFrom(InterfaceEntry *ie, int addrType)
{
    IPvXAddress ret;
    if (addrType==ADDR_IPv6 || addrType==ADDR_PREFER_IPv6)
    {
        if (ie->ipv6())
            ret = getInterfaceIPv6Address(ie);
        if (ret.isUnspecified() && addrType==ADDR_PREFER_IPv6 && ie->ipv4())
            ret = ie->ipv4()->inetAddress();
    }
    else if (addrType==ADDR_IPv4 || addrType==ADDR_PREFER_IPv4)
    {
        if (ie->ipv4())
            ret = ie->ipv4()->inetAddress();
        if (ret.isUnspecified() && addrType==ADDR_PREFER_IPv4 && ie->ipv6())
            ret = getInterfaceIPv6Address(ie);
    }
    else
    {
        opp_error("IPAddressResolver: unknown addrType %d", addrType);
    }
    return ret;
}

IPAddress IPAddressResolver::getIPv4AddressFrom(InterfaceTable *ift)
{
    IPAddress addr;
    if (ift->numInterfaces()==0)
        opp_error("IPAddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->fullPath().c_str());

    // choose first usable interface address (configured for IPv4, non-loopback if, addr non-null)
    for (int i=0; i<ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);
        if (ie->ipv4() && !ie->ipv4()->inetAddress().isUnspecified() && !ie->isLoopback())
        {
            addr = ie->ipv4()->inetAddress();
            break;
        }
    }
    return addr;
}

IPv6Address IPAddressResolver::getIPv6AddressFrom(InterfaceTable *ift)
{
#ifdef WITH_IPv6
    // browse interfaces and pick a globally routable address
    if (ift->numInterfaces()==0)
        opp_error("IPAddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->fullPath().c_str());

    IPv6Address addr;
    for (int i=0; i<ift->numInterfaces() && addr.isUnspecified(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);
        if (!ie->ipv6() || ie->isLoopback())
            continue;
        IPv6Address ifAddr = ie->ipv6()->preferredAddress();
        if (addr.isGlobal() && ifAddr.isGlobal() && addr!=ifAddr)
            EV << ift->fullPath() << " has at least two globally routable addresses on different interfaces\n";
        if (ifAddr.isGlobal())
            addr = ifAddr;
    }
    return addr;
#else
    return IPv6Address();
#endif
}

IPv6Address IPAddressResolver::getInterfaceIPv6Address(InterfaceEntry *ie)
{
#ifdef WITH_IPv6
    if (!ie->ipv6())
        return IPv6Address();
    return ie->ipv6()->preferredAddress();
#else
    return IPv6Address();
#endif
}

InterfaceTable *IPAddressResolver::interfaceTableOf(cModule *host)
{
    // find InterfaceTable
    cModule *mod = host->submodule("interfaceTable");
    if (!mod)
        opp_error("IPAddressResolver: InterfaceTable not found as submodule "
                  " `interfaceTable' in host/router `%s'", host->fullPath().c_str());
    return check_and_cast<InterfaceTable *>(mod);
}

RoutingTable *IPAddressResolver::routingTableOf(cModule *host)
{
    // find RoutingTable
    cModule *mod = host->submodule("routingTable");
    if (!mod)
        opp_error("IPAddressResolver: RoutingTable not found as submodule "
                  " `routingTable' in host/router `%s'", host->fullPath().c_str());
    return check_and_cast<RoutingTable *>(mod);
}

#ifdef WITH_IPv6
RoutingTable6 *IPAddressResolver::routingTable6Of(cModule *host)
{
    // find RoutingTable
    cModule *mod = host->submodule("routingTable6");
    if (!mod)
        opp_error("IPAddressResolver: RoutingTable6 not found as submodule "
                  " `routingTable6' in host/router `%s'", host->fullPath().c_str());
    return check_and_cast<RoutingTable6 *>(mod);
}
#endif

NotificationBoard *IPAddressResolver::notificationBoardOf(cModule *host)
{
    // find NotificationBoard
    cModule *mod = host->submodule("notificationBoard");
    if (!mod)
        opp_error("IPAddressResolver: NotificationBoard not found as submodule "
                  " notificationBoard' in host/router `%s'", host->fullPath().c_str());
    return check_and_cast<NotificationBoard *>(mod);
}

InterfaceTable *IPAddressResolver::findInterfaceTableOf(cModule *host)
{
    cModule *mod = host->submodule("interfaceTable");
    return dynamic_cast<InterfaceTable *>(mod);
}

RoutingTable *IPAddressResolver::findRoutingTableOf(cModule *host)
{
    cModule *mod = host->submodule("routingTable");
    return dynamic_cast<RoutingTable *>(mod);
}

#ifdef WITH_IPv6
RoutingTable6 *IPAddressResolver::findRoutingTable6Of(cModule *host)
{
    cModule *mod = host->submodule("routingTable6");
    return dynamic_cast<RoutingTable6 *>(mod);
}
#endif

NotificationBoard *IPAddressResolver::findNotificationBoardOf(cModule *host)
{
    cModule *mod = host->submodule("notificationBoard");
    return dynamic_cast<NotificationBoard *>(mod);
}




