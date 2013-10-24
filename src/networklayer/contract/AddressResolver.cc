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

#include "AddressResolver.h"
#include "IInterfaceTable.h"

#include "ModulePathAddress.h"
#include "ModuleIdAddress.h"
#include "GenericNetworkProtocolInterfaceData.h"

#ifdef WITH_IPv4
#include "IPv4NetworkConfigurator.h"
#include "IIPv4RoutingTable.h"
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#include "IPv6RoutingTable.h"
#endif

#include "GenericRoutingTable.h"

Address AddressResolver::resolve(const char *s, int addrType)
{
    Address addr;
    if (!tryResolve(s, addr, addrType))
        throw cRuntimeError("AddressResolver: address `%s' not configured (yet?)", s);
    return addr;
}

std::vector<Address> AddressResolver::resolve(std::vector<std::string> strs, int addrType)
{
    std::vector<Address> result;
    int n = strs.size();
    result.reserve(n);
    for (int i=0; i<n; i++)
        result.push_back(resolve(strs[i].c_str(), addrType));
    return result;
}

bool AddressResolver::tryResolve(const char *s, Address& result, int addrType)
{
    // empty address
    result = Address();
    if (!s || !*s)
        return true;

    // handle address literal
    if (result.tryParse(s))
        return true;

    // must be " modulename [ { '%' interfacename | '>' destnode } ] [ '(' protocol ')' ] [ '/' ] " syntax
    // interfacename: existing_interface_of_module | 'routerId'
    // protocol: 'ipv4' | 'ipv6' | 'mac' | 'modulepath' | 'moduleid'
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
        throw cRuntimeError("AddressResolver: syntax error parsing address spec `%s'", s);

    // find module
    cModule *mod = simulation.getModuleByPath(modname.c_str());
    if (!mod)
        throw cRuntimeError("AddressResolver: module `%s' not found", modname.c_str());


    // check protocol
    if (!protocol.empty())
    {
        if (protocol == "ipv4")
            addrType = ADDR_IPv4;
        else if (protocol == "ipv6")
            addrType = ADDR_IPv6;
        else if (protocol == "mac")
            addrType = ADDR_MAC;
        else if (protocol == "modulepath")
            addrType = ADDR_MODULEPATH;
        else if (protocol == "moduleid")
            addrType = ADDR_MODULEID;
        else
            throw cRuntimeError("AddressResolver: error parsing address spec `%s': address type must be `(ipv4)' or `(ipv6)'", s);
    }
    if (netmask)
        addrType |= ADDR_MASK;

    // find interface for dest node
    // get address from the given module/interface
    if (!destnodename.empty())
    {
        cModule *destnode = simulation.getModuleByPath(destnodename.c_str());
        if (!destnode)
            throw cRuntimeError("AddressResolver: destination module `%s' not found", destnodename.c_str());
        result = addressOf(mod, destnode, addrType);
    }
    else if (ifname.empty())
        result = addressOf(mod, addrType);
    else if (ifname == "routerId")
        result = routerIdOf(mod); // addrType is meaningless here, routerId is protocol independent
    else
        result = addressOf(mod, ifname.c_str(), addrType);
    return !result.isUnspecified();
}

Address AddressResolver::routerIdOf(cModule *host)
{
#ifdef WITH_IPv4
    IIPv4RoutingTable *rt = routingTableOf(host);
    return Address(rt->getRouterId());
#else
    throw cRuntimeError("INET was compiled without IPv4 support");
#endif
}

Address AddressResolver::addressOf(cModule *host, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    return getAddressFrom(ift, addrType);
}

Address AddressResolver::addressOf(cModule *host, const char *ifname, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    InterfaceEntry *ie = ift->getInterfaceByName(ifname);
    if (ie)
        return getAddressFrom(ie, addrType);

    throw cRuntimeError("AddressResolver: no interface called `%s' in interface table of `%s'", ifname, host->getFullPath().c_str());
}

Address AddressResolver::addressOf(cModule *host, cModule *destmod, int addrType)
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
    throw cRuntimeError("AddressResolver: no interface connected to `%s' module in interface table of `%s'", destmod->getFullPath().c_str(), host->getFullPath().c_str());
}

Address AddressResolver::getAddressFrom(IInterfaceTable *ift, int addrType)
{
    Address ret;
    bool netmask = addrType & ADDR_MASK;
    if ((addrType & ADDR_IPv4) && getIPv4AddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_IPv6) && getIPv6AddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_MAC) && getMACAddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_MODULEPATH) && getModulePathAddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_MODULEID) && getModuleIdAddressFrom(ret, ift, netmask))
        return ret;
    else
        throw cRuntimeError("AddressResolver: unknown addrType %d", addrType);
    return ret;
}

