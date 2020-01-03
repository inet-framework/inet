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

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/ModuleIdAddress.h"
#include "inet/networklayer/common/ModulePathAddress.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

#ifdef WITH_IPv4
#include "inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_NEXTHOP
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#include "inet/networklayer/nexthop/NextHopRoutingTable.h"
#endif // ifdef WITH_NEXTHOP

namespace inet {

L3Address L3AddressResolver::resolve(const char *s, int addrType)
{
    L3Address addr;
    if (!tryResolve(s, addr, addrType))
        throw cRuntimeError("L3AddressResolver: address `%s' not configured (yet?)", s);
    return addr;
}

std::vector<L3Address> L3AddressResolver::resolve(std::vector<std::string> strs, int addrType)
{
    std::vector<L3Address> result;
    int n = strs.size();
    result.reserve(n);
    for (int i = 0; i < n; i++)
        result.push_back(resolve(strs[i].c_str(), addrType));
    return result;
}

bool L3AddressResolver::tryParse(L3Address& result, const char *addr, int addrType)
{
    Ipv6Address ipv6;
    MacAddress mac;
    ModuleIdAddress moduleId;
    ModulePathAddress modulePath;
    if (((addrType & ADDR_IPv4) != 0) && Ipv4Address::isWellFormed(addr))
        result.set(Ipv4Address(addr));
    else if (((addrType & ADDR_IPv6) != 0) && ipv6.tryParse(addr))
        result.set(ipv6);
    else if (((addrType & ADDR_MAC) != 0) && mac.tryParse(addr))
        result.set(mac);
    else if (((addrType & ADDR_MODULEID) != 0) && moduleId.tryParse(addr))
        result.set(moduleId);
    else if (((addrType & ADDR_MODULEPATH) != 0) && modulePath.tryParse(addr))
        result.set(modulePath);
    else
        return false;
    return true;
}


bool L3AddressResolver::tryResolve(const char *s, L3Address& result, int addrType)
{
    // empty address
    result = L3Address();
    if (!s || !*s)
        return true;

    // handle address literal
    if (tryParse(result, s, addrType))
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

    if (c == '%') {
        {
            p = nextsep + 1;
            nextsep = strpbrk(p, "(/");
            if (!nextsep)
                nextsep = endp;
        }
        ifname.assign(p, nextsep - p);
        c = *nextsep;
    }
    else if (c == '>') {
        {
            p = nextsep + 1;
            nextsep = strpbrk(p, "(/");
            if (!nextsep)
                nextsep = endp;
        }
        destnodename.assign(p, nextsep - p);
        c = *nextsep;
    }

    if (c == '(') {
        {
            p = nextsep + 1;
            nextsep = strpbrk(p, ")");
            if (!nextsep)
                nextsep = endp;
        }
        protocol.assign(p, nextsep - p);
        c = *nextsep;
        if (c == ')') {
            {
                p = nextsep + 1;
                nextsep = p;
            }
            c = *nextsep;
        }
    }

    if (c == '/') {
        netmask = true;
        {
            p = nextsep + 1;
            nextsep = p;
        }
        c = *nextsep;
    }

    if (c)
        throw cRuntimeError("L3AddressResolver: syntax error parsing address spec `%s'", s);

    // find module
    cModule *mod = getSimulation()->getModuleByPath(modname.c_str());
    if (!mod)
        throw cRuntimeError("L3AddressResolver: module `%s' not found", modname.c_str());

    // check protocol
    if (!protocol.empty()) {
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
            throw cRuntimeError("L3AddressResolver: error parsing address spec `%s': address type must be `(ipv4)' or `(ipv6)'", s);
    }
    if (netmask)
        addrType |= ADDR_MASK;

    // find interface for dest node
    // get address from the given module/interface
    if (!destnodename.empty()) {
        cModule *destnode = getSimulation()->getModuleByPath(destnodename.c_str());
        if (!destnode)
            throw cRuntimeError("L3AddressResolver: destination module `%s' not found", destnodename.c_str());
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

L3Address L3AddressResolver::routerIdOf(cModule *host)
{
#ifdef WITH_IPv4
    IIpv4RoutingTable *rt = getIpv4RoutingTableOf(host);
    return L3Address(rt->getRouterId());
#else // ifdef WITH_IPv4
    throw cRuntimeError("INET was compiled without Ipv4 support");
#endif // ifdef WITH_IPv4
}

L3Address L3AddressResolver::addressOf(cModule *host, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    return getAddressFrom(ift, addrType);
}

L3Address L3AddressResolver::addressOf(cModule *host, const char *ifname, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    InterfaceEntry *ie = ift->findInterfaceByName(ifname);
    if (ie)
        return getAddressFrom(ie, addrType);

    throw cRuntimeError("L3AddressResolver: no interface called `%s' in interface table of `%s'", ifname, host->getFullPath().c_str());
}

L3Address L3AddressResolver::addressOf(cModule *host, cModule *destmod, int addrType)
{
    IInterfaceTable *ift = interfaceTableOf(host);
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie) {
            int gateId = ie->getNodeOutputGateId();
            if (gateId != -1)
                if (host->gate(gateId)->pathContains(destmod))
                    return getAddressFrom(ie, addrType);

        }
    }
    throw cRuntimeError("L3AddressResolver: no interface connected to `%s' module in interface table of `%s'", destmod->getFullPath().c_str(), host->getFullPath().c_str());
}

L3Address L3AddressResolver::getAddressFrom(IInterfaceTable *ift, int addrType)
{
    L3Address ret;
    bool netmask = addrType & ADDR_MASK;
    if ((addrType & ADDR_IPv4) && getIpv4AddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_IPv6) && getIpv6AddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_MAC) && getMacAddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_MODULEPATH) && getModulePathAddressFrom(ret, ift, netmask))
        return ret;
    else if ((addrType & ADDR_MODULEID) && getModuleIdAddressFrom(ret, ift, netmask))
        return ret;
    else
        throw cRuntimeError("L3AddressResolver: unknown addrType %d", addrType);
    return ret;
}

