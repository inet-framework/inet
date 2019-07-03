//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/ospfv2/OspfPacketSerializer.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"

namespace inet {

namespace ospf {

Register_Serializer(OspfPacket, OspfPacketSerializer);
Register_Serializer(OspfHelloPacket, OspfPacketSerializer);
Register_Serializer(OspfDatabaseDescriptionPacket, OspfPacketSerializer);
Register_Serializer(OspfLinkStateRequestPacket, OspfPacketSerializer);
Register_Serializer(OspfLinkStateUpdatePacket, OspfPacketSerializer);
Register_Serializer(OspfLinkStateAcknowledgementPacket, OspfPacketSerializer);

void OspfPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ospfPacket = staticPtrCast<const OspfPacket>(chunk);
    serializeOspfHeader(stream, ospfPacket);
    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            const auto& helloPacket = staticPtrCast<const OspfHelloPacket>(ospfPacket);
            stream.writeIpv4Address(helloPacket->getNetworkMask());
            stream.writeUint16Be(helloPacket->getHelloInterval());
            serializeOspfOptions(stream, helloPacket->getOptions());
            stream.writeByte(helloPacket->getRouterPriority());
            stream.writeUint32Be(helloPacket->getRouterDeadInterval());
            stream.writeIpv4Address(helloPacket->getDesignatedRouter());
            stream.writeIpv4Address(helloPacket->getBackupDesignatedRouter());
            for (size_t i = 0; i < helloPacket->getNeighborArraySize(); ++i) {
                stream.writeIpv4Address(helloPacket->getNeighbor(i));
            }
            break;
        }
        case DATABASE_DESCRIPTION_PACKET: {
            const auto& ddPacket = staticPtrCast<const OspfDatabaseDescriptionPacket>(ospfPacket);
            stream.writeUint16Be(ddPacket->getInterfaceMTU());
            serializeOspfOptions(stream, ddPacket->getOptions());
            auto& options = ddPacket->getDdOptions();
            stream.writeNBitsOfUint64Be(options.unused, 5);
            stream.writeBit(options.I_Init);
            stream.writeBit(options.M_More);
            stream.writeBit(options.MS_MasterSlave);
            stream.writeUint32Be(ddPacket->getDdSequenceNumber());
            for (unsigned int i = 0; i < ddPacket->getLsaHeadersArraySize(); ++i) {
                serializeLsaHeader(stream, ddPacket->getLsaHeaders(i));
            }
            break;
        }
        case LINKSTATE_REQUEST_PACKET: {
            const auto& requestPacket = staticPtrCast<const OspfLinkStateRequestPacket>(ospfPacket);
            for (size_t i = 0; i < requestPacket->getRequestsArraySize(); ++i) {
                auto& req = requestPacket->getRequests(i);
                stream.writeUint32Be(req.lsType);
                stream.writeIpv4Address(req.linkStateID);
                stream.writeIpv4Address(req.advertisingRouter);
            }
            break;
        }
        case LINKSTATE_UPDATE_PACKET: {
            const auto& updatePacket = staticPtrCast<const OspfLinkStateUpdatePacket>(ospfPacket);
            stream.writeUint32Be(updatePacket->getOspfLSAsArraySize());
            for (size_t i = 0; i < updatePacket->getOspfLSAsArraySize(); ++i) {
                serializeLsa(stream, *updatePacket->getOspfLSAs(i));
            }
            break;
        }
        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            const auto& ackPacket = staticPtrCast<const OspfLinkStateAcknowledgementPacket>(ospfPacket);
            for (size_t i = 0; i < ackPacket->getLsaHeadersArraySize(); ++i) {
                serializeLsaHeader(stream, ackPacket->getLsaHeaders(i));
            }
            break;
        }
        default:
            throw cRuntimeError("Unknown OSPF message type in OspfPacketSerializer");
    }
}

