#ifndef __INET_OSPFV3LSA_H_
#define __INET_OSPFV3LSA_H_

#include <omnetpp.h>

#include "inet/routing/ospfv3/OSPFv3Common.h"
#include "inet/routing/ospfv3/OSPFv3Packet_m.h"
#include "inet/common/INETDefs.h"

// NexHop is defined in OSPFv3Common.h
// Every OSPFv3 class representing pakcet is extedned by Routing and Tracking info,
// which are needed for calculating SPF and wtchting time since LSA is written down into LSDB

namespace inet {

class INET_API LSATrackingInfo
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


class INET_API RoutingInfo
{
  private:
    std::vector<NextHop> nextHops;
    unsigned long distance;
    OSPFv3LSA *parent;

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
    void setParent(OSPFv3LSA *p) { parent = p; }
    OSPFv3LSA *getParent() const { return parent; }
};

class INET_API RouterLSA : public OSPFv3RouterLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    RouterLSA() : OSPFv3RouterLSA(), RoutingInfo() {}
    RouterLSA(const OSPFv3RouterLSA& lsa) : OSPFv3RouterLSA(lsa), RoutingInfo() {}
    RouterLSA(const RouterLSA& lsa) : OSPFv3RouterLSA(lsa), RoutingInfo(lsa) {}
    virtual ~RouterLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3RouterLSA *lsa);
    bool differsFrom(const OSPFv3RouterLSA *routerLSA) const;
};

class INET_API NetworkLSA : public OSPFv3NetworkLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    NetworkLSA() : OSPFv3NetworkLSA(), RoutingInfo() {}
    NetworkLSA(const OSPFv3NetworkLSA& lsa) : OSPFv3NetworkLSA(lsa), RoutingInfo() {}
    NetworkLSA(const NetworkLSA& lsa) : OSPFv3NetworkLSA(lsa), RoutingInfo(lsa) {}
    virtual ~NetworkLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3NetworkLSA *lsa);
    bool differsFrom(const OSPFv3NetworkLSA *networkLSA) const;
};

class INET_API InterAreaPrefixLSA : public OSPFv3InterAreaPrefixLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    InterAreaPrefixLSA() : OSPFv3InterAreaPrefixLSA(), RoutingInfo() {}
    InterAreaPrefixLSA(const OSPFv3InterAreaPrefixLSA& lsa) : OSPFv3InterAreaPrefixLSA(lsa), RoutingInfo() {}
    InterAreaPrefixLSA(const InterAreaPrefixLSA& lsa) : OSPFv3InterAreaPrefixLSA(lsa), RoutingInfo(lsa) {}
    virtual ~InterAreaPrefixLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3InterAreaPrefixLSA *lsa);
    bool differsFrom(const OSPFv3InterAreaPrefixLSA *interAreaPrefixLSA) const;
};

class INET_API InterAreaRouterLSA : public OSPFv3InterAreaRouterLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    InterAreaRouterLSA() : OSPFv3InterAreaRouterLSA(), RoutingInfo() {}
    InterAreaRouterLSA(const OSPFv3InterAreaRouterLSA& lsa) : OSPFv3InterAreaRouterLSA(lsa), RoutingInfo() {}
    InterAreaRouterLSA(const InterAreaRouterLSA& lsa) : OSPFv3InterAreaRouterLSA(lsa), RoutingInfo(lsa) {}
    virtual ~InterAreaRouterLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3IntraAreaPrefixLSA *lsa);
    bool differsFrom(const OSPFv3IntraAreaPrefixLSA *interAreaRouterLSA) const;
};

class INET_API ASExternalLSA : public OSPFv3ASExternalLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    ASExternalLSA() : OSPFv3ASExternalLSA(), RoutingInfo() {}
    ASExternalLSA(const OSPFv3ASExternalLSA& lsa) : OSPFv3ASExternalLSA(lsa), RoutingInfo() {}
    ASExternalLSA(const ASExternalLSA& lsa) : OSPFv3ASExternalLSA(lsa), RoutingInfo(lsa) {}
    virtual ~ASExternalLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3ASExternalLSA *lsa);
    bool differsFrom(const OSPFv3ASExternalLSA *aSExternalLSA) const;
};

class INET_API NssaLSA : public OSPFv3NssaLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    NssaLSA() : OSPFv3NssaLSA(), RoutingInfo() {}
    NssaLSA(const OSPFv3NssaLSA& lsa) : OSPFv3NssaLSA(lsa), RoutingInfo() {}
    NssaLSA(const NssaLSA& lsa) : OSPFv3NssaLSA(lsa), RoutingInfo(lsa) {}
    virtual ~NssaLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3NssaLSA *lsa);
    bool differsFrom(const OSPFv3NssaLSA *nssaLSA) const;
};

class INET_API LinkLSA : public OSPFv3LinkLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    LinkLSA() : OSPFv3LinkLSA(), RoutingInfo() {}
    LinkLSA(const OSPFv3LinkLSA& lsa) : OSPFv3LinkLSA(lsa), RoutingInfo() {}
    LinkLSA(const LinkLSA& lsa) : OSPFv3LinkLSA(lsa), RoutingInfo(lsa) {}
    virtual ~LinkLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3LinkLSA *lsa);
    bool differsFrom(const OSPFv3LinkLSA *linkLSA) const;
};

class INET_API IntraAreaPrefixLSA : public OSPFv3IntraAreaPrefixLSA, public RoutingInfo, public LSATrackingInfo
{
  public:
    IntraAreaPrefixLSA() : OSPFv3IntraAreaPrefixLSA(), RoutingInfo() {}
    IntraAreaPrefixLSA(const OSPFv3IntraAreaPrefixLSA& lsa) : OSPFv3IntraAreaPrefixLSA(lsa), RoutingInfo() {}
    IntraAreaPrefixLSA(const IntraAreaPrefixLSA& lsa) : OSPFv3IntraAreaPrefixLSA(lsa), RoutingInfo(lsa) {}
    virtual ~IntraAreaPrefixLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3IntraAreaPrefixLSA *lsa);
    bool differsFrom(const OSPFv3IntraAreaPrefixLSA *intraAreaPrefixLSA) const;
};

class INET_API OSPFv3SPFVertex
{
  private:
    VertexID vertexID;
    OSPFv3LSA* asocLSA;
    int distance; //link state cost of the current set of shortest paths from the root
    uint16_t type; //router or network lsa
    OSPFv3SPFVertex* parent = nullptr;


  public:
    OSPFv3SPFVertex(OSPFv3LSA* asocLSA, int distance);

};

unsigned int calculateLSASize(const OSPFv3LSA *lsaC);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3LSAHeader& lsa);
inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3LSA& lsa) { ostr << lsa.getHeader(); return ostr; }
std::ostream& operator<<(std::ostream& ostr, const OSPFv3NetworkLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3RouterLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3InterAreaPrefixLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3ASExternalLSA& lsa);

}//namespace inet

#endif
