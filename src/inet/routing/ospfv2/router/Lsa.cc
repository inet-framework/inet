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

#include "inet/routing/ospfv2/router/Lsa.h"

namespace inet {

namespace ospf {

bool operator<(const OspfLsaHeader& leftLSA, const OspfLsaHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsSequenceNumber();

    if (leftSequenceNumber > rightSequenceNumber)
        return false;
    else if (leftSequenceNumber < rightSequenceNumber)
        return true;
    else {
        // TODO: checksum comparison should be added here

        unsigned short leftAge = leftLSA.getLsAge();
        unsigned short rightAge = rightLSA.getLsAge();

        if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE))
            return true;
        else if ((leftAge == MAX_AGE) && (rightAge != MAX_AGE))
            return false;
        else if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge))
            return true;
    }
    return false;
}

bool operator==(const OspfLsaHeader& leftLSA, const OspfLsaHeader& rightLSA)
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

bool operator==(const OspfOptions& leftOptions, const OspfOptions& rightOptions)
{
    return (leftOptions.E_ExternalRoutingCapability == rightOptions.E_ExternalRoutingCapability) &&
           (leftOptions.MC_MulticastForwarding == rightOptions.MC_MulticastForwarding) &&
           (leftOptions.NP_Type7LSA == rightOptions.NP_Type7LSA) &&
           (leftOptions.EA_ForwardExternalLSAs == rightOptions.EA_ForwardExternalLSAs) &&
           (leftOptions.DC_DemandCircuits == rightOptions.DC_DemandCircuits);
}

B calculateLSASize(const OspfLsa *lsa)
{
    B lsaLength = OSPF_LSA_HEADER_LENGTH;
    switch(lsa->getHeader().getLsType()) {
        case LsaType::ROUTERLSA_TYPE: {
            lsaLength += OSPF_ROUTERLSA_HEADER_LENGTH;
            auto routerLsa = check_and_cast<const OspfRouterLsa*>(lsa);
            for (uint32_t i = 0; i < routerLsa->getLinksArraySize(); i++) {
                const Link& link = routerLsa->getLinks(i);
                lsaLength += OSPF_LINK_HEADER_LENGTH + (OSPF_TOS_LENGTH * link.getTosDataArraySize());
            }
            break;
        }
        case LsaType::NETWORKLSA_TYPE: {
            auto networkLsa = check_and_cast<const OspfNetworkLsa*>(lsa);
            lsaLength += OSPF_NETWORKLSA_MASK_LENGTH + (OSPF_NETWORKLSA_ADDRESS_LENGTH * networkLsa->getAttachedRoutersArraySize());
            break;
        }
        case LsaType::SUMMARYLSA_NETWORKS_TYPE:
        case LsaType::SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            auto summaryLSA = check_and_cast<const OspfSummaryLsa*>(lsa);
            lsaLength += OSPF_SUMMARYLSA_HEADER_LENGTH + (OSPF_TOS_LENGTH * summaryLSA->getTosDataArraySize());
            break;
        }
        case LsaType::AS_EXTERNAL_LSA_TYPE: {
            auto asExternalLSA = check_and_cast<const OspfAsExternalLsa*>(lsa);
            lsaLength += OSPF_ASEXTERNALLSA_HEADER_LENGTH + (OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH * asExternalLSA->getContents().getExternalTOSInfoArraySize());
            break;
        }
        default:
            throw cRuntimeError("Unknown LsaType value: %d", (int)lsa->getHeader().getLsType());
    }
    return lsaLength;
}

B calculateLSASize(const OspfSummaryLsa *summaryLSA)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_SUMMARYLSA_HEADER_LENGTH
           + (OSPF_TOS_LENGTH * summaryLSA->getTosDataArraySize());
}

B calculateLSASize(const OspfAsExternalLsa *asExternalLSA)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_ASEXTERNALLSA_HEADER_LENGTH
           + (OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH * asExternalLSA->getContents().getExternalTOSInfoArraySize());
}

std::ostream& operator<<(std::ostream& ostr, const OspfLsaHeader& lsaHeader)
{
    ostr << "LSAHeader (age: " << lsaHeader.getLsAge()
         << ", type: ";
    switch (lsaHeader.getLsType()) {
        case ROUTERLSA_TYPE:
            ostr << "RouterLsa";
            break;

        case NETWORKLSA_TYPE:
            ostr << "NetworkLsa";
            break;

        case SUMMARYLSA_NETWORKS_TYPE:
            ostr << "SummaryLSA_Networks";
            break;

        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            ostr << "SummaryLSA_ASBoundaryRouters";
            break;

        case AS_EXTERNAL_LSA_TYPE:
            ostr << "AsExternalLsa";
            break;

        default:
            ostr << "Unknown";
            break;
    }
    ostr << ", LSID: " << lsaHeader.getLinkStateID().str(false)
         << ", advertisingRouter: " << lsaHeader.getAdvertisingRouter().str(false)
         << ", seqNumber: " << lsaHeader.getLsSequenceNumber();
    ostr << ")";
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OspfNetworkLsa& lsa)
{
    unsigned int cnt = lsa.getAttachedRoutersArraySize();
    ostr << "numAttachedRouters: " << cnt << ", ";
    ostr << "Attached routers: ";
    if (cnt) {
        for (unsigned int i = 0; i < cnt; i++) {
            ostr << lsa.getAttachedRouters(i) << " ";
        }
    }
    ostr << ", Mask: " << lsa.getNetworkMask();
    ostr << ", " << lsa.getHeader();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const TosData& tos)
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
        ostr << Ipv4Address(data).str(false);
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

std::ostream& operator<<(std::ostream& ostr, const OspfRouterLsa& lsa)
{
    ostr << "flags: ";
    if (lsa.getV_VirtualLinkEndpoint())
        ostr << "V ";
    if (lsa.getE_ASBoundaryRouter())
        ostr << "E ";
    if (lsa.getB_AreaBorderRouter())
        ostr << "B ";
    ostr << ", numberOfLinks: " << lsa.getNumberOfLinks() << ", ";
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

std::ostream& operator<<(std::ostream& ostr, const OspfSummaryLsa& lsa)
{
    ostr << "Mask: " << lsa.getNetworkMask().str(false)
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

std::ostream& operator<<(std::ostream& ostr, const ExternalTosInfo& tos)
{
    ostr << "Tos: {" << tos.tos
         << "}, MetricType: " << tos.E_ExternalMetricType
         << ", fwAddr: " << tos.forwardingAddress
         << ", extRouteTag: " << tos.externalRouteTag;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OspfAsExternalLsa& lsa)
{
    const OspfAsExternalLsaContents& contents = lsa.getContents();
    ostr << "Mask: " << contents.getNetworkMask()
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