const Ptr<Chunk> OspfPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ospfPacket = makeShared<OspfPacket>();
    uint16_t packetLength = deserializeOspfHeader(stream, ospfPacket);
    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            auto helloPacket = makeShared<OspfHelloPacket>();
            copyHeaderFields(ospfPacket, helloPacket);
            helloPacket->setNetworkMask(stream.readIpv4Address());
            helloPacket->setHelloInterval(stream.readUint16Be());
            deserializeOspfOptions(stream, helloPacket->getOptionsForUpdate());
            helloPacket->setRouterPriority(stream.readByte());
            helloPacket->setRouterDeadInterval(stream.readUint32Be());
            helloPacket->setDesignatedRouter(stream.readIpv4Address());
            helloPacket->setBackupDesignatedRouter(stream.readIpv4Address());
            int numNeighbors = (B(packetLength) - OSPF_HEADER_LENGTH - OSPF_HELLO_HEADER_LENGTH).get() / 4;
            if (numNeighbors < 0)
                helloPacket->markIncorrect();
            helloPacket->setNeighborArraySize(numNeighbors);
            for (int i = 0; i< numNeighbors; i++) {
                helloPacket->setNeighbor(i, stream.readIpv4Address());
            }
            return helloPacket;
        }
        case DATABASE_DESCRIPTION_PACKET: {
            auto ddPacket = makeShared<OspfDatabaseDescriptionPacket>();
            copyHeaderFields(ospfPacket, ddPacket);
            ddPacket->setInterfaceMTU(stream.readUint16Be());
            deserializeOspfOptions(stream, ddPacket->getOptionsForUpdate());
            auto& ddOptions = ddPacket->getDdOptionsForUpdate();
            ddOptions.unused = stream.readNBitsToUint64Be(5);
            ddOptions.I_Init = stream.readBit();
            ddOptions.M_More = stream.readBit();
            ddOptions.MS_MasterSlave = stream.readBit();
            ddPacket->setDdSequenceNumber(stream.readUint32Be());
            int numLsaHeaders = ((B(packetLength) - OSPF_HEADER_LENGTH - OSPF_DD_HEADER_LENGTH) / OSPF_LSA_HEADER_LENGTH).get();
            if (numLsaHeaders < 0)
                ddPacket->markIncorrect();
            ddPacket->setLsaHeadersArraySize(numLsaHeaders);
            for (int i = 0; i< numLsaHeaders; i++) {
                OspfLsaHeader *lsaHeader = new OspfLsaHeader();
                deserializeLsaHeader(stream, *lsaHeader);
                ddPacket->setLsaHeaders(i, *lsaHeader);
            }
            return ddPacket;
        }
        case LINKSTATE_REQUEST_PACKET: {
            auto requestPacket = makeShared<OspfLinkStateRequestPacket>();
            copyHeaderFields(ospfPacket, requestPacket);
            int numReq = (B(packetLength) - OSPF_HEADER_LENGTH).get() / OSPF_REQUEST_LENGTH.get();
            if (numReq < 0)
                requestPacket->markIncorrect();
            requestPacket->setRequestsArraySize(numReq);
            for (int i = 0; i < numReq; i++) {
                LsaRequest *req = new LsaRequest();
                req->lsType = stream.readUint32Be();
                req->linkStateID = stream.readIpv4Address();
                req->advertisingRouter = stream.readIpv4Address();
                requestPacket->setRequests(i, *req);
            }
            return requestPacket;
        }
        case LINKSTATE_UPDATE_PACKET: {
            auto updatePacket = makeShared<OspfLinkStateUpdatePacket>();
            copyHeaderFields(ospfPacket, updatePacket);
            uint32_t numLSAs = stream.readUint32Be();
            updatePacket->setOspfLSAsArraySize(numLSAs);
            for (uint32_t i = 0; i < numLSAs; i++) {
                deserializeLsa(stream, updatePacket, i);
            }
            return updatePacket;
        }
        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            auto ackPacket = makeShared<OspfLinkStateAcknowledgementPacket>();
            copyHeaderFields(ospfPacket, ackPacket);
            int numHeaders = (B(packetLength) - OSPF_HEADER_LENGTH).get() / OSPF_LSA_HEADER_LENGTH.get();
            if (numHeaders < 0)
                ackPacket->markIncorrect();
            ackPacket->setLsaHeadersArraySize(numHeaders);
            for (int i = 0; i < numHeaders; i++) {
                OspfLsaHeader *lsaHeader = new OspfLsaHeader();
                deserializeLsaHeader(stream, *lsaHeader);
                ackPacket->setLsaHeaders(i, *lsaHeader);
            }
            return ackPacket;
        }
        default: {
            ospfPacket->markIncorrect();
            return ospfPacket;
        }
    }
}

