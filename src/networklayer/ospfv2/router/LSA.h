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

#ifndef __LSA_HPP__
#define __LSA_HPP__

#include "OSPFPacket_m.h"
#include "OSPFcommon.h"
#include <vector>
#include <math.h>

namespace OSPF {

struct NextHop {
    unsigned char ifIndex;
    IPv4Address   hopAddress;
    RouterID      advertisingRouter;
};

class RoutingInfo
{
private:
    std::vector<NextHop>  nextHops;
    unsigned long         distance;
    OSPFLSA*              parent;

public:
    RoutingInfo() : distance(0), parent(NULL) {}
    RoutingInfo(const RoutingInfo& routingInfo) : nextHops(routingInfo.nextHops), distance(routingInfo.distance), parent(routingInfo.parent) {}
    virtual ~RoutingInfo() {}

    void            addNextHop(NextHop nextHop)  { nextHops.push_back(nextHop); }
    void            clearNextHops()  { nextHops.clear(); }
    unsigned int    getNextHopCount() const  { return nextHops.size(); }
    NextHop         getNextHop(unsigned int index) const  { return nextHops[index]; }
    void            setDistance(unsigned long d)  { distance = d; }
    unsigned long   getDistance() const  { return distance; }
    void            setParent(OSPFLSA* p)  { parent = p; }
    OSPFLSA*        getParent() const  { return parent; }
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

    void            setSource(InstallSource installSource)  { source = installSource; }
    InstallSource   getSource() const  { return source; }
    void            incrementInstallTime()  { installTime++; }
    void            resetInstallTime()  { installTime = 0; }
    unsigned long   getInstallTime() const  { return installTime; }
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

    bool  validateLSChecksum() const  { return true; } // not implemented

    bool  update(const OSPFRouterLSA* lsa);
    bool  differsFrom(const OSPFRouterLSA* routerLSA) const;
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

    bool  validateLSChecksum() const  { return true; } // not implemented

    bool  update(const OSPFNetworkLSA* lsa);
    bool  differsFrom(const OSPFNetworkLSA* networkLSA) const;
};

class SummaryLSA : public OSPFSummaryLSA,
                   public RoutingInfo,
                   public LSATrackingInfo
{
protected:
    bool    purgeable;
public:
    SummaryLSA() : OSPFSummaryLSA(), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    SummaryLSA(const OSPFSummaryLSA& lsa) : OSPFSummaryLSA(lsa), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    SummaryLSA(const SummaryLSA& lsa) : OSPFSummaryLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~SummaryLSA() {}

    bool  getPurgeable() const  { return purgeable; }
    void  setPurgeable(bool purge = true)  { purgeable = purge; }

    bool  validateLSChecksum() const  { return true; } // not implemented

    bool  update(const OSPFSummaryLSA* lsa);
    bool  differsFrom(const OSPFSummaryLSA* summaryLSA) const;
};

class ASExternalLSA : public OSPFASExternalLSA,
                      public RoutingInfo,
                      public LSATrackingInfo
{
protected:
    bool    purgeable;
public:
    ASExternalLSA() : OSPFASExternalLSA(), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    ASExternalLSA(const OSPFASExternalLSA& lsa) : OSPFASExternalLSA(lsa), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    ASExternalLSA(const ASExternalLSA& lsa) : OSPFASExternalLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~ASExternalLSA() {}

    bool  getPurgeable() const  { return purgeable; }
    void  setPurgeable(bool purge = true)  { purgeable = purge; }

    bool  validateLSChecksum() const  { return true; } // not implemented

    bool  update(const OSPFASExternalLSA* lsa);
    bool  differsFrom(const OSPFASExternalLSA* asExternalLSA) const;
};

} // namespace OSPF

/**
 * Returns true if leftLSA is older than rightLSA.
 */