Address AddressResolver::getAddressFrom(InterfaceEntry *ie, int addrType)
{
    Address ret;
    bool mask = addrType & ADDR_MASK;

    if ((addrType & ADDR_IPv4) && getInterfaceIPv4Address(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_IPv6) && getInterfaceIPv6Address(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_MAC) && getInterfaceMACAddress(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_MODULEPATH) && getInterfaceModulePathAddress(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_MODULEID) && getInterfaceModuleIdAddress(ret, ie, mask))
        return ret;
    else
        throw cRuntimeError("AddressResolver: unknown addrType %d", addrType);

    return ret;
}

bool AddressResolver::getIPv4AddressFrom(Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("AddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifdef WITH_IPv4
    // choose first usable interface address (configured for IPv4, non-loopback if, addr non-null)
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        if (getInterfaceIPv4Address(retAddr, ie, netmask))
            return true;
    }
#endif
    return false;
}

bool AddressResolver::getIPv6AddressFrom(Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    // browse interfaces and pick a globally routable address
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("AddressResolver: interface table `%s' has no interface registered "
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

bool AddressResolver::getMACAddressFrom(Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("AddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

    // choose first usable interface address (configured for generic, non-loopback if, addr non-null)
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (!ie->getGenericNetworkProtocolData() || ie->isLoopback())
            continue;
        if (getInterfaceMACAddress(retAddr, ie, netmask))
            return true;
    }
    return false;
}

bool AddressResolver::getModulePathAddressFrom(Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("AddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

    // choose first usable interface address (configured for generic, non-loopback if, addr non-null)
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (!ie->getGenericNetworkProtocolData() || ie->isLoopback())
            continue;
        if (getInterfaceModulePathAddress(retAddr, ie, netmask))
            return true;
    }
    return false;
}

bool AddressResolver::getModuleIdAddressFrom(Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces()==0)
        throw cRuntimeError("AddressResolver: interface table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", ift->getFullPath().c_str());

    // choose first usable interface address (configured for generic, non-loopback if, addr non-null)
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (!ie->getGenericNetworkProtocolData() || ie->isLoopback())
            continue;
        if (getInterfaceModuleIdAddress(retAddr, ie, netmask))
            return true;
    }
    return false;
}

bool AddressResolver::getInterfaceIPv6Address(Address &ret, InterfaceEntry *ie, bool netmask)
{
#ifdef WITH_IPv6
    if (netmask)
        return false;   // IPv6 netmask not supported yet
    if (ie->ipv6Data())
    {
        IPv6Address addr = ie->ipv6Data()->getPreferredAddress();
        if (!addr.isUnspecified())
        {
            ret.set(addr);
            return true;
        }
    }
#endif
    return false;
}

bool AddressResolver::getInterfaceIPv4Address(Address &ret, InterfaceEntry *ie, bool netmask)
{
#ifdef WITH_IPv4
    if (ie->ipv4Data())
    {
        IPv4Address addr = ie->ipv4Data()->getIPAddress();
        if (!addr.isUnspecified())
        {
            ret.set(netmask ? ie->ipv4Data()->getNetmask() : addr);
            return true;
        }
    }
    else
    {
        // find address in the configurator's notebook
        // TODO: how do we know where is the configurator? get the path from a NED parameter?
        AddressResolver *configurator = dynamic_cast<AddressResolver *>(simulation.getModuleByPath("configurator"));
        if (configurator)
            return configurator->getInterfaceIPv4Address(ret, ie, netmask);
    }
#endif
    return false;
}

bool AddressResolver::getInterfaceMACAddress(Address &ret, InterfaceEntry *ie, bool netmask)
{
    if (ie->getGenericNetworkProtocolData())
    {
        Address addr = ie->getGenericNetworkProtocolData()->getAddress();
        if (!addr.isUnspecified() && addr.getType() == Address::MAC)
        {
            ret = addr;
            return true;
        }
    }
    return false;
}

bool AddressResolver::getInterfaceModulePathAddress(Address &ret, InterfaceEntry *ie, bool netmask)
{
    if (ie->getGenericNetworkProtocolData())
    {
        Address addr = ie->getGenericNetworkProtocolData()->getAddress();
        if (!addr.isUnspecified() && addr.getType() == Address::MODULEPATH)
        {
            ret = addr;
            return true;
        }
        return true;
    }
    return false;
}

