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

#include "inet/routing/ospfv2/router/LSA.h"

namespace inet {

namespace ospf {

bool operator<(const OSPFLSAHeader& leftLSA, const OSPFLSAHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsSequenceNumber();

    if (leftSequenceNumber < rightSequenceNumber) {
        return true;
    }
    if (leftSequenceNumber == rightSequenceNumber) {
        unsigned short leftAge = leftLSA.getLsAge();
        unsigned short rightAge = rightLSA.getLsAge();

        if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE)) {
            return true;
        }
        if ((leftAge == MAX_AGE) && (rightAge != MAX_AGE)) {
            return false;
        }
        if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge)) {
            return true;
        }
    }
    return false;
}

bool operator==(const OSPFLSAHeader& leftLSA, const OSPFLSAHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsSequenceNumber();
    unsigned short leftAge = leftLSA.getLsAge();
    unsigned short rightAge = rightLSA.getLsAge();

    if ((leftSequenceNumber == rightSequenceNumber) &&
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

bool operator==(const OSPFOptions& leftOptions, const OSPFOptions& rightOptions)
{
    return (leftOptions.E_ExternalRoutingCapability == rightOptions.E_ExternalRoutingCapability) &&
           (leftOptions.MC_MulticastForwarding == rightOptions.MC_MulticastForwarding) &&
           (leftOptions.NP_Type7LSA == rightOptions.NP_Type7LSA) &&
           (leftOptions.EA_ForwardExternalLSAs == rightOptions.EA_ForwardExternalLSAs) &&
           (leftOptions.DC_DemandCircuits == rightOptions.DC_DemandCircuits);
}

unsigned int calculateLSASize(const OSPFRouterLSA *routerLSA)
{
    unsigned int lsaLength = OSPF_LSA_HEADER_LENGTH + OSPF_ROUTERLSA_HEADER_LENGTH;
    unsigned short linkCount = routerLSA->getLinksArraySize();

    for (unsigned short i = 0; i < linkCount; i++) {
        const Link& link = routerLSA->getLinks(i);
        lsaLength += OSPF_LINK_HEADER_LENGTH + (link.getTosDataArraySize() * OSPF_TOS_LENGTH);
    }

    return lsaLength;
}

unsigned int calculateLSASize(const OSPFNetworkLSA *networkLSA)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_NETWORKLSA_MASK_LENGTH
           + (networkLSA->getAttachedRoutersArraySize() * OSPF_NETWORKLSA_ADDRESS_LENGTH);
}

unsigned int calculateLSASize(const OSPFSummaryLSA *summaryLSA)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_SUMMARYLSA_HEADER_LENGTH
           + (summaryLSA->getTosDataArraySize() * OSPF_TOS_LENGTH);
}

unsigned int calculateLSASize(const OSPFASExternalLSA *asExternalLSA)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_ASEXTERNALLSA_HEADER_LENGTH
           + (asExternalLSA->getContents().getExternalTOSInfoArraySize() * OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH);
}

std::ostream& operator<<(std::ostream& ostr, const OSPFLSAHeader& lsaHeader)
{
    ostr << "LSAHeader: age=" << lsaHeader.getLsAge()
         << ", type=";
    switch (lsaHeader.getLsType()) {
        case ROUTERLSA_TYPE:
            ostr << "RouterLSA";
            break;

        case NETWORKLSA_TYPE:
            ostr << "NetworkLSA";
            break;

        case SUMMARYLSA_NETWORKS_TYPE:
            ostr << "SummaryLSA_Networks";
            break;

        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            ostr << "SummaryLSA_ASBoundaryRouters";
            break;

        case AS_EXTERNAL_LSA_TYPE:
            ostr << "ASExternalLSA";
            break;

        default:
            ostr << "Unknown";
            break;
    }
    ostr << ", LSID=" << lsaHeader.getLinkStateID().str(false)
         << ", advertisingRouter=" << lsaHeader.getAdvertisingRouter().str(false)
         << ", seqNumber=" << lsaHeader.getLsSequenceNumber()
         << endl;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFNetworkLSA& lsa)
{
    ostr << "Mask: " << lsa.getNetworkMask();
    unsigned int cnt = lsa.getAttachedRoutersArraySize();
    if (cnt) {
        ostr << ", Attached routers:";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " " << lsa.getAttachedRouters(i);
        }
    }
    ostr << ", " << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const TOSData& tos)
{
    ostr << "tos: " << (int)tos.tos
         << "metric:";
    for (int i = 0; i < 3; i++)
        ostr << " " << (int)tos.tosMetric[i];
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Link& link)
{
    ostr << "ID: " << link.getLinkID().str(false)
         << ", data: ";
    unsigned long data = link.getLinkData();
    if ((data & 0xFF000000) != 0)
        ostr << IPv4Address(data).str(false);
    else
        ostr << data;
    ostr << ", cost: " << link.getLinkCost();
    unsigned int cnt = link.getTosDataArraySize();
    if (cnt) {
        ostr << ", tos: {";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " " << link.getTosData(i);
        }
        ostr << "}";
    }
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFRouterLSA& lsa)
{
    if (lsa.getV_VirtualLinkEndpoint())
        ostr << "V, ";
    if (lsa.getE_ASBoundaryRouter())
        ostr << "E, ";
    if (lsa.getB_AreaBorderRouter())
        ostr << "B, ";
    ostr << "numberOfLinks: " << lsa.getNumberOfLinks() << ", ";
    unsigned int cnt = lsa.getLinksArraySize();
    if (cnt) {
        ostr << "Links: {";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " {" << lsa.getLinks(i) << "}";
        }
        ostr << "}, ";
    }
    ostr << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFSummaryLSA& lsa)
{
    ostr << "Mask: " << lsa.getNetworkMask()
         << ", Cost: " << lsa.getRouteCost() << ", ";
    unsigned int cnt = lsa.getTosDataArraySize();
    if (cnt) {
        ostr << ", tosData: {";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " " << lsa.getTosData(i);
        }
        ostr << "}, ";
    }
    ostr << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const ExternalTOSInfo& tos)
{
    ostr << "TOSData: {" << tos.tosData
         << "}, MetricType: " << tos.E_ExternalMetricType
         << ", fwAddr: " << tos.forwardingAddress
         << ", extRouteTag: " << tos.externalRouteTag;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OSPFASExternalLSA& lsa)
{
    const OSPFASExternalLSAContents& contents = lsa.getContents();
    ostr << "Mask: " << contents.getNetworkMask()
         << ", Cost: " << contents.getRouteCost()
         << ", MetricType: " << contents.getE_ExternalMetricType()
         << ", Forward: " << contents.getForwardingAddress()
         << ", ExtRouteTag: " << contents.getExternalRouteTag()
         << ", ";
    unsigned int cnt = contents.getExternalTOSInfoArraySize();
    if (cnt) {
        ostr << ", tosData: {";
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << " " << contents.getExternalTOSInfo(i);
        }
        ostr << "}, ";
    }
    ostr << lsa.getHeader();
    return ostr;
}

} // namespace ospf

} // namespace inet

