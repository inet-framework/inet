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
    int localPreference = 0;
    bool IBGP_learned = false;

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
    int getLocalPreference(void) const { return localPreference; }
    void setLocalPreference(int l) { localPreference = l; }
    bool isIBgpLearned(void) { return IBGP_learned; }
    void setIBgpLearned(bool i) { IBGP_learned = i;}
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
    setAdminDist(dBGPExternal);
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
    if (entry.getDestination().isUnspecified())
        out << "0.0.0.0";
    else
        out << entry.getDestination();
    out << "/";
    if (entry.getNetmask().isUnspecified())
        out << "0";
    else
        out << entry.getNetmask().getNetmaskLength();
    out << " nextHop: " << entry.getGateway().str(false)
        << " cost: " << entry.getMetric()
        << " if: " << entry.getInterfaceName()
        << " origin: " << BgpRoutingTableEntry::getPathTypeString(entry.getPathType());
    if(entry.isIBgpLearned())
        out << " localPref: " << entry.getLocalPreference();
    out << " ASlist: ";
    for (uint32_t i = 0; i < entry.getASCount(); i++)
        out << entry.getAS(i) << ' ';

    return out;
}

inline std::string BgpRoutingTableEntry::str() const
{
    std::stringstream out;
    out << getSourceTypeAbbreviation();
    out << " ";
    if (getDestination().isUnspecified())
        out << "0.0.0.0";
    else
        out << getDestination();
    out << "/";
    if (getNetmask().isUnspecified())
        out << "0";
    else
        out << getNetmask().getNetmaskLength();
    out << " gw:";
    if (getGateway().isUnspecified())
        out << "*  ";
    else
        out << getGateway() << "  ";
    if(getRoutingTable() && getRoutingTable()->isAdminDistEnabled())
        out << "AD:" << getAdminDist() << "  ";
    out << "metric:" << getMetric() << "  ";
    out << "if:";
    if (!getInterface())
        out << "*";
    else
        out << getInterfaceName();

    out << " origin: " << BgpRoutingTableEntry::getPathTypeString(_pathType);
    if(IBGP_learned)
        out << " localPref: " << getLocalPreference();
    out << " ASlist: ";
    for (auto &element : _ASList)
        out << element << ' ';

    return out.str();
}

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTINGTABLEENTRY_H

