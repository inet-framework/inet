#include "inet/routing/ospfv3/process/OSPFv3LSA.h"

namespace inet{

OSPFv3SPFVertex::OSPFv3SPFVertex(OSPFv3LSA* asocLSA, int distance)
{
    this->distance = distance;
    this->asocLSA = asocLSA;
    this->type = asocLSA->getHeader().getLsaType();
    this->vertexID.routerID = asocLSA->getHeader().getLinkStateID();

    if(type==NETWORK_LSA)
    {
        this->vertexID.interfaceID = 1;
    }
}

unsigned int calculateLSASize(const OSPFv3LSA *lsaC)
{
    auto lsa = lsaC->dup(); // make editable copy of lsa
    //OSPFv3LSAType lsaType = static_cast<OSPFv3LSAType>(lsa->getHeader().getLsaType());
    uint16_t code = lsa->getHeader().getLsaType();
    unsigned int lsaLength;

    switch(code){
        case ROUTER_LSA:
        {
            OSPFv3RouterLSA* routerLSA = dynamic_cast<OSPFv3RouterLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_ROUTER_LSA_HEADER_LENGTH;
            unsigned short linkCount = routerLSA->getRoutersArraySize();
            lsaLength += linkCount*OSPFV3_ROUTER_LSA_BODY_LENGTH;

            break;
        }
        case NETWORK_LSA:
        {
            OSPFv3NetworkLSA* networkLSA = dynamic_cast<OSPFv3NetworkLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_NETWORK_LSA_HEADER_LENGTH;
            unsigned short attachedCount = networkLSA->getAttachedRouterArraySize();
            lsaLength += attachedCount*OSPFV3_NETWORK_LSA_ATTACHED_LENGTH;

            break;
        }
        case INTER_AREA_PREFIX_LSA:
        {
            OSPFv3InterAreaPrefixLSA* prefixLSA = dynamic_cast<OSPFv3InterAreaPrefixLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH + OSPFV3_INTER_AREA_PREFIX_LSA_BODY_LENGTH;

            break;
        }
        case LINK_LSA:
        {
            OSPFv3LinkLSA* linkLSA = dynamic_cast<OSPFv3LinkLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_LINK_LSA_BODY_LENGTH;
            unsigned short linkCount = linkLSA->getNumPrefixes();
            lsaLength += linkCount*OSPFV3_LINK_LSA_PREFIX_LENGTH;

            break;
        }
        case INTRA_AREA_PREFIX_LSA:
        {
            OSPFv3IntraAreaPrefixLSA* prefixLSA = dynamic_cast<OSPFv3IntraAreaPrefixLSA* >(lsa);
            lsaLength = OSPFV3_LSA_HEADER_LENGTH + OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH;
            unsigned short prefixCount = prefixLSA->getNumPrefixes();
            lsaLength += prefixCount*OSPFV3_INTRA_AREA_PREFIX_LSA_PREFIX_LENGTH;

            break;
        }
    }

    return lsaLength;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3LSAHeader& lsaHeader)
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

std::ostream& operator<<(std::ostream& ostr, const OSPFv3NetworkLSA& lsa)
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

std::ostream& operator<<(std::ostream& ostr, const OSPFv3RouterLSA& lsa)
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

std::ostream& operator<<(std::ostream& ostr, const OSPFv3InterAreaPrefixLSA& lsa)
{
    ostr << "Mask: " << lsa.getPrefixLen()
         << ", Cost: " << lsa.getMetric() << ", ";
    ostr << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFv3ASExternalLSA& lsa)
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

}//namespace inet