bool AddressResolver::getInterfaceModuleIdAddress(Address &ret, InterfaceEntry *ie, bool netmask)
{
    if (ie->getGenericNetworkProtocolData())
    {
        Address addr = ie->getGenericNetworkProtocolData()->getAddress();
        if (!addr.isUnspecified() && addr.getType() == Address::MODULEID)
        {
            ret = addr;
            return true;
        }
        return true;
    }
    return false;
}

IInterfaceTable *AddressResolver::interfaceTableOf(cModule *host)
{
    // find IInterfaceTable
    cModule *mod = host->getSubmodule("interfaceTable");
    if (!mod)
        throw cRuntimeError("AddressResolver: IInterfaceTable not found as submodule "
                  " `interfaceTable' in host/router `%s'", host->getFullPath().c_str());

    return check_and_cast<IInterfaceTable *>(mod);
}

IIPv4RoutingTable *AddressResolver::routingTableOf(cModule *host)
{
    IIPv4RoutingTable *mod = findIPv4RoutingTableOf(host);
    if (!mod)
        throw cRuntimeError("AddressResolver: IIPv4RoutingTable not found as submodule "
                  " `routingTable' in host/router `%s'", host->getFullPath().c_str());
    return mod;
}

IPv6RoutingTable *AddressResolver::routingTable6Of(cModule *host)
{
    // find IPv6RoutingTable
    IPv6RoutingTable *mod = findIPv6RoutingTableOf(host);
    if (!mod)
        throw cRuntimeError("AddressResolver: IPv6RoutingTable not found as submodule "
                  " `routingTable' in host/router `%s'", host->getFullPath().c_str());
    return mod;
}

IInterfaceTable *AddressResolver::findInterfaceTableOf(cModule *host)
{
    cModule *mod = host->getSubmodule("interfaceTable");
    return dynamic_cast<IInterfaceTable *>(mod);
}

IIPv4RoutingTable *AddressResolver::findIPv4RoutingTableOf(cModule *host)
{
#ifdef WITH_IPv4
    // KLUDGE: TODO: look deeper temporarily
    IIPv4RoutingTable *rt = dynamic_cast<IIPv4RoutingTable *>(host->getSubmodule("routingTable"));
    if (!rt) rt = dynamic_cast<IIPv4RoutingTable *>(host->getModuleByPath(".routingTable.ipv4"));
    return rt;
#else
    return NULL;
#endif
}

IPv6RoutingTable *AddressResolver::findIPv6RoutingTableOf(cModule *host)
{
#ifdef WITH_IPv6
    // KLUDGE: TODO: look deeper temporarily
    IPv6RoutingTable *rt = dynamic_cast<IPv6RoutingTable *>(host->getSubmodule("routingTable"));
    if (!rt) rt = dynamic_cast<IPv6RoutingTable *>(host->getModuleByPath(".routingTable.ipv6"));
    return rt;
#else
    return NULL;
#endif
}

GenericRoutingTable *AddressResolver::findGenericRoutingTableOf(cModule *host)
{
    // KLUDGE: TODO: look deeper temporarily
    GenericRoutingTable *rt = dynamic_cast<GenericRoutingTable *>(host->getSubmodule("routingTable"));
    if (!rt) rt = dynamic_cast<GenericRoutingTable *>(host->getModuleByPath(".routingTable.generic"));
    return rt;
}

cModule *AddressResolver::findHostWithAddress(const Address & add)
{
    if (add.isUnspecified() || add.isMulticast())
        return NULL;

    cTopology topo("topo");
    topo.extractByProperty("node");

    // fill in isIPNode, ift and rt members in nodeInfo[]

    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cModule *mod = topo.getNode(i)->getModule();
        IInterfaceTable * itable = AddressResolver().findInterfaceTableOf(mod);
        if (itable != NULL)
        {
            for (int i = 0; i < itable->getNumInterfaces(); i++)
            {
                InterfaceEntry *entry = itable->getInterface(i);
                switch (add.getType())
                {
#ifdef WITH_IPv6
                    case Address::IPv6:
                        if (entry->ipv6Data()->hasAddress(add.toIPv6()))
                            return mod;
                        break;
#endif
#ifdef WITH_IPv4
                    case Address::IPv4:
                        if (entry->ipv4Data()->getIPAddress() == add.toIPv4())
                            return mod;
                        break;
#endif
                    default:
                        throw cRuntimeError("findHostWithAddress() doesn't accept AddressType '%s', yet", Address::getTypeName(add.getType()));
                        break;
                }
            }
        }
    }
    return NULL;
}