void OspfPacketSerializer::serializeOspfHeader(MemoryOutputStream& stream, const IntrusivePtr<const OspfPacket>& ospfPacket)
{
    stream.writeByte(ospfPacket->getVersion());
    stream.writeByte(ospfPacket->getType());
    stream.writeUint16Be(B(ospfPacket->getChunkLength()).get());
    stream.writeIpv4Address(ospfPacket->getRouterID());
    stream.writeIpv4Address(ospfPacket->getAreaID());
    auto crcMode = ospfPacket->getCrcMode();
    if (crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize Ospf header without properly computed CRC");
    stream.writeUint16Be(ospfPacket->getCrc());
    stream.writeUint16Be(ospfPacket->getAuthenticationType());
    for (unsigned int i = 0; i < 8; ++i) {
        stream.writeByte(ospfPacket->getAuthentication(i));
    }
}

uint16_t OspfPacketSerializer::deserializeOspfHeader(MemoryInputStream& stream, IntrusivePtr<OspfPacket>& ospfPacket)
{
    int ospfVer = stream.readUint8();
    if (ospfVer != 2)
        ospfPacket->markIncorrect();
    ospfPacket->setVersion(ospfVer);
    int ospfType = stream.readUint8();
    if (ospfType > LINKSTATE_ACKNOWLEDGEMENT_PACKET)
        ospfPacket->markIncorrect();
    ospfPacket->setType(static_cast<OspfPacketType>(ospfType));
    uint16_t packetLength = stream.readUint16Be();
    ospfPacket->setChunkLength(B(packetLength));
    ospfPacket->setRouterID(stream.readIpv4Address());
    ospfPacket->setAreaID(stream.readIpv4Address());
    ospfPacket->setCrc(stream.readUint16Be());
    ospfPacket->setCrcMode(CRC_COMPUTED);
    ospfPacket->setAuthenticationType(stream.readUint16Be());
    for (int i = 0; i < 8; ++i) {
        ospfPacket->setAuthentication(i, stream.readUint8());
    }
    return packetLength;
}

void OspfPacketSerializer::serializeLsaHeader(MemoryOutputStream& stream, const OspfLsaHeader& lsaHeader)
{
    stream.writeUint16Be(lsaHeader.getLsAge());
    serializeOspfOptions(stream, lsaHeader.getLsOptions());
    stream.writeByte(lsaHeader.getLsType());
    stream.writeIpv4Address(lsaHeader.getLinkStateID());
    stream.writeIpv4Address(lsaHeader.getAdvertisingRouter());
    stream.writeUint32Be(lsaHeader.getLsSequenceNumber());
    auto crcMode = lsaHeader.getLsCrcMode();
    if (crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize Ospf LSA header without properly computed CRC");
    stream.writeUint16Be(lsaHeader.getLsCrc());
    stream.writeUint16Be(lsaHeader.getLsaLength());
}

void OspfPacketSerializer::deserializeLsaHeader(MemoryInputStream& stream, OspfLsaHeader& lsaHeader)
{
    lsaHeader.setLsAge(stream.readUint16Be());
    deserializeOspfOptions(stream, lsaHeader.getLsOptionsForUpdate());
    lsaHeader.setLsType(static_cast<LsaType>(stream.readByte()));
    lsaHeader.setLinkStateID(stream.readIpv4Address());
    lsaHeader.setAdvertisingRouter(stream.readIpv4Address());
    lsaHeader.setLsSequenceNumber(stream.readUint32Be());
    lsaHeader.setLsCrc(stream.readUint16Be());
    lsaHeader.setLsaLength(stream.readUint16Be());
    lsaHeader.setLsCrcMode(CRC_COMPUTED);
}

void OspfPacketSerializer::serializeRouterLsa(MemoryOutputStream& stream, const OspfRouterLsa& routerLsa)
{
    stream.writeNBitsOfUint64Be(routerLsa.getReserved1(), 5);
    stream.writeBit(routerLsa.getV_VirtualLinkEndpoint());
    stream.writeBit(routerLsa.getE_ASBoundaryRouter());
    stream.writeBit(routerLsa.getB_AreaBorderRouter());
    stream.writeByte(routerLsa.getReserved2());
    uint16_t numLinks = routerLsa.getNumberOfLinks();
    stream.writeUint16Be(numLinks);
    for (int32_t i = 0; i < numLinks; ++i) {
        const Link& link = routerLsa.getLinks(i);
        stream.writeIpv4Address(link.getLinkID());
        stream.writeUint32Be(link.getLinkData());
        stream.writeByte(link.getType());
        stream.writeByte(link.getNumberOfTOS());
        stream.writeUint16Be(link.getLinkCost());
        uint32_t numTos = link.getTosDataArraySize();
        for (uint32_t j = 0; j < numTos; ++j) {
            const TosData& tos = link.getTosData(j);
            stream.writeByte(tos.tos);
            stream.writeByte(0);
            stream.writeUint16Be(tos.tosMetric);
        }
    }
}

void OspfPacketSerializer::deserializeRouterLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfRouterLsa& routerLsa)
{
    routerLsa.setReserved1(stream.readNBitsToUint64Be(5));
    routerLsa.setV_VirtualLinkEndpoint(stream.readBit());
    routerLsa.setE_ASBoundaryRouter(stream.readBit());
    routerLsa.setB_AreaBorderRouter(stream.readBit());
    routerLsa.setReserved2(stream.readByte());
    uint16_t numLinks = stream.readUint16Be();
    routerLsa.setNumberOfLinks(numLinks);
    routerLsa.setLinksArraySize(numLinks);
    for (int32_t i = 0; i < numLinks; ++i) {
        Link *link = new Link();
        link->setLinkID(stream.readIpv4Address());
        link->setLinkData(stream.readUint32Be());
        link->setType(static_cast<LinkType>(stream.readByte()));
        link->setNumberOfTOS(stream.readByte());
        link->setLinkCost(stream.readUint16Be());
        uint32_t numTos = link->getNumberOfTOS();
        link->setTosDataArraySize(numTos);
        for (uint32_t j = 0; j < numTos; ++j) {
            TosData *tos = new TosData();
            tos->tos = stream.readByte();
            if (stream.readByte() != 0)
                updatePacket->markIncorrect();
            tos->tosMetric = stream.readUint16Be();
            link->setTosData(j, *tos);
        }
        routerLsa.setLinks(i, *link);
    }
}

void OspfPacketSerializer::serializeNetworkLsa(MemoryOutputStream& stream, const OspfNetworkLsa& networkLsa)
{
    stream.writeIpv4Address(networkLsa.getNetworkMask());
    for (size_t i = 0; i < networkLsa.getAttachedRoutersArraySize(); i++) {
        stream.writeIpv4Address(networkLsa.getAttachedRouters(i));
    }
}

void OspfPacketSerializer::deserializeNetworkLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfNetworkLsa& networkLsa)
{
    networkLsa.setNetworkMask(stream.readIpv4Address());
    int numAttachedRouters = (B(networkLsa.getHeader().getLsaLength()) -
            OSPF_LSA_HEADER_LENGTH - OSPF_NETWORKLSA_MASK_LENGTH).get() / OSPF_NETWORKLSA_ADDRESS_LENGTH.get();
    if (numAttachedRouters < 0)
        updatePacket->markIncorrect();
    else
        networkLsa.setAttachedRoutersArraySize(numAttachedRouters);
    for (int i = 0; i < numAttachedRouters; ++i) {
        networkLsa.setAttachedRouters(i, stream.readIpv4Address());
    }
}

