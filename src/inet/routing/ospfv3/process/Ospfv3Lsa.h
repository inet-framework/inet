#ifndef __INET_OSPFV3LSA_H_
#define __INET_OSPFV3LSA_H_

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"

// NexHop is defined in Ospfv3Common.h
// Every Ospfv3 class representing pakcet is extedned by Routing and Tracking info,
// which are needed for calculating SPF and wtchting time since LSA is written down into LSDB

namespace inet {
namespace ospfv3 {

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
    Ospfv3Lsa *parent;

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
    void setParent(Ospfv3Lsa *p) { parent = p; }
    Ospfv3Lsa *getParent() const { return parent; }
};

class INET_API RouterLSA : public Ospfv3RouterLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    RouterLSA() : Ospfv3RouterLsa(), RoutingInfo() {}
    RouterLSA(const Ospfv3RouterLsa& lsa) : Ospfv3RouterLsa(lsa), RoutingInfo() {}
    RouterLSA(const RouterLSA& lsa) : Ospfv3RouterLsa(lsa), RoutingInfo(lsa) {}
    virtual ~RouterLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3RouterLsa *lsa);
    bool differsFrom(const Ospfv3RouterLsa *routerLSA) const;
};

class INET_API NetworkLSA : public Ospfv3NetworkLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    NetworkLSA() : Ospfv3NetworkLsa(), RoutingInfo() {}
    NetworkLSA(const Ospfv3NetworkLsa& lsa) : Ospfv3NetworkLsa(lsa), RoutingInfo() {}
    NetworkLSA(const NetworkLSA& lsa) : Ospfv3NetworkLsa(lsa), RoutingInfo(lsa) {}
    virtual ~NetworkLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3NetworkLsa *lsa);
    bool differsFrom(const Ospfv3NetworkLsa *networkLSA) const;
};

class INET_API InterAreaPrefixLSA : public Ospfv3InterAreaPrefixLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    InterAreaPrefixLSA() : Ospfv3InterAreaPrefixLsa(), RoutingInfo() {}
    InterAreaPrefixLSA(const Ospfv3InterAreaPrefixLsa& lsa) : Ospfv3InterAreaPrefixLsa(lsa), RoutingInfo() {}
    InterAreaPrefixLSA(const InterAreaPrefixLSA& lsa) : Ospfv3InterAreaPrefixLsa(lsa), RoutingInfo(lsa) {}
    virtual ~InterAreaPrefixLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3InterAreaPrefixLsa *lsa);
    bool differsFrom(const Ospfv3InterAreaPrefixLsa *interAreaPrefixLSA) const;
};

class INET_API InterAreaRouterLSA : public Ospfv3InterAreaRouterLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    InterAreaRouterLSA() : Ospfv3InterAreaRouterLsa(), RoutingInfo() {}
    InterAreaRouterLSA(const Ospfv3InterAreaRouterLsa& lsa) : Ospfv3InterAreaRouterLsa(lsa), RoutingInfo() {}
    InterAreaRouterLSA(const InterAreaRouterLSA& lsa) : Ospfv3InterAreaRouterLsa(lsa), RoutingInfo(lsa) {}
    virtual ~InterAreaRouterLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3IntraAreaPrefixLsa *lsa);
    bool differsFrom(const Ospfv3IntraAreaPrefixLsa *interAreaRouterLSA) const;
};

class INET_API ASExternalLSA : public Ospfv3AsExternalLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    ASExternalLSA() : Ospfv3AsExternalLsa(), RoutingInfo() {}
    ASExternalLSA(const Ospfv3AsExternalLsa& lsa) : Ospfv3AsExternalLsa(lsa), RoutingInfo() {}
    ASExternalLSA(const ASExternalLSA& lsa) : Ospfv3AsExternalLsa(lsa), RoutingInfo(lsa) {}
    virtual ~ASExternalLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3AsExternalLsa *lsa);
    bool differsFrom(const Ospfv3AsExternalLsa *aSExternalLSA) const;
};

class INET_API NssaLSA : public Ospfv3NssaLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    NssaLSA() : Ospfv3NssaLsa(), RoutingInfo() {}
    NssaLSA(const Ospfv3NssaLsa& lsa) : Ospfv3NssaLsa(lsa), RoutingInfo() {}
    NssaLSA(const NssaLSA& lsa) : Ospfv3NssaLsa(lsa), RoutingInfo(lsa) {}
    virtual ~NssaLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3NssaLsa *lsa);
    bool differsFrom(const Ospfv3NssaLsa *nssaLSA) const;
};

class INET_API LinkLSA : public Ospfv3LinkLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    LinkLSA() : Ospfv3LinkLsa(), RoutingInfo() {}
    LinkLSA(const Ospfv3LinkLsa& lsa) : Ospfv3LinkLsa(lsa), RoutingInfo() {}
    LinkLSA(const LinkLSA& lsa) : Ospfv3LinkLsa(lsa), RoutingInfo(lsa) {}
    virtual ~LinkLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3LinkLsa *lsa);
    bool differsFrom(const Ospfv3LinkLsa *linkLSA) const;
};

class INET_API IntraAreaPrefixLSA : public Ospfv3IntraAreaPrefixLsa, public RoutingInfo, public LSATrackingInfo
{
  public:
    IntraAreaPrefixLSA() : Ospfv3IntraAreaPrefixLsa(), RoutingInfo() {}
    IntraAreaPrefixLSA(const Ospfv3IntraAreaPrefixLsa& lsa) : Ospfv3IntraAreaPrefixLsa(lsa), RoutingInfo() {}
    IntraAreaPrefixLSA(const IntraAreaPrefixLSA& lsa) : Ospfv3IntraAreaPrefixLsa(lsa), RoutingInfo(lsa) {}
    virtual ~IntraAreaPrefixLSA() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const Ospfv3IntraAreaPrefixLsa *lsa);
    bool differsFrom(const Ospfv3IntraAreaPrefixLsa *intraAreaPrefixLSA) const;
};

class INET_API Ospfv3SpfVertex
{
  private:
    VertexID vertexID;
    Ospfv3Lsa* asocLSA;
    int distance; //link state cost of the current set of shortest paths from the root
    uint16_t type; //router or network lsa
    // Ospfv3SpfVertex* parent = nullptr;   //TODO unused field

  public:
    Ospfv3SpfVertex(Ospfv3Lsa* asocLSA, int distance);
};

//unsigned int calculateLSASize(const Ospfv3Lsa *lsaC);
B calculateLSASize(const Ospfv3RouterLsa *routerLSA);
B calculateLSASize(const Ospfv3NetworkLsa *networkLSA);
B calculateLSASize(const Ospfv3InterAreaPrefixLsa *prefixLSA);
B calculateLSASize(const Ospfv3LinkLsa *linkLSA);
B calculateLSASize(const Ospfv3IntraAreaPrefixLsa *prefixLSA);

std::ostream& operator<<(std::ostream& ostr, const Ospfv3LsaHeader& lsa);
inline std::ostream& operator<<(std::ostream& ostr, const Ospfv3Lsa& lsa) { ostr << lsa.getHeader(); return ostr; }
std::ostream& operator<<(std::ostream& ostr, const Ospfv3NetworkLsa& lsa);
std::ostream& operator<<(std::ostream& ostr, const Ospfv3RouterLsa& lsa);
std::ostream& operator<<(std::ostream& ostr, const Ospfv3InterAreaPrefixLsa& lsa);
std::ostream& operator<<(std::ostream& ostr, const Ospfv3AsExternalLsa& lsa);

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3LSA_H_

