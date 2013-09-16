//
// Copyright (C) 2005 Andras Varga
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


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <algorithm>
//#include <sstream>

#include "InterfaceEntry.h"

#include "IInterfaceTable.h"

#ifdef WITH_IPv4
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#endif


void InterfaceProtocolData::changed(int category)
{
    // notify the containing InterfaceEntry that something changed
    if (ownerp)
        ownerp->changed(category);
}


InterfaceEntry::InterfaceEntry(cModule* ifmod)
{
    ownerp = NULL;
    interfaceModule = ifmod;

    nwLayerGateIndex = -1;
    nodeOutputGateId = -1;
    nodeInputGateId = -1;

    mtu = 0;

    state = UP;
    carrier = true;
    broadcast = false;
    multicast = false;
    pointToPoint = false;
    loopback = false;
    datarate = 0;

    ipv4data = NULL;
    ipv6data = NULL;
    isisdata = NULL;
    trilldata = NULL;
    vlandata = NULL;
    estimateCostProcessArray.clear();
}

InterfaceEntry::~InterfaceEntry()
{
    resetInterface();
}

std::string InterfaceEntry::info() const
{
    std::stringstream out;
    out << (getName()[0] ? getName() : "*");
    if (getNetworkLayerGateIndex()==-1)
        out << "  on:-";
    else
        out << "  on:nwLayer.ifOut[" << getNetworkLayerGateIndex() << "]";
    out << "  MTU:" << getMTU();
    if (!isUp()) out << " DOWN";
    if (isBroadcast()) out << " BROADCAST";
    if (isMulticast()) out << " MULTICAST";
    if (isPointToPoint()) out << " POINTTOPOINT";
    if (isLoopback()) out << " LOOPBACK";
    out << "  macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();

#ifdef WITH_IPv4
    if (ipv4data)
        out << " " << ipv4data->info();
#endif
#ifdef WITH_IPv6
    if (ipv6data)
        out << " " << ipv6data->info();
#endif
    if (isisdata)
        out << " " << ((InterfaceProtocolData *)isisdata)->info(); // Khmm...
    if (trilldata)
        out << " " << ((InterfaceProtocolData *)trilldata)->info(); // Khmm...
    if (vlandata)
        out << " " << ((InterfaceProtocolData *)vlandata)->info() << "\n"; // Khmm...
    return out.str();
}

std::string InterfaceEntry::detailedInfo() const
{
    std::stringstream out;
    out << "name:" << (getName()[0] ? getName() : "*");
    if (getNetworkLayerGateIndex()==-1)
        out << "  on:-";
    else
        out << "  on:nwLayer.ifOut[" << getNetworkLayerGateIndex() << "]";
    out << "MTU: " << getMTU() << " \t";
    if (!isUp()) out << "DOWN ";
    if (isBroadcast()) out << "BROADCAST ";
    if (isMulticast()) out << "MULTICAST ";
    if (isPointToPoint()) out << "POINTTOPOINT ";
    if (isLoopback()) out << "LOOPBACK ";
    out << "\n";
    out << "  macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();
    out << "\n";
#ifdef WITH_IPv4
    if (ipv4data)
        out << " " << ipv4data->detailedInfo() << "\n";
#endif
#ifdef WITH_IPv6
    if (ipv6data)
        out << " " << ipv6data->detailedInfo() << "\n";
#endif
    if (isisdata)
        out << " " << ((InterfaceProtocolData *)isisdata)->detailedInfo() << "\n"; // Khmm...
    if (trilldata)
        out << " " << ((InterfaceProtocolData *)trilldata)->detailedInfo() << "\n"; // Khmm...
    if (vlandata)
        out << " " << ((InterfaceProtocolData *)vlandata)->detailedInfo() << "\n"; // Khmm...
    return out.str();
}
std::string InterfaceEntry::getFullPath() const
{
    return ownerp == NULL ? getFullName() : ownerp->getHostModule()->getFullPath() + "." + getFullName();
}

void InterfaceEntry::changed(int category)
{
    if (ownerp)
        ownerp->interfaceChanged(this, category);
}

void InterfaceEntry::resetInterface()
{
#ifdef WITH_IPv4
    if (ipv4data && ipv4data->ownerp == this)
        delete ipv4data;
    ipv4data = NULL;
#else
    if (ipv4data)
        throw cRuntimeError(this, "Model error: ipv4data filled, but INET was compiled without IPv4 support");
#endif
#ifdef WITH_IPv6
    if (ipv6data && ipv6data->ownerp == this)
        delete ipv6data;
    ipv6data = NULL;
#else
    if (ipv6data)
        throw cRuntimeError(this, "Model error: ipv6data filled, but INET was compiled without IPv6 support");
#endif
    if (isisdata && ((InterfaceProtocolData *)isisdata)->ownerp == this)
        delete (InterfaceProtocolData *)isisdata;
    isisdata = NULL;
    if (trilldata && ((InterfaceProtocolData *)trilldata)->ownerp == this)
        delete (InterfaceProtocolData *)trilldata;
    trilldata = NULL;
    if (vlandata && ((InterfaceProtocolData *)vlandata)->ownerp == this)
        delete (InterfaceProtocolData *)vlandata;
    vlandata = NULL;
}

void InterfaceEntry::setIPv4Data(IPv4InterfaceData *p)
{
#ifdef WITH_IPv4
    if (ipv4data && ipv4data->ownerp == this)
        delete ipv4data;
    ipv4data = p;
    p->ownerp = this;
    configChanged();
#else
    throw cRuntimeError(this, "setIPv4Data(): INET was compiled without IPv4 support");
#endif
}

void InterfaceEntry::setIPv6Data(IPv6InterfaceData *p)
{
#ifdef WITH_IPv6
    if (ipv6data && ipv6data->ownerp == this)
        delete ipv6data;
    ipv6data = p;
    p->ownerp = this;
    configChanged();
#else
    throw cRuntimeError(this, "setIPv6Data(): INET was compiled without IPv6 support");
#endif
}

void InterfaceEntry::setTRILLInterfaceData(TRILLInterfaceData *p)
{
    if (trilldata && ((InterfaceProtocolData *)trilldata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)trilldata; // Khmm...
    trilldata = p;
    ((InterfaceProtocolData*)p)->ownerp = this; // Khmm...
    configChanged();
}

void InterfaceEntry::setISISInterfaceData(ISISInterfaceData *p)
{
    if (isisdata && ((InterfaceProtocolData *)isisdata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)isisdata; // Khmm...
    isisdata = p;
    ((InterfaceProtocolData*)p)->ownerp = this; // Khmm...
    configChanged();
}

void InterfaceEntry::setVLANInterfaceData(VLANInterfaceData *p)
{
    if (vlandata && ((InterfaceProtocolData *)vlandata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)vlandata; // Khmm...
    vlandata = p;
    ((InterfaceProtocolData*)p)->ownerp = this; // Khmm...
    configChanged();
}

bool InterfaceEntry::setEstimateCostProcess(int position, MacEstimateCostProcess *p)
{
    ASSERT(position >= 0);
    if (estimateCostProcessArray.size() <= (size_t)position)
    {
        estimateCostProcessArray.resize(position+1, NULL);
    }
    if (estimateCostProcessArray[position]!=NULL)
        return false;
    estimateCostProcessArray[position] = p;
    return true;
}

MacEstimateCostProcess* InterfaceEntry::getEstimateCostProcess(int position)
{
    ASSERT(position >= 0);
    if ((size_t)position < estimateCostProcessArray.size())
    {
        return estimateCostProcessArray[position];
    }
    return NULL;
}