void OspfPacketSerializer::serializeSummaryLsa(MemoryOutputStream& stream, const OspfSummaryLsa& summaryLsa)
{
    stream.writeIpv4Address(summaryLsa.getNetworkMask());
    stream.writeByte(0);
    stream.writeUint24Be(summaryLsa.getRouteCost());
    for (size_t i = 0; i < summaryLsa.getTosDataArraySize(); i++) {
        const TosData& tos = summaryLsa.getTosData(i);
        stream.writeByte(tos.tos);
        stream.writeUint24Be(tos.tosMetric);
    }
}

void OspfPacketSerializer::deserializeSummaryLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfSummaryLsa& summaryLsa)
{
    summaryLsa.setNetworkMask(stream.readIpv4Address());
    if (stream.readByte() != 0)
        updatePacket->markIncorrect();
    summaryLsa.setRouteCost(stream.readUint24Be());
    int numTos = (B(summaryLsa.getHeader().getLsaLength()) -
            OSPF_LSA_HEADER_LENGTH - OSPF_NETWORKLSA_MASK_LENGTH - B(4)).get() / OSPF_TOS_LENGTH.get();
    if (numTos < 0)
        updatePacket->markIncorrect();
    else
        summaryLsa.setTosDataArraySize(numTos);
    for (int i = 0; i < numTos; i++) {
        TosData *tos = new TosData();
        tos->tos = stream.readUint8();
        tos->tosMetric = stream.readUint24Be();
        summaryLsa.setTosData(i, *tos);
    }
}