L3Address L3AddressResolver::getAddressFrom(InterfaceEntry *ie, int addrType)
{
    L3Address ret;
    bool mask = addrType & ADDR_MASK;

    if ((addrType & ADDR_IPv4) && getInterfaceIpv4Address(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_IPv6) && getInterfaceIpv6Address(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_MAC) && getInterfaceMacAddress(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_MODULEPATH) && getInterfaceModulePathAddress(ret, ie, mask))
        return ret;
    else if ((addrType & ADDR_MODULEID) && getInterfaceModuleIdAddress(ret, ie, mask))
        return ret;
    else
        throw cRuntimeError("L3AddressResolver: unknown addrType %d at %s", addrType, ie->getInterfaceFullPath().c_str());

    return ret;
}

bool L3AddressResolver::getIpv4AddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces() == 0)
        throw cRuntimeError("L3AddressResolver: interface table `%s' has no interface registered "
                            "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifdef WITH_IPv4
    // choose first usable interface address (configured for Ipv4, non-loopback if, addr non-null)
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        if (getInterfaceIpv4Address(retAddr, ie, netmask))
            return true;
    }
#endif // ifdef WITH_IPv4
    return false;
}

bool L3AddressResolver::getIpv6AddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    // browse interfaces and pick a globally routable address
    if (ift->getNumInterfaces() == 0)
        throw cRuntimeError("L3AddressResolver: interface table `%s' has no interface registered "
                            "(yet? try in a later init stage!)", ift->getFullPath().c_str());

#ifndef WITH_IPv6
    return false;
#else // ifndef WITH_IPv6
    if (netmask)
        return false; // Ipv6 netmask not supported yet

    bool ret = false;
    Ipv6Address::Scope retScope = Ipv6Address::UNSPECIFIED;

    for (int i = 0; i < ift->getNumInterfaces() && retScope != Ipv6Address::GLOBAL; i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
        if (!ipv6Data || ie->isLoopback())
            continue;
        Ipv6Address curAddr = ipv6Data->getPreferredAddress();
        Ipv6Address::Scope curScope = curAddr.getScope();
        if (curScope > retScope) {
            retAddr = curAddr;
            retScope = curScope;
            ret = true;
        }
    }
    return ret;
#endif // ifndef WITH_IPv6
}

bool L3AddressResolver::getMacAddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces() == 0)
        throw cRuntimeError("L3AddressResolver: interface table `%s' has no interface registered "
                            "(yet? try in a later init stage!)", ift->getFullPath().c_str());

    // choose first usable interface address (configured for generic, non-loopback if, addr non-null)
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        if (getInterfaceMacAddress(retAddr, ie, netmask))
            return true;
    }
    return false;
}

