//
// Copyright (C) 2016
// Author: Alfonso Ariza
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

#ifndef __INET_DYMOMULTICASTROUTESET_H
#define __INET_DYMOMULTICASTROUTESET_H

#include <omnetpp.h>
#include "inet/common/Ptr.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/dymo/DymoDefs.h"
#include "inet/routing/dymo/Dymo_m.h"


namespace inet {

namespace dymo {

class INET_API DymoSets : public cObject
{
  private:
    struct OriginatorAddressPair {
        L3Address prefixAddr;
        int orPrefixLen;
        bool operator<(const OriginatorAddressPair& other) const {
            if (orPrefixLen != other.orPrefixLen)
                return (orPrefixLen < other.orPrefixLen);
            else
                return prefixAddr < other.prefixAddr;
        };
        bool operator>(const OriginatorAddressPair& other) const { return other < *this; };
        bool operator==(const OriginatorAddressPair& other) const
        {
            return ((orPrefixLen == other.orPrefixLen) && (prefixAddr == other.prefixAddr));
        };
        bool operator!=(const OriginatorAddressPair& other) const {
            return ((orPrefixLen != other.orPrefixLen) || (prefixAddr != other.prefixAddr));
        }
    };

    struct MulticastRouteInfo{
        DymoSequenceNumber orSequenceNumber;
        L3Address target;
        DymoSequenceNumber trSequenceNumber;
        bool hasMetricType;
        DymoMetricType mType;
        double metric;
        simtime_t timeStamp;
        simtime_t removeTime;
    };

    struct ErrorRouteInfo{
        L3Address source;
        L3Address unreachableAddress;
        simtime_t timeout;
        bool operator<(const ErrorRouteInfo& other) const {
            if (source < other.source)
                return true;
            else
                return unreachableAddress < other.unreachableAddress;
        };
        bool operator>(const ErrorRouteInfo& other) const { return other < *this; };
        bool operator==(const ErrorRouteInfo& other) const
        {
            return ((source == other.source) && (unreachableAddress == other.unreachableAddress));
        };
        bool operator!=(const ErrorRouteInfo& other) const {
            return ((source != other.source) || (unreachableAddress != other.unreachableAddress));
        }
    };

    bool active = true;

    simtime_t lifetime = 300;
    simtime_t rerrTimeout = 3;
    // Is it necessary? it shouldn't be more than 1 element in this vector
    typedef std::vector <MulticastRouteInfo> MulticastRouteVect;
    typedef std::map<OriginatorAddressPair,MulticastRouteVect> MulticastRouteSet;
    typedef std::set<ErrorRouteInfo> ErrorRouteSet;

    MulticastRouteSet multicastRouteSet;
    ErrorRouteSet errorRouteSet;
    L3Address selfAddress;


    bool checkRREQ(const Ptr<const Rreq>&);
    bool checkRERR(const Ptr<const Rerr>&);

  public:
    DymoSets();
    virtual ~DymoSets() {multicastRouteSet.clear();}

    virtual void setLifeTime(const simtime_t &v) {lifetime = v;}
    virtual simtime_t getLifeTime() {return lifetime;}

    virtual void setRerrTimeout(const simtime_t &v) {rerrTimeout = v;}
    virtual simtime_t getRerrTimeout() {return rerrTimeout;}

    virtual void setSelftAddress(const L3Address & add){selfAddress = add;}
    virtual L3Address getSlftAddress() {return selfAddress;}

    virtual void setActive(const bool &v) {active = v;}
    virtual bool getActive() {return active;}

    virtual void clear() {multicastRouteSet.clear();}
    virtual bool check(const Ptr<const RteMsg>& rteMsg);
};

} // namespace dymo

} // namespace inet

#endif // ifndef __INET_DYMOMULTICASTROUTESET_H