void OspfPacketSerializer::serializeAsExternalLsa(MemoryOutputStream& stream, const OspfAsExternalLsa& asExternalLsa)
{
    auto& contents = asExternalLsa.getContents();
    stream.writeIpv4Address(contents.getNetworkMask());

    for (size_t i = 0; i < asExternalLsa.getContents().getExternalTOSInfoArraySize(); ++i) {
        const ExternalTosInfo& exTos = contents.getExternalTOSInfo(i);
        stream.writeBit(exTos.E_ExternalMetricType);
        stream.writeNBitsOfUint64Be(exTos.tos, 7);
        stream.writeUint24Be(exTos.routeCost);
        stream.writeIpv4Address(exTos.forwardingAddress);
        stream.writeUint32Be(exTos.externalRouteTag);
    }
}

void OspfPacketSerializer::deserializeAsExternalLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfAsExternalLsa& asExternalLsa)
{
    auto& contents = asExternalLsa.getContentsForUpdate();
    contents.setNetworkMask(stream.readIpv4Address());

    int numExternalTos = (B(asExternalLsa.getHeader().getLsaLength()) -
            OSPF_LSA_HEADER_LENGTH - OSPF_ASEXTERNALLSA_HEADER_LENGTH).get() / OSPF_ASEXTERNALLSA_TOS_INFO_LENGTH.get();
    if (numExternalTos < 0)
        updatePacket->markIncorrect();
    else
        contents.setExternalTOSInfoArraySize(numExternalTos);
    for (int i = 0; i < numExternalTos; i++) {
        ExternalTosInfo *extTos = new ExternalTosInfo();
        extTos->E_ExternalMetricType = stream.readBit();
        extTos->tos = stream.readNBitsToUint64Be(7);
        extTos->routeCost = stream.readUint24Be();
        extTos->forwardingAddress = stream.readIpv4Address();
        extTos->externalRouteTag = stream.readUint32Be();
        contents.setExternalTOSInfo(i, *extTos);
    }
}

void OspfPacketSerializer::serializeLsa(MemoryOutputStream& stream, const OspfLsa& lsa)
{
    auto& lsaHeader = lsa.getHeader();
    serializeLsaHeader(stream, lsaHeader);
    LsaType type = lsaHeader.getLsType();
    switch (type) {
    case ROUTERLSA_TYPE: {
        const OspfRouterLsa *routerLsa = static_cast<const OspfRouterLsa *>(&lsa);
        serializeRouterLsa(stream, *routerLsa);
        break;
    }
    case NETWORKLSA_TYPE: {
        const OspfNetworkLsa *networkLsa = static_cast<const OspfNetworkLsa *>(&lsa);
        serializeNetworkLsa(stream, *networkLsa);
        break;
    }
    case SUMMARYLSA_NETWORKS_TYPE:
    case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
        const OspfSummaryLsa *summaryLsa = static_cast<const OspfSummaryLsa *>(&lsa);
        serializeSummaryLsa(stream, *summaryLsa);
        break;
    }
    case AS_EXTERNAL_LSA_TYPE: {
        const OspfAsExternalLsa *asExternalLsa = static_cast<const OspfAsExternalLsa *>(&lsa);
        serializeAsExternalLsa(stream, *asExternalLsa);
        break;
    }
    default:
        throw cRuntimeError("Cannot serialize BGP packet: type %d not supported.", type);
    }
}

