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

namespace ospfv2 {

bool operator<(const Ospfv2LsaHeader& leftLSA, const Ospfv2LsaHeader& rightLSA)
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

bool operator==(const Ospfv2LsaHeader& leftLSA, const Ospfv2LsaHeader& rightLSA)
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

bool operator==(const Ospfv2Options& leftOptions, const Ospfv2Options& rightOptions)
{
    return (leftOptions.E_ExternalRoutingCapability == rightOptions.E_ExternalRoutingCapability) &&
           (leftOptions.MC_MulticastForwarding == rightOptions.MC_MulticastForwarding) &&
           (leftOptions.NP_Type7LSA == rightOptions.NP_Type7LSA) &&
           (leftOptions.EA_ForwardExternalLSAs == rightOptions.EA_ForwardExternalLSAs) &&
           (leftOptions.DC_DemandCircuits == rightOptions.DC_DemandCircuits);
}

B calculateLSASize(const Ospfv2Lsa *lsa)
{
    switch(lsa->getHeader().getLsType()) {
        case Ospfv2LsaType::ROUTERLSA_TYPE: {
            auto routerLsa = check_and_cast<const Ospfv2RouterLsa*>(lsa);
            return calculateLsaSize(*routerLsa);
        }
        case Ospfv2LsaType::NETWORKLSA_TYPE: {
            auto networkLsa = check_and_cast<const Ospfv2NetworkLsa*>(lsa);
            return calculateLsaSize(*networkLsa);
        }
        case Ospfv2LsaType::SUMMARYLSA_NETWORKS_TYPE:
        case Ospfv2LsaType::SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            auto summaryLsa = check_and_cast<const Ospfv2SummaryLsa*>(lsa);
            return calculateLsaSize(*summaryLsa);
        }
        case Ospfv2LsaType::AS_EXTERNAL_LSA_TYPE: {
            auto asExternalLsa = check_and_cast<const Ospfv2AsExternalLsa*>(lsa);
            return calculateLsaSize(*asExternalLsa);
        }
        default:
            throw cRuntimeError("Unknown LsaType value: %d", (int)lsa->getHeader().getLsType());
            break;
    }
}

B calculateLsaSize(const Ospfv2RouterLsa& lsa)
{
    B lsaLength = OSPFv2_LSA_HEADER_LENGTH + OSPFv2_ROUTERLSA_HEADER_LENGTH;
    for (uint32_t i = 0; i < lsa.getLinksArraySize(); i++) {
        const auto& link = lsa.getLinks(i);
        lsaLength += OSPFv2_LINK_HEADER_LENGTH + (OSPFv2_TOS_LENGTH * link.getTosDataArraySize());
    }
    return lsaLength;
}

B calculateLsaSize(const Ospfv2NetworkLsa& lsa)
{
    return OSPFv2_LSA_HEADER_LENGTH + OSPFv2_NETWORKLSA_MASK_LENGTH
            + (OSPFv2_NETWORKLSA_ADDRESS_LENGTH * lsa.getAttachedRoutersArraySize());
}

B calculateLsaSize(const Ospfv2SummaryLsa& lsa)
{
    return OSPFv2_LSA_HEADER_LENGTH + OSPFv2_SUMMARYLSA_HEADER_LENGTH
           + (OSPFv2_TOS_LENGTH * lsa.getTosDataArraySize());
}

B calculateLsaSize(const Ospfv2AsExternalLsa& lsa)
{
    return OSPFv2_LSA_HEADER_LENGTH + OSPFv2_ASEXTERNALLSA_HEADER_LENGTH
           + (OSPFv2_ASEXTERNALLSA_TOS_INFO_LENGTH * lsa.getContents().getExternalTOSInfoArraySize());
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv2LsaRequest& request)
{
    ostr << "type=" << request.lsType
         << ", LSID=" << request.linkStateID
         << ", advertisingRouter=" << request.advertisingRouter;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv2LsaHeader& lsaHeader)
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

std::ostream& operator<<(std::ostream& ostr, const Ospfv2NetworkLsa& lsa)
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

std::ostream& operator<<(std::ostream& ostr, const Ospfv2TosData& tos)
{
    ostr << "tos: " << (int)tos.tos
         << "metric: " << tos.tosMetric;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv2Link& link)
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

std::ostream& operator<<(std::ostream& ostr, const Ospfv2RouterLsa& lsa)
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

std::ostream& operator<<(std::ostream& ostr, const Ospfv2SummaryLsa& lsa)
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

std::ostream& operator<<(std::ostream& ostr, const Ospfv2ExternalTosInfo& tos)
{
    ostr << "Tos: {" << tos.tos
         << "}, MetricType: " << tos.E_ExternalMetricType
         << ", fwAddr: " << tos.forwardingAddress
         << ", extRouteTag: " << tos.externalRouteTag;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv2AsExternalLsaContents& contents)
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

std::ostream& operator<<(std::ostream& ostr, const Ospfv2AsExternalLsa& lsa)
{
    ostr << lsa.getHeader() << lsa.getContents();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const Ospfv2Lsa& lsa)
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

} // namespace ospfv2
} // namespace inet

