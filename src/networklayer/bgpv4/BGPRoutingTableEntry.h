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

#include "RoutingTable.h"
#include "BGPCommon.h"

namespace BGP {

class INET_API RoutingTableEntry : public IPv4Route
{

public:
    typedef unsigned char RoutingPathType;

    RoutingTableEntry(void);
    RoutingTableEntry(const IPv4Route* entry);
    virtual ~RoutingTableEntry(void) {}

    void            setPathType(RoutingPathType type)               { _pathType = type; }
    RoutingPathType getPathType(void) const                         { return _pathType; }
    void            addAS(ASID newAS)                               { _ASList.push_back(newAS); }
    unsigned int    getASCount(void) const                          { return _ASList.size(); }
    ASID            getAS(unsigned int index) const                 { return _ASList[index]; }

    private:
    // destinationID is RoutingEntry::host
    // addressMask is RoutingEntry::netmask
    RoutingPathType         _pathType;
    std::vector<ASID>       _ASList;
};

} // namespace BGP

inline BGP::RoutingTableEntry::RoutingTableEntry(void) :
    IPv4Route(), _pathType(BGP::Incomplete)
{
    setNetmask(IPv4Address::ALLONES_ADDRESS);
    setMetric(BGP::DEFAULT_COST);
    setSource(IPv4Route::BGP);
}

inline BGP::RoutingTableEntry::RoutingTableEntry(const IPv4Route* entry)
{
    setDestination(entry->getDestination());
    setNetmask(entry->getNetmask());
    setGateway(entry->getGateway());
    setInterface(entry->getInterface());
    setMetric(BGP::DEFAULT_COST);
    setSource(IPv4Route::BGP);
}

inline std::ostream& operator<<(std::ostream& out, BGP::RoutingTableEntry& entry)
{
    out << "BGP - Destination: "
        << entry.getDestination().str()
        << '/'
        << entry.getNetmask().str()
        << " , PathType: ";
    switch (entry.getPathType()) {
        case BGP::EGP:          out << "EGP";           break;
        case BGP::IGP:          out << "IGP";           break;
        case BGP::Incomplete:   out << "Incomplete";    break;
        default:                out << "Unknown";       break;
    }

    out << " , NextHops: "
        << entry.getGateway()
        << " , AS: ";
    unsigned int ASCount = entry.getASCount();
    for (unsigned int i = 0; i < ASCount; i++) {
        out << entry.getAS(i)
            << ' ';
    }
    return out;
}

#endif

