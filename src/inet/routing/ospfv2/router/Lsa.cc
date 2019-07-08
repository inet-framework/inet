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
    switch(lsa->getHeader().getLsType()) {
        case LsaType::ROUTERLSA_TYPE: {
            auto routerLsa = check_and_cast<const OspfRouterLsa*>(lsa);
            return calculateLsaSize(*routerLsa);
        }
        case LsaType::NETWORKLSA_TYPE: {
            auto networkLsa = check_and_cast<const OspfNetworkLsa*>(lsa);
            return calculateLsaSize(*networkLsa);
        }
        case LsaType::SUMMARYLSA_NETWORKS_TYPE:
        case LsaType::SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            auto summaryLsa = check_and_cast<const OspfSummaryLsa*>(lsa);
            return calculateLsaSize(*summaryLsa);
        }
        case LsaType::AS_EXTERNAL_LSA_TYPE: {
            auto asExternalLsa = check_and_cast<const OspfAsExternalLsa*>(lsa);
            return calculateLsaSize(*asExternalLsa);
        }
        default:
            throw cRuntimeError("Unknown LsaType value: %d", (int)lsa->getHeader().getLsType());
            break;
    }
}

B calculateLsaSize(const OspfRouterLsa& lsa)
{
    B lsaLength = OSPF_LSA_HEADER_LENGTH + OSPF_ROUTERLSA_HEADER_LENGTH;
    for (uint32_t i = 0; i < lsa.getLinksArraySize(); i++) {
        const Link& link = lsa.getLinks(i);
        lsaLength += OSPF_LINK_HEADER_LENGTH + (OSPF_TOS_LENGTH * link.getTosDataArraySize());
    }
    return lsaLength;
}

B calculateLsaSize(const OspfNetworkLsa& lsa)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_NETWORKLSA_MASK_LENGTH
            + (OSPF_NETWORKLSA_ADDRESS_LENGTH * lsa.getAttachedRoutersArraySize());
}

B calculateLsaSize(const OspfSummaryLsa& lsa)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_SUMMARYLSA_HEADER_LENGTH
           + (OSPF_TOS_LENGTH * lsa.getTosDataArraySize());
}

B calculateLsaSize(const OspfAsExternalLsa& lsa)
{
    return OSPF_LSA_HEADER_LENGTH + OSPF_ASEXTERNALLSA_HEADER_LENGTH
           + (OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH * lsa.getContents().getExternalTOSInfoArraySize());
}

std::ostream& operator<<(std::ostream& ostr, const LsaRequest& request)
{
    ostr << "type=" << request.lsType
         << ", LSID=" << request.linkStateID
         << ", advertisingRouter=" << request.advertisingRouter;
    return ostr;
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
    ostr << lsa.getHeader();
    ostr << ", numAttachedRouters: " << cnt;
    if (cnt) {
        ostr << ", Attached routers: {";
        for (unsigned int i = 0; i < cnt; i++) {
            if (i)
                ostr << ", ";
            ostr << lsa.getAttachedRouters(i);
        }
        ostr << "}";
    }
    ostr << ", Mask: " << lsa.getNetworkMask();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const TosData& tos)
{
    ostr << "tos: " << (int)tos.tos
         << "metric: " << tos.tosMetric;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Link& link)
{
    ostr << "ID: " << link.getLinkID().str(false)
         << ", data: ";
    unsigned long data = link.getLinkData();
    if ((data & 0xFF000000) != 0)
        ostr << Ipv4Address(data);
    else
        ostr << data;
    ostr << ", type=";
    switch (link.getType()) {
        case POINTTOPOINT_LINK:
            ostr << "PointToPoint";
            break;

        case TRANSIT_LINK:
            ostr << "Transit";
            break;

        case STUB_LINK:
            ostr << "Stub";
            break;

        case VIRTUAL_LINK:
            ostr << "Virtual";
            break;

        default:
            ostr << "Unknown";
            break;
    }
    ostr << ", cost: " << link.getLinkCost();
    unsigned int cnt = link.getTosDataArraySize();
    if (cnt) {
        ostr << ", tos: {";
        for (unsigned int i = 0; i < cnt; i++) {
            if (i)
                ostr << ", ";
            ostr << link.getTosData(i);
        }
        ostr << "}";
    }
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OspfRouterLsa& lsa)
{
    ostr << lsa.getHeader()
         << ", flags: "
         << (lsa.getV_VirtualLinkEndpoint() ? "V" : "_")
         << (lsa.getE_ASBoundaryRouter() ? "E" : "_")
         << (lsa.getB_AreaBorderRouter() ? "B" : "_")
         << ", numberOfLinks: " << lsa.getNumberOfLinks() << ", ";
    unsigned int cnt = lsa.getLinksArraySize();
    if (cnt) {
        ostr << "Links: {";
        for (unsigned int i = 0; i < cnt; i++) {
            if (i)
                ostr << ",";
            ostr << " {" << lsa.getLinks(i) << "}";
        }
        ostr << "}";
    }
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OspfSummaryLsa& lsa)
{
    ostr << lsa.getHeader();
    ostr << ", Mask: " << lsa.getNetworkMask().str(false)
         << ", Cost: " << lsa.getRouteCost() << ", ";
    unsigned int cnt = lsa.getTosDataArraySize();
    if (cnt) {
        ostr << ", tosData: {";
        for (unsigned int i = 0; i < cnt; i++) {
            if (i)
                ostr << ", ";
            ostr << lsa.getTosData(i);
        }
        ostr << "}";
    }
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

std::ostream& operator<<(std::ostream& ostr, const OspfAsExternalLsaContents& contents)
{
    ostr << "Mask: " << contents.getNetworkMask()
         << ", ";
    unsigned int cnt = contents.getExternalTOSInfoArraySize();
    if (cnt) {
        ostr << ", tosData: {";
        for (unsigned int i = 0; i < cnt; i++) {
            if (i)
                ostr << ", ";
            ostr << contents.getExternalTOSInfo(i);
        }
        ostr << "}";
    }
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OspfAsExternalLsa& lsa)
{
    ostr << lsa.getHeader() << lsa.getContents();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const OspfLsa& lsa)
{
    ASSERT(lsa.getHeader().getLsCrcMode() != CrcMode::CRC_MODE_UNDEFINED);
    ASSERT(lsa.getHeader().getLsaLength() != 0);

    switch (lsa.getHeader().getLsType()) {
        case ROUTERLSA_TYPE:
            ostr << *check_and_cast<const RouterLsa *>(&lsa);
            break;

        case NETWORKLSA_TYPE:
            ostr << *check_and_cast<const NetworkLsa *>(&lsa);
            break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            ostr << *check_and_cast<const SummaryLsa *>(&lsa);
            break;

        case AS_EXTERNAL_LSA_TYPE:
            ostr << *check_and_cast<const AsExternalLsa *>(&lsa);
            break;

        default:
            ostr << lsa.getHeader();
            ostr << ", Unknown";
            break;
    }
    return ostr;
}

} // namespace ospf
} // namespace inet