bool L3AddressResolver::getModulePathAddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces() == 0)
        throw cRuntimeError("L3AddressResolver: interface table `%s' has no interface registered "
                            "(yet? try in a later init stage!)", ift->getFullPath().c_str());

    // choose first usable interface address (configured for generic, non-loopback if, addr non-null)
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        if (getInterfaceModulePathAddress(retAddr, ie, netmask))
            return true;
    }
    return false;
}

bool L3AddressResolver::getModuleIdAddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask)
{
    if (ift->getNumInterfaces() == 0)
        throw cRuntimeError("L3AddressResolver: interface table `%s' has no interface registered "
                            "(yet? try in a later init stage!)", ift->getFullPath().c_str());

    // choose first usable interface address (configured for generic, non-loopback if, addr non-null)
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        if (getInterfaceModuleIdAddress(retAddr, ie, netmask))
            return true;
    }
    return false;
}

bool L3AddressResolver::getInterfaceIpv6Address(L3Address& ret, InterfaceEntry *ie, bool netmask)
{
#ifdef WITH_IPv6
    if (netmask)
        return false; // Ipv6 netmask not supported yet
    if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>()) {
        Ipv6Address addr = ipv6Data->getPreferredAddress();
        if (!addr.isUnspecified()) {
            ret.set(addr);
            return true;
        }
    }
#endif // ifdef WITH_IPv6
    return false;
}

bool L3AddressResolver::getInterfaceIpv4Address(L3Address& ret, InterfaceEntry *ie, bool netmask)
{
#ifdef WITH_IPv4
    if (auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>()) {
        Ipv4Address addr = ipv4Data->getIPAddress();
        if (!addr.isUnspecified()) {
            ret.set(netmask ? ipv4Data->getNetmask() : addr);
            return true;
        }
    }
    else {
        // find address in the configurator's notebook
        // TODO: how do we know where is the configurator? get the path from a NED parameter?
        L3AddressResolver *configurator = dynamic_cast<L3AddressResolver *>(getSimulation()->getModuleByPath("configurator"));
        if (configurator)
            return configurator->getInterfaceIpv4Address(ret, ie, netmask);
    }
#endif // ifdef WITH_IPv4
    return false;
}

bool L3AddressResolver::getInterfaceMacAddress(L3Address& ret, InterfaceEntry *ie, bool netmask)
{
    if (!ie->getMacAddress().isUnspecified()) {
        ret = ie->getMacAddress();
        return true;
    }
    return false;
}

bool L3AddressResolver::getInterfaceModulePathAddress(L3Address& ret, InterfaceEntry *ie, bool netmask)
{
    ret = ie->getModulePathAddress();
    return true;
}

bool L3AddressResolver::getInterfaceModuleIdAddress(L3Address& ret, InterfaceEntry *ie, bool netmask)
{
    ret = ie->getModuleIdAddress();
    return true;
}

IInterfaceTable *L3AddressResolver::interfaceTableOf(cModule *host)
{
    // find IInterfaceTable
    cModule *mod = host->getSubmodule("interfaceTable");
    if (!mod)
        throw cRuntimeError("L3AddressResolver: IInterfaceTable not found as submodule "
                            " `interfaceTable' in host/router `%s'", host->getFullPath().c_str());

    return check_and_cast<IInterfaceTable *>(mod);
}

IIpv4RoutingTable *L3AddressResolver::getIpv4RoutingTableOf(cModule *host)
{
    IIpv4RoutingTable *mod = findIpv4RoutingTableOf(host);
    if (!mod)
        throw cRuntimeError("L3AddressResolver: IIpv4RoutingTable not found as submodule "
                            " `routingTable' in host/router `%s'", host->getFullPath().c_str());
    return mod;
}

Ipv6RoutingTable *L3AddressResolver::getIpv6RoutingTableOf(cModule *host)
{
    // find Ipv6RoutingTable
    Ipv6RoutingTable *mod = findIpv6RoutingTableOf(host);
    if (!mod)
        throw cRuntimeError("L3AddressResolver: Ipv6RoutingTable not found as submodule "
                            " `routingTable' in host/router `%s'", host->getFullPath().c_str());
    return mod;
}

IInterfaceTable *L3AddressResolver::findInterfaceTableOf(cModule *host)
{
    return dynamic_cast<IInterfaceTable *>(host->getSubmodule("interfaceTable"));
}

