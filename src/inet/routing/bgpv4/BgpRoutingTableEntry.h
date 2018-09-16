//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPROUTINGTABLEENTRY_H
#define __INET_BGPROUTINGTABLEENTRY_H

#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/routing/bgpv4/BgpCommon.h"

namespace inet {

namespace bgp {

class INET_API BgpRoutingTableEntry : public Ipv4Route
{
private:
    typedef unsigned char RoutingPathType;
    // destinationID is RoutingEntry::host
    // addressMask is RoutingEntry::netmask
    RoutingPathType _pathType = INCOMPLETE;
    std::vector<AsId> _ASList;

  public:
    BgpRoutingTableEntry(void);
    BgpRoutingTableEntry(const Ipv4Route *entry);
    virtual ~BgpRoutingTableEntry(void) {}

    void setPathType(RoutingPathType type) { _pathType = type; }
    RoutingPathType getPathType(void) const { return _pathType; }
    static const std::string getPathTypeString(RoutingPathType type);
    void addAS(AsId newAS) { _ASList.push_back(newAS); }
    unsigned int getASCount(void) const { return _ASList.size(); }
    AsId getAS(unsigned int index) const { return _ASList[index]; }
    virtual std::string str() const;
};

inline BgpRoutingTableEntry::BgpRoutingTableEntry(void)
{
    setNetmask(Ipv4Address::ALLONES_ADDRESS);
    setMetric(DEFAULT_COST);
    setSourceType(IRoute::BGP);
}

inline BgpRoutingTableEntry::BgpRoutingTableEntry(const Ipv4Route *entry)
{
    setDestination(entry->getDestination());
    setNetmask(entry->getNetmask());
    setGateway(entry->getGateway());
    setInterface(entry->getInterface());
    setMetric(DEFAULT_COST);
    setSourceType(IRoute::BGP);
}

inline const std::string BgpRoutingTableEntry::getPathTypeString(RoutingPathType type)
{
    if(type == IGP)
        return "IGP";
    else if(type == EGP)
        return "EGP";
    else if(type == INCOMPLETE)
        return "INCOMPLETE";

    return "Unknown";
}

inline std::ostream& operator<<(std::ostream& out, BgpRoutingTableEntry& entry)
{
    out << "dest: " << entry.getDestination().str(false)
        << " nextHop: " << entry.getGateway().str(false)
        << " mask: " << entry.getNetmask().str()
        << " cost: " << entry.getMetric()
        << " if: " << entry.getInterfaceName()
        << " pathType: " << BgpRoutingTableEntry::getPathTypeString(entry.getPathType())
        << " ASlist: ";
    for (uint32_t i = 0; i < entry.getASCount(); i++)
        out << entry.getAS(i) << ' ';

    return out;
}

inline std::string BgpRoutingTableEntry::str() const
{
    std::stringstream out;

    out << "dest:";
    if (getDestination().isUnspecified())
        out << "*  ";
    else
        out << getDestination() << "  ";
    out << "gw:";
    if (getGateway().isUnspecified())
        out << "*  ";
    else
        out << getGateway() << "  ";
    out << "mask:";
    if (getNetmask().isUnspecified())
        out << "*  ";
    else
        out << getNetmask() << "  ";
    if(getRoutingTable() && getRoutingTable()->isAdminDistEnabled())
        out << "AD:" << getAdminDist() << "  ";
    out << "metric:" << getMetric() << "  ";
    out << "if:";
    if (!getInterface())
        out << "*";
    else
        out << getInterface()->getInterfaceName();
    if (getInterface() && getInterface()->ipv4Data())
        out << "(" << getInterface()->getIpv4Address() << ")";
    out << "  ";
    out << (getGateway().isUnspecified() ? "DIRECT" : "REMOTE");
    out << " " << IRoute::sourceTypeName(getSourceType());

    out << " pathType: " << BgpRoutingTableEntry::getPathTypeString(_pathType)
    << " ASlist: ";
    for (auto &element : _ASList)
        out << element << ' ';

    return out.str();
}

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTINGTABLEENTRY_H

