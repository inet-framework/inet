#include "inet/routing/ospfv3/process/Ospfv3Lsa.h"

namespace inet {
namespace ospfv3 {

Ospfv3SpfVertex::Ospfv3SpfVertex(Ospfv3Lsa *asocLSA, int distance)
{
    this->distance = distance;
    this->asocLSA = asocLSA;
    this->type = asocLSA->getHeader().getLsaType();
    this->vertexID.routerID = asocLSA->getHeader().getLinkStateID();

    if (type == NETWORK_LSA) {
        this->vertexID.interfaceID = 1;
    }
}

// for ROUTER_LSA
B calculateLSASize(const Ospfv3RouterLsa *routerLSA)
{
    return OSPFV3_LSA_HEADER_LENGTH + OSPFV3_ROUTER_LSA_HEADER_LENGTH +
           (OSPFV3_ROUTER_LSA_BODY_LENGTH * routerLSA->getRoutersArraySize());
}

// for NETWORK_LSA
B calculateLSASize(const Ospfv3NetworkLsa *networkLSA)
{
    return OSPFV3_LSA_HEADER_LENGTH + OSPFV3_NETWORK_LSA_HEADER_LENGTH +
           (OSPFV3_NETWORK_LSA_ATTACHED_LENGTH * networkLSA->getAttachedRouterArraySize());
}

// for  INTER_AREA_PREFIX_LSA
B calculateLSASize(const Ospfv3InterAreaPrefixLsa *prefixLSA)
{
//    return OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_BODY_LENGTH;
    return OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH + OSPFV3_LSA_PREFIX_HEADER_LENGTH + B(((prefixLSA->getPrefix().prefixLen + 31) / 32) * 4);
}

// for LINK_LSA
B calculateLSASize(const Ospfv3LinkLsa *linkLSA)
{
    B lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_LINK_LSA_BODY_LENGTH;
    uint32_t prefixCount = linkLSA->getNumPrefixes();
    for (uint32_t i = 0; i < prefixCount; i++) {
        lsaLength += OSPFV3_LSA_PREFIX_HEADER_LENGTH;
        lsaLength += B(((linkLSA->getPrefixes(i).prefixLen + 31) / 32) * 4);
    }
    return lsaLength;
}

// for INTRA_AREA_PREFIX_LSA
B calculateLSASize(const Ospfv3IntraAreaPrefixLsa *prefixLSA)
{
    B lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH;

    uint32_t prefixCount = prefixLSA->getNumPrefixes();
    for (uint32_t i = 0; i < prefixCount; i++) {
        lsaLength += OSPFV3_LSA_PREFIX_HEADER_LENGTH;
        lsaLength += B(((prefixLSA->getPrefixes(i).prefixLen + 31) / 32) * 4);
    }
    return lsaLength;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv3LsaHeader& lsaHeader)
{
    ostr << "LSAHeader: age=" << lsaHeader.getLsaAge()
         << ", type=";
    switch (lsaHeader.getLsaType()) {
        case ROUTER_LSA:
            ostr << "RouterLSA";
            break;

        case NETWORK_LSA:
            ostr << "NetworkLSA";
            break;

        case INTER_AREA_PREFIX_LSA:
            ostr << "InterAreaPrefixLSA";
            break;

        case INTER_AREA_ROUTER_LSA:
            ostr << "InterAreaRouterLSA";
            break;

        case AS_EXTERNAL_LSA:
            ostr << "ASExternalLSA";
            break;
        case LINK_LSA:
            ostr << "LinkLSA";
            break;
        case INTRA_AREA_PREFIX_LSA:
            ostr << "IntraAreaPrefixLSA";
            break;

        default:
            ostr << "Unknown";
            break;
    }
    ostr << ", LSID=" << lsaHeader.getLinkStateID().str(false)
         << ", advertisingRouter=" << lsaHeader.getAdvertisingRouter().str(false)
         << ", seqNumber=" << lsaHeader.getLsaSequenceNumber()
         << endl;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv3NetworkLsa& lsa)
{
    unsigned int cnt = lsa.getAttachedRouterArraySize();
    if (cnt) {
        ostr << ", Attached routers:";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " " << lsa.getAttachedRouter(i);
        }
    }
    ostr << ", " << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv3RouterLsa& lsa)
{
    if (lsa.getVBit())
        ostr << "V, ";
    if (lsa.getEBit())
        ostr << "E, ";
    if (lsa.getBBit())
        ostr << "B, ";
    ostr << "numberOfLinks: " << lsa.getRoutersArraySize() << ", ";
    unsigned int cnt = lsa.getRoutersArraySize();
    if (cnt) {
        ostr << "Links: {";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " {" << lsa.getRouters(i).neighborRouterID << "}";
        }
        ostr << "}, ";
    }
    ostr << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv3InterAreaPrefixLsa& lsa)
{
    ostr << "Mask: " << lsa.getPrefix().prefixLen
         << ", Cost: " << lsa.getMetric() << ", ";
    ostr << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv3AsExternalLsa& lsa)
{
    if (lsa.getEBit())
        ostr << "E, ";
    if (lsa.getTBit())
        ostr << "T, ";
    if (lsa.getFBit())
        ostr << "F, ";

    ostr << "Referenced LSA Type: " << lsa.getReferencedLSType()
         << ", Cost: " << lsa.getMetric()
         << ", Forward: " << lsa.getForwardingAddress()
         << ", ExtRouteTag: " << lsa.getExternalRouteTag()
         << ", Referenced LSID: " << lsa.getReferencedLSID()
         << ", ";

    ostr << lsa.getHeader();
    return ostr;
}

} // namespace ospfv3
} // namespace inet