inline bool operator<(const OSPFLSAHeader& leftLSA, const OSPFLSAHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsSequenceNumber();

    if (leftSequenceNumber < rightSequenceNumber) {
        return true;
    }
    if (leftSequenceNumber == rightSequenceNumber) {
        unsigned short leftChecksum = leftLSA.getLsChecksum();
        unsigned short rightChecksum = rightLSA.getLsChecksum();

        if (leftChecksum < rightChecksum) {
            return true;
        }
        if (leftChecksum == rightChecksum) {
            unsigned short leftAge = leftLSA.getLsAge();
            unsigned short rightAge = rightLSA.getLsAge();

            if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE)) {
                return true;
            }
            if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Returns true if leftLSA is the same age as rightLSA.
 */
inline bool operator==(const OSPFLSAHeader& leftLSA, const OSPFLSAHeader& rightLSA)
{
    long           leftSequenceNumber = leftLSA.getLsSequenceNumber();
    long           rightSequenceNumber = rightLSA.getLsSequenceNumber();
    unsigned short leftChecksum = leftLSA.getLsChecksum();
    unsigned short rightChecksum = rightLSA.getLsChecksum();
    unsigned short leftAge = leftLSA.getLsAge();
    unsigned short rightAge = rightLSA.getLsAge();

    if ((leftSequenceNumber == rightSequenceNumber) &&
        (leftChecksum == rightChecksum) &&
        (((leftAge == MAX_AGE) && (rightAge == MAX_AGE)) ||
         (((leftAge != MAX_AGE) && (rightAge != MAX_AGE)) &&
          (abs(leftAge - rightAge) <= MAX_AGE_DIFF))))
    {
        return true;
    }
    else {
        return false;
    }
}

inline bool operator==(const OSPFOptions& leftOptions, const OSPFOptions& rightOptions)
{
    return ((leftOptions.E_ExternalRoutingCapability == rightOptions.E_ExternalRoutingCapability) &&
            (leftOptions.MC_MulticastForwarding == rightOptions.MC_MulticastForwarding) &&
            (leftOptions.NP_Type7LSA == rightOptions.NP_Type7LSA) &&
            (leftOptions.EA_ForwardExternalLSAs == rightOptions.EA_ForwardExternalLSAs) &&
            (leftOptions.DC_DemandCircuits == rightOptions.DC_DemandCircuits));
}

inline bool operator!=(const OSPFOptions& leftOptions, const OSPFOptions& rightOptions)
{
    return (!(leftOptions == rightOptions));
}

inline bool operator==(const OSPF::NextHop& leftHop, const OSPF::NextHop& rightHop)
{
    return ((leftHop.ifIndex == rightHop.ifIndex) &&
            (leftHop.hopAddress == rightHop.hopAddress) &&
            (leftHop.advertisingRouter == rightHop.advertisingRouter));
}

inline bool operator!=(const OSPF::NextHop& leftHop, const OSPF::NextHop& rightHop)
{
    return (!(leftHop == rightHop));
}

inline unsigned int calculateLSASize(const OSPFRouterLSA* routerLSA)
{
    unsigned int   lsaLength = OSPF_LSA_HEADER_LENGTH + OSPF_ROUTERLSA_HEADER_LENGTH;
    unsigned short linkCount = routerLSA->getLinksArraySize();

    for (unsigned short i = 0; i < linkCount; i++) {
        const Link& link = routerLSA->getLinks(i);
        lsaLength += OSPF_LINK_HEADER_LENGTH + (link.getTosDataArraySize() * OSPF_TOS_LENGTH);
    }

    return lsaLength;
}

inline unsigned int calculateLSASize(const OSPFNetworkLSA* networkLSA)
{
    return (OSPF_LSA_HEADER_LENGTH + OSPF_NETWORKLSA_MASK_LENGTH +
            (networkLSA->getAttachedRoutersArraySize() * OSPF_NETWORKLSA_ADDRESS_LENGTH));
}

inline unsigned int calculateLSASize(const OSPFSummaryLSA* summaryLSA)
{
    return (OSPF_LSA_HEADER_LENGTH + OSPF_SUMMARYLSA_HEADER_LENGTH +
            (summaryLSA->getTosDataArraySize() * OSPF_TOS_LENGTH));
}

inline unsigned int calculateLSASize(const OSPFASExternalLSA* asExternalLSA)
{
    return (OSPF_LSA_HEADER_LENGTH + OSPF_ASEXTERNALLSA_HEADER_LENGTH +
            (asExternalLSA->getContents().getExternalTOSInfoArraySize() * OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH));
}

inline void printLSAHeader(const OSPFLSAHeader& lsaHeader, std::ostream& output) {
    char addressString[16];
    output << "LSAHeader: age="
           << lsaHeader.getLsAge()
           << ", type=";
    switch (lsaHeader.getLsType()) {
        case ROUTERLSA_TYPE:                     output << "RouterLSA";                     break;
        case NETWORKLSA_TYPE:                    output << "NetworkLSA";                    break;
        case SUMMARYLSA_NETWORKS_TYPE:           output << "SummaryLSA_Networks";           break;
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:  output << "SummaryLSA_ASBoundaryRouters";  break;
        case AS_EXTERNAL_LSA_TYPE:                 output << "ASExternalLSA";                 break;
        default:                                output << "Unknown";                       break;
    }
    output << ", LSID="
           << addressStringFromULong(addressString, sizeof(addressString), lsaHeader.getLinkStateID());
    output << ", advertisingRouter="
           << addressStringFromULong(addressString, sizeof(addressString), lsaHeader.getAdvertisingRouter().getInt())
           << ", seqNumber="
           << lsaHeader.getLsSequenceNumber();
    output << endl;
}

inline std::ostream& operator<<(std::ostream& ostr, OSPFLSA& lsa)
{
    printLSAHeader(lsa.getHeader(), ostr);
    return ostr;
}

#endif // __LSA_HPP__

