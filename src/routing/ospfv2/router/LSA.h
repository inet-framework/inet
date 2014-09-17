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

#include "inet/routing/ospfv2/router/OSPFcommon.h"
#include "inet/routing/ospfv2/OSPFPacket_m.h"

namespace inet {

namespace ospf {

struct NextHop
{
    int ifIndex;
    IPv4Address hopAddress;
    RouterID advertisingRouter;
};

class RoutingInfo
{
  private:
    std::vector<NextHop> nextHops;
    unsigned long distance;
    OSPFLSA *parent;

  public:
    RoutingInfo() : distance(0), parent(NULL) {}
    RoutingInfo(const RoutingInfo& routingInfo) : nextHops(routingInfo.nextHops), distance(routingInfo.distance), parent(routingInfo.parent) {}
    virtual ~RoutingInfo() {}

    void addNextHop(NextHop nextHop) { nextHops.push_back(nextHop); }
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
    void setDistance(unsigned long d) { distance = d; }
    unsigned long getDistance() const { return distance; }
    void setParent(OSPFLSA *p) { parent = p; }
    OSPFLSA *getParent() const { return parent; }
};

class LSATrackingInfo
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
    LSATrackingInfo() : source(FLOODED), installTime(0) {}
    LSATrackingInfo(const LSATrackingInfo& info) : source(info.source), installTime(info.installTime) {}

    void setSource(InstallSource installSource) { source = installSource; }
    InstallSource getSource() const { return source; }
    void incrementInstallTime() { installTime++; }
    void resetInstallTime() { installTime = 0; }
    unsigned long getInstallTime() const { return installTime; }
};

class RouterLSA : public OSPFRouterLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  public:
    RouterLSA() : OSPFRouterLSA(), RoutingInfo(), LSATrackingInfo() {}
    RouterLSA(const OSPFRouterLSA& lsa) : OSPFRouterLSA(lsa), RoutingInfo(), LSATrackingInfo() {}
    RouterLSA(const RouterLSA& lsa) : OSPFRouterLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa) {}
    virtual ~RouterLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFRouterLSA *lsa);
    bool differsFrom(const OSPFRouterLSA *routerLSA) const;
};

class NetworkLSA : public OSPFNetworkLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  public:
    NetworkLSA() : OSPFNetworkLSA(), RoutingInfo(), LSATrackingInfo() {}
    NetworkLSA(const OSPFNetworkLSA& lsa) : OSPFNetworkLSA(lsa), RoutingInfo(), LSATrackingInfo() {}
    NetworkLSA(const NetworkLSA& lsa) : OSPFNetworkLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa) {}
    virtual ~NetworkLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFNetworkLSA *lsa);
    bool differsFrom(const OSPFNetworkLSA *networkLSA) const;
};

class SummaryLSA : public OSPFSummaryLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  protected:
    bool purgeable;

  public:
    SummaryLSA() : OSPFSummaryLSA(), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    SummaryLSA(const OSPFSummaryLSA& lsa) : OSPFSummaryLSA(lsa), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    SummaryLSA(const SummaryLSA& lsa) : OSPFSummaryLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~SummaryLSA() {}

    bool getPurgeable() const { return purgeable; }
    void setPurgeable(bool purge = true) { purgeable = purge; }

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFSummaryLSA *lsa);
    bool differsFrom(const OSPFSummaryLSA *summaryLSA) const;
};

class ASExternalLSA : public OSPFASExternalLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  protected:
    bool purgeable;

  public:
    ASExternalLSA() : OSPFASExternalLSA(), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    ASExternalLSA(const OSPFASExternalLSA& lsa) : OSPFASExternalLSA(lsa), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    ASExternalLSA(const ASExternalLSA& lsa) : OSPFASExternalLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~ASExternalLSA() {}

    bool getPurgeable() const { return purgeable; }
    void setPurgeable(bool purge = true) { purgeable = purge; }

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFASExternalLSA *lsa);
    bool differsFrom(const OSPFASExternalLSA *asExternalLSA) const;
};

/**
 * Returns true if leftLSA is older than rightLSA.
 */
bool operator<(const OSPFLSAHeader& leftLSA, const OSPFLSAHeader& rightLSA);

/**
 * Returns true if leftLSA is the same age as rightLSA.
 */
bool operator==(const OSPFLSAHeader& leftLSA, const OSPFLSAHeader& rightLSA);

bool operator==(const OSPFOptions& leftOptions, const OSPFOptions& rightOptions);

inline bool operator!=(const OSPFOptions& leftOptions, const OSPFOptions& rightOptions)
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

unsigned int calculateLSASize(const OSPFRouterLSA *routerLSA);
unsigned int calculateLSASize(const OSPFNetworkLSA *networkLSA);
unsigned int calculateLSASize(const OSPFSummaryLSA *summaryLSA);
unsigned int calculateLSASize(const OSPFASExternalLSA *asExternalLSA);

std::ostream& operator<<(std::ostream& ostr, const OSPFLSAHeader& lsa);
inline std::ostream& operator<<(std::ostream& ostr, const OSPFLSA& lsa) { ostr << lsa.getHeader(); return ostr; }
std::ostream& operator<<(std::ostream& ostr, const OSPFNetworkLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const TOSData& tos);
std::ostream& operator<<(std::ostream& ostr, const Link& link);
std::ostream& operator<<(std::ostream& ostr, const OSPFRouterLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const OSPFSummaryLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const ExternalTOSInfo& tos);
std::ostream& operator<<(std::ostream& ostr, const OSPFASExternalLSA& lsa);

} // namespace ospf

} // namespace inet

#endif    // __LSA_HPP__