IIpv4RoutingTable *L3AddressResolver::findIpv4RoutingTableOf(cModule *host)
{
#ifdef WITH_IPv4
    return dynamic_cast<IIpv4RoutingTable *>(host->getModuleByPath(".ipv4.routingTable"));
#else // ifdef WITH_IPv4
    return nullptr;
#endif // ifdef WITH_IPv4
}

Ipv6RoutingTable *L3AddressResolver::findIpv6RoutingTableOf(cModule *host)
{
#ifdef WITH_IPv6
    return dynamic_cast<Ipv6RoutingTable *>(host->getModuleByPath(".ipv6.routingTable"));
#else // ifdef WITH_IPv6
    return nullptr;
#endif // ifdef WITH_IPv6
}

NextHopRoutingTable *L3AddressResolver::findNextHopRoutingTableOf(cModule *host)
{
#ifdef WITH_NEXTHOP
    return dynamic_cast<NextHopRoutingTable *>(host->getModuleByPath(".generic.routingTable"));
#else // ifdef WITH_NEXTHOP
    return nullptr;
#endif // ifdef WITH_NEXTHOP
}

std::vector<cModule*> L3AddressResolver::collectNetworkNodes()
{
    std::vector<cModule*> result;
    doCollectNetworkNodes(getSimulation()->getSystemModule(), result);
    return result;
}

void L3AddressResolver::doCollectNetworkNodes(cModule *parent, std::vector<cModule*>& result)
{
    for (cModule::SubmoduleIterator it(parent); !it.end(); ++it) {
        cModule *submodule = *it;
        if (submodule->getProperties()->getAsBool("networkNode"))
            result.push_back(submodule);
        else
            doCollectNetworkNodes(submodule, result);
    }
}

cModule *L3AddressResolver::findHostWithAddress(const L3Address& add)
{
    if (add.isUnspecified() || add.isMulticast())
        return nullptr;

    auto networkNodes = collectNetworkNodes();
    for (cModule *mod : networkNodes) {
        IInterfaceTable *itable = L3AddressResolver().findInterfaceTableOf(mod);
        if (itable != nullptr) {
            for (int i = 0; i < itable->getNumInterfaces(); i++) {
                InterfaceEntry *entry = itable->getInterface(i);
                switch (add.getType()) {
#ifdef WITH_IPv6
                    case L3Address::IPv6: {
                        auto protocolData = entry->findProtocolData<Ipv6InterfaceData>();
                        if (protocolData != nullptr && protocolData->hasAddress(add.toIpv6()))
                            return mod;
                        break;
                    }

#endif // ifdef WITH_IPv6
#ifdef WITH_IPv4
                    case L3Address::IPv4: {
                        auto protocolData = entry->findProtocolData<Ipv4InterfaceData>();
                        if (protocolData != nullptr && protocolData->getIPAddress() == add.toIpv4())
                            return mod;
                        break;
                    }

#endif // ifdef WITH_IPv4
                    case L3Address::MAC:
                        if (entry->getMacAddress() == add.toMac())
                            return mod;
                        break;
                    default:
                        (void)entry;    // eliminate warning: unused variable 'entry'
                        throw cRuntimeError("findHostWithAddress() doesn't accept AddressType '%s', yet", L3Address::getTypeName(add.getType()));
                        break;
                }
            }
        }
    }
    return nullptr;
}

InterfaceEntry *L3AddressResolver::findInterfaceWithMacAddress(const MacAddress& addr)
{
    if (addr.isUnspecified() || addr.isBroadcast() || addr.isMulticast())
        return nullptr;

    auto networkNodes = collectNetworkNodes();
    for (cModule *mod : networkNodes) {
        IInterfaceTable *itable = L3AddressResolver().findInterfaceTableOf(mod);
        if (itable != nullptr) {
            for (int i = 0; i < itable->getNumInterfaces(); i++) {
                InterfaceEntry *entry = itable->getInterface(i);
                if (entry->getMacAddress() == addr)
                    return entry;
            }
        }
    }
    return nullptr;
}

cModule *L3AddressResolver::findHostWithMacAddress(const MacAddress& addr)
{
    InterfaceEntry *entry = findInterfaceWithMacAddress(addr);
    return entry ? entry->getInterfaceTable()->getHostModule() : nullptr;
}

} // namespace inet

