//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#ifndef __INET_LSA_H
#define __INET_LSA_H

#include <math.h>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"
#include "inet/routing/ospfv2/OspfPacket_m.h"

namespace inet {

namespace ospf {

struct NextHop
{
    int ifIndex;
    Ipv4Address hopAddress;
    RouterId advertisingRouter;
};

class INET_API RoutingInfo
{
  private:
    std::vector<NextHop> nextHops;
    unsigned long distance;
    OspfLsa *parent;

  public:
    RoutingInfo() : distance(0), parent(nullptr) {}
    RoutingInfo(const RoutingInfo& routingInfo) : nextHops(routingInfo.nextHops), distance(routingInfo.distance), parent(routingInfo.parent) {}
    virtual ~RoutingInfo() {}

    void addNextHop(NextHop nextHop) { nextHops.push_back(nextHop); }
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
    void setDistance(unsigned long d) { distance = d; }
    unsigned long getDistance() const { return distance; }
    void setParent(OspfLsa *p) { parent = p; }
    OspfLsa *getParent() const { return parent; }
};

class INET_API LsaTrackingInfo
{
  public:
    enum InstallSource {
        ORIGINATED = 0,
        FLOODED = 1
    };

  private:
    InstallSource source;
    unsigned long installTime;

  public:
    LsaTrackingInfo() : source(FLOODED), installTime(0) {}
    LsaTrackingInfo(const LsaTrackingInfo& info) : source(info.source), installTime(info.installTime) {}

    void setSource(InstallSource installSource) { source = installSource; }
    InstallSource getSource() const { return source; }
    void incrementInstallTime() { installTime++; }
    void resetInstallTime() { installTime = 0; }
    unsigned long getInstallTime() const { return installTime; }
};

class INET_API RouterLsa : public OspfRouterLsa,
    public RoutingInfo,
    public LsaTrackingInfo
{
  public:
    RouterLsa() : OspfRouterLsa(), RoutingInfo(), LsaTrackingInfo() {}
    RouterLsa(const OspfRouterLsa& lsa) : OspfRouterLsa(lsa), RoutingInfo(), LsaTrackingInfo() {}
    RouterLsa(const RouterLsa& lsa) : OspfRouterLsa(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
    virtual ~RouterLsa() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OspfRouterLsa *lsa);
    bool differsFrom(const OspfRouterLsa *routerLSA) const;
};

class INET_API NetworkLsa : public OspfNetworkLsa,
    public RoutingInfo,
    public LsaTrackingInfo
{
  public:
    NetworkLsa() : OspfNetworkLsa(), RoutingInfo(), LsaTrackingInfo() {}
    NetworkLsa(const OspfNetworkLsa& lsa) : OspfNetworkLsa(lsa), RoutingInfo(), LsaTrackingInfo() {}
    NetworkLsa(const NetworkLsa& lsa) : OspfNetworkLsa(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
    virtual ~NetworkLsa() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OspfNetworkLsa *lsa);
    bool differsFrom(const OspfNetworkLsa *networkLSA) const;
};

class INET_API SummaryLsa : public OspfSummaryLsa,
    public RoutingInfo,
    public LsaTrackingInfo
{
  protected:
    bool purgeable;

  public:
    SummaryLsa() : OspfSummaryLsa(), RoutingInfo(), LsaTrackingInfo(), purgeable(false) {}
    SummaryLsa(const OspfSummaryLsa& lsa) : OspfSummaryLsa(lsa), RoutingInfo(), LsaTrackingInfo(), purgeable(false) {}
    SummaryLsa(const SummaryLsa& lsa) : OspfSummaryLsa(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~SummaryLsa() {}

    bool getPurgeable() const { return purgeable; }
    void setPurgeable(bool purge = true) { purgeable = purge; }

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OspfSummaryLsa *lsa);
    bool differsFrom(const OspfSummaryLsa *summaryLSA) const;
};

class INET_API AsExternalLsa : public OspfAsExternalLsa,
    public RoutingInfo,
    public LsaTrackingInfo
{
  protected:
    bool purgeable;

  public:
    AsExternalLsa() : OspfAsExternalLsa(), RoutingInfo(), LsaTrackingInfo(), purgeable(false) {}
    AsExternalLsa(const OspfAsExternalLsa& lsa) : OspfAsExternalLsa(lsa), RoutingInfo(), LsaTrackingInfo(), purgeable(false) {}
    AsExternalLsa(const AsExternalLsa& lsa) : OspfAsExternalLsa(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~AsExternalLsa() {}

    bool getPurgeable() const { return purgeable; }
    void setPurgeable(bool purge = true) { purgeable = purge; }

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OspfAsExternalLsa *lsa);
    bool differsFrom(const OspfAsExternalLsa *asExternalLSA) const;
};

/**
 * Returns true if leftLSA is older than rightLSA.
 */
bool operator<(const OspfLsaHeader& leftLSA, const OspfLsaHeader& rightLSA);

/**
 * Returns true if leftLSA is the same age as rightLSA.
 */
bool operator==(const OspfLsaHeader& leftLSA, const OspfLsaHeader& rightLSA);

bool operator==(const OspfOptions& leftOptions, const OspfOptions& rightOptions);

inline bool operator!=(const OspfOptions& leftOptions, const OspfOptions& rightOptions)
{
    return !(leftOptions == rightOptions);
}

inline bool operator==(const NextHop& leftHop, const NextHop& rightHop)
{
    return (leftHop.ifIndex == rightHop.ifIndex) &&
           (leftHop.hopAddress == rightHop.hopAddress) &&
           (leftHop.advertisingRouter == rightHop.advertisingRouter);
}

inline bool operator!=(const NextHop& leftHop, const NextHop& rightHop)
{
    return !(leftHop == rightHop);
}

B calculateLSASize(const OspfRouterLsa *routerLSA);
B calculateLSASize(const OspfNetworkLsa *networkLSA);
B calculateLSASize(const OspfSummaryLsa *summaryLSA);
B calculateLSASize(const OspfAsExternalLsa *asExternalLSA);

std::ostream& operator<<(std::ostream& ostr, const OspfLsaHeader& lsa);
inline std::ostream& operator<<(std::ostream& ostr, const OspfLsa& lsa) { ostr << lsa.getHeader(); return ostr; }
std::ostream& operator<<(std::ostream& ostr, const OspfNetworkLsa& lsa);
std::ostream& operator<<(std::ostream& ostr, const TosData& tos);
std::ostream& operator<<(std::ostream& ostr, const Link& link);
std::ostream& operator<<(std::ostream& ostr, const OspfRouterLsa& lsa);
std::ostream& operator<<(std::ostream& ostr, const OspfSummaryLsa& lsa);
std::ostream& operator<<(std::ostream& ostr, const ExternalTosInfo& tos);
std::ostream& operator<<(std::ostream& ostr, const OspfAsExternalLsa& lsa);

} // namespace ospf

} // namespace inet

#endif    // __LSA_HPP__