void OspfPacketSerializer::deserializeLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, int i)
{
    OspfLsaHeader *lsaHeader = new OspfLsaHeader();
    deserializeLsaHeader(stream, *lsaHeader);
    LsaType type = lsaHeader->getLsType();
    switch (type) {
        case ROUTERLSA_TYPE: {
            OspfRouterLsa *routerLsa = new OspfRouterLsa();
            routerLsa->setHeader(*lsaHeader);
            deserializeRouterLsa(stream, updatePacket, *routerLsa);
            updatePacket->setOspfLSAs(i, routerLsa);
            break;
        }
        case NETWORKLSA_TYPE: {
            OspfNetworkLsa *networkLsa = new OspfNetworkLsa();
            networkLsa->setHeader(*lsaHeader);
            deserializeNetworkLsa(stream, updatePacket, *networkLsa);
            updatePacket->setOspfLSAs(i, networkLsa);
            break;
        }
        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            OspfSummaryLsa *summaryLsa = new OspfSummaryLsa();
            summaryLsa->setHeader(*lsaHeader);
            deserializeSummaryLsa(stream, updatePacket, *summaryLsa);
            updatePacket->setOspfLSAs(i, summaryLsa);
            break;
        }
        case AS_EXTERNAL_LSA_TYPE: {
            OspfAsExternalLsa *asExternalLsa = new OspfAsExternalLsa();
            asExternalLsa->setHeader(*lsaHeader);
            deserializeAsExternalLsa(stream, updatePacket, *asExternalLsa);
            updatePacket->setOspfLSAs(i, asExternalLsa);
            break;
        }
        default:
            updatePacket->markIncorrect();
    }
}

void OspfPacketSerializer::serializeOspfOptions(MemoryOutputStream& stream, const OspfOptions& options)
{
    stream.writeBit(options.unused_1);
    stream.writeBit(options.unused_2);
    stream.writeBit(options.DC_DemandCircuits);
    stream.writeBit(options.EA_ForwardExternalLSAs);
    stream.writeBit(options.NP_Type7LSA);
    stream.writeBit(options.MC_MulticastForwarding);
    stream.writeBit(options.E_ExternalRoutingCapability);
    stream.writeBit(options.unused_3);
}

void OspfPacketSerializer::deserializeOspfOptions(MemoryInputStream& stream, OspfOptions& options)
{
    options.unused_1 = stream.readBit();
    options.unused_2 = stream.readBit();
    options.DC_DemandCircuits = stream.readBit();
    options.EA_ForwardExternalLSAs = stream.readBit();
    options.NP_Type7LSA = stream.readBit();
    options.MC_MulticastForwarding = stream.readBit();
    options.E_ExternalRoutingCapability = stream.readBit();
    options.unused_3 = stream.readBit();
}

void OspfPacketSerializer::copyHeaderFields(const Ptr<OspfPacket> from, Ptr<OspfPacket> to)
{
    to->setVersion(from->getVersion());
    to->setType(from->getType());
    to->setChunkLength(from->getChunkLength());
    to->setRouterID(from->getRouterID());
    to->setAreaID(from->getAreaID());
    to->setCrc(from->getCrc());
    to->setCrcMode(from->getCrcMode());
    to->setAuthenticationType(from->getAuthenticationType());
    for (int i = 0; i < 8; ++i) {
        to->setAuthentication(i, from->getAuthentication(i));
    }
}

} // namespace ospf

} // namespace inet

