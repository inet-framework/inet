//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospfv2/Ospfv2PacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {

namespace ospfv2 {

using namespace ospf;

Register_Serializer(Ospfv2Packet, Ospfv2PacketSerializer);
Register_Serializer(Ospfv2HelloPacket, Ospfv2PacketSerializer);
Register_Serializer(Ospfv2DatabaseDescriptionPacket, Ospfv2PacketSerializer);
Register_Serializer(Ospfv2LinkStateRequestPacket, Ospfv2PacketSerializer);
Register_Serializer(Ospfv2LinkStateUpdatePacket, Ospfv2PacketSerializer);
Register_Serializer(Ospfv2LinkStateAcknowledgementPacket, Ospfv2PacketSerializer);

void Ospfv2PacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ospfPacket = staticPtrCast<const Ospfv2Packet>(chunk);
    serializeOspfHeader(stream, ospfPacket);
    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            const auto& helloPacket = staticPtrCast<const Ospfv2HelloPacket>(ospfPacket);
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
            const auto& ddPacket = staticPtrCast<const Ospfv2DatabaseDescriptionPacket>(ospfPacket);
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
            const auto& requestPacket = staticPtrCast<const Ospfv2LinkStateRequestPacket>(ospfPacket);
            for (size_t i = 0; i < requestPacket->getRequestsArraySize(); ++i) {
                auto& req = requestPacket->getRequests(i);
                stream.writeUint32Be(req.lsType);
                stream.writeIpv4Address(req.linkStateID);
                stream.writeIpv4Address(req.advertisingRouter);
            }
            break;
        }
        case LINKSTATE_UPDATE_PACKET: {
            const auto& updatePacket = staticPtrCast<const Ospfv2LinkStateUpdatePacket>(ospfPacket);
            stream.writeUint32Be(updatePacket->getOspfLSAsArraySize());
            for (size_t i = 0; i < updatePacket->getOspfLSAsArraySize(); ++i) {
                serializeLsa(stream, *updatePacket->getOspfLSAs(i));
            }
            break;
        }
        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            const auto& ackPacket = staticPtrCast<const Ospfv2LinkStateAcknowledgementPacket>(ospfPacket);
            for (size_t i = 0; i < ackPacket->getLsaHeadersArraySize(); ++i) {
                serializeLsaHeader(stream, ackPacket->getLsaHeaders(i));
            }
            break;
        }
        default:
            throw cRuntimeError("Unknown OSPF message type in Ospfv2PacketSerializer");
    }
}

const Ptr<Chunk> Ospfv2PacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ospfPacket = makeShared<Ospfv2Packet>();
    uint16_t packetLength = deserializeOspfHeader(stream, ospfPacket);
    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            auto helloPacket = makeShared<Ospfv2HelloPacket>();
            copyHeaderFields(ospfPacket, helloPacket);
            helloPacket->setNetworkMask(stream.readIpv4Address());
            helloPacket->setHelloInterval(stream.readUint16Be());
            deserializeOspfOptions(stream, helloPacket->getOptionsForUpdate());
            helloPacket->setRouterPriority(stream.readByte());
            helloPacket->setRouterDeadInterval(stream.readUint32Be());
            helloPacket->setDesignatedRouter(stream.readIpv4Address());
            helloPacket->setBackupDesignatedRouter(stream.readIpv4Address());
            int numNeighbors = (B(packetLength) - OSPFv2_HEADER_LENGTH - OSPFv2_HELLO_HEADER_LENGTH).get() / 4;
            if (numNeighbors < 0)
                helloPacket->markIncorrect();
            helloPacket->setNeighborArraySize(numNeighbors);
            for (int i = 0; i < numNeighbors; i++) {
                helloPacket->setNeighbor(i, stream.readIpv4Address());
            }
            return helloPacket;
        }
        case DATABASE_DESCRIPTION_PACKET: {
            auto ddPacket = makeShared<Ospfv2DatabaseDescriptionPacket>();
            copyHeaderFields(ospfPacket, ddPacket);
            ddPacket->setInterfaceMTU(stream.readUint16Be());
            deserializeOspfOptions(stream, ddPacket->getOptionsForUpdate());
            auto& ddOptions = ddPacket->getDdOptionsForUpdate();
            ddOptions.unused = stream.readNBitsToUint64Be(5);
            ddOptions.I_Init = stream.readBit();
            ddOptions.M_More = stream.readBit();
            ddOptions.MS_MasterSlave = stream.readBit();
            ddPacket->setDdSequenceNumber(stream.readUint32Be());
            int numLsaHeaders = ((B(packetLength) - OSPFv2_HEADER_LENGTH - OSPFv2_DD_HEADER_LENGTH) / OSPFv2_LSA_HEADER_LENGTH).get();
            if (numLsaHeaders < 0)
                ddPacket->markIncorrect();
            ddPacket->setLsaHeadersArraySize(numLsaHeaders);
            for (int i = 0; i < numLsaHeaders; i++) {
                Ospfv2LsaHeader *lsaHeader = new Ospfv2LsaHeader();
                deserializeLsaHeader(stream, *lsaHeader);
                ddPacket->setLsaHeaders(i, *lsaHeader);
            }
            return ddPacket;
        }
        case LINKSTATE_REQUEST_PACKET: {
            auto requestPacket = makeShared<Ospfv2LinkStateRequestPacket>();
            copyHeaderFields(ospfPacket, requestPacket);
            int numReq = (B(packetLength) - OSPFv2_HEADER_LENGTH).get() / OSPFv2_REQUEST_LENGTH.get();
            if (numReq < 0)
                requestPacket->markIncorrect();
            requestPacket->setRequestsArraySize(numReq);
            for (int i = 0; i < numReq; i++) {
                auto *req = new Ospfv2LsaRequest();
                req->lsType = stream.readUint32Be();
                req->linkStateID = stream.readIpv4Address();
                req->advertisingRouter = stream.readIpv4Address();
                requestPacket->setRequests(i, *req);
            }
            return requestPacket;
        }
        case LINKSTATE_UPDATE_PACKET: {
            auto updatePacket = makeShared<Ospfv2LinkStateUpdatePacket>();
            copyHeaderFields(ospfPacket, updatePacket);
            uint32_t numLSAs = stream.readUint32Be();
            updatePacket->setOspfLSAsArraySize(numLSAs);
            for (uint32_t i = 0; i < numLSAs; i++) {
                deserializeLsa(stream, updatePacket, i);
            }
            return updatePacket;
        }
        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            auto ackPacket = makeShared<Ospfv2LinkStateAcknowledgementPacket>();
            copyHeaderFields(ospfPacket, ackPacket);
            int numHeaders = (B(packetLength) - OSPFv2_HEADER_LENGTH).get() / OSPFv2_LSA_HEADER_LENGTH.get();
            if (numHeaders < 0)
                ackPacket->markIncorrect();
            ackPacket->setLsaHeadersArraySize(numHeaders);
            for (int i = 0; i < numHeaders; i++) {
                Ospfv2LsaHeader *lsaHeader = new Ospfv2LsaHeader();
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

void Ospfv2PacketSerializer::serializeOspfHeader(MemoryOutputStream& stream, const Ptr<const Ospfv2Packet>& ospfPacket)
{
    stream.writeByte(ospfPacket->getVersion());
    stream.writeByte(ospfPacket->getType());
    stream.writeUint16Be(ospfPacket->getPacketLengthField());
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

uint16_t Ospfv2PacketSerializer::deserializeOspfHeader(MemoryInputStream& stream, Ptr<Ospfv2Packet>& ospfPacket)
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
    ospfPacket->setPacketLengthField(packetLength);
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

void Ospfv2PacketSerializer::serializeLsaHeader(MemoryOutputStream& stream, const Ospfv2LsaHeader& lsaHeader)
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

void Ospfv2PacketSerializer::deserializeLsaHeader(MemoryInputStream& stream, Ospfv2LsaHeader& lsaHeader)
{
    lsaHeader.setLsAge(stream.readUint16Be());
    deserializeOspfOptions(stream, lsaHeader.getLsOptionsForUpdate());
    lsaHeader.setLsType(static_cast<Ospfv2LsaType>(stream.readByte()));
    lsaHeader.setLinkStateID(stream.readIpv4Address());
    lsaHeader.setAdvertisingRouter(stream.readIpv4Address());
    lsaHeader.setLsSequenceNumber(stream.readUint32Be());
    lsaHeader.setLsCrc(stream.readUint16Be());
    lsaHeader.setLsaLength(stream.readUint16Be());
    lsaHeader.setLsCrcMode(CRC_COMPUTED);
}

void Ospfv2PacketSerializer::serializeRouterLsa(MemoryOutputStream& stream, const Ospfv2RouterLsa& routerLsa)
{
    stream.writeNBitsOfUint64Be(routerLsa.getReserved1(), 5);
    stream.writeBit(routerLsa.getV_VirtualLinkEndpoint());
    stream.writeBit(routerLsa.getE_ASBoundaryRouter());
    stream.writeBit(routerLsa.getB_AreaBorderRouter());
    stream.writeByte(routerLsa.getReserved2());
    uint16_t numLinks = routerLsa.getNumberOfLinks();
    stream.writeUint16Be(numLinks);
    for (int32_t i = 0; i < numLinks; ++i) {
        const auto& link = routerLsa.getLinks(i);
        stream.writeIpv4Address(link.getLinkID());
        stream.writeUint32Be(link.getLinkData());
        stream.writeByte(link.getType());
        stream.writeByte(link.getNumberOfTOS());
        stream.writeUint16Be(link.getLinkCost());
        uint32_t numTos = link.getTosDataArraySize();
        for (uint32_t j = 0; j < numTos; ++j) {
            const auto& tos = link.getTosData(j);
            stream.writeByte(tos.tos);
            stream.writeByte(0);
            stream.writeUint16Be(tos.tosMetric);
        }
    }
}

void Ospfv2PacketSerializer::deserializeRouterLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2RouterLsa& routerLsa)
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
        auto *link = new Ospfv2Link();
        link->setLinkID(stream.readIpv4Address());
        link->setLinkData(stream.readUint32Be());
        link->setType(static_cast<LinkType>(stream.readByte()));
        link->setNumberOfTOS(stream.readByte());
        link->setLinkCost(stream.readUint16Be());
        uint32_t numTos = link->getNumberOfTOS();
        link->setTosDataArraySize(numTos);
        for (uint32_t j = 0; j < numTos; ++j) {
            auto *tos = new Ospfv2TosData();
            tos->tos = stream.readByte();
            if (stream.readByte() != 0)
                updatePacket->markIncorrect();
            tos->tosMetric = stream.readUint16Be();
            link->setTosData(j, *tos);
        }
        routerLsa.setLinks(i, *link);
        delete link;
    }
}

void Ospfv2PacketSerializer::serializeNetworkLsa(MemoryOutputStream& stream, const Ospfv2NetworkLsa& networkLsa)
{
    stream.writeIpv4Address(networkLsa.getNetworkMask());
    for (size_t i = 0; i < networkLsa.getAttachedRoutersArraySize(); i++) {
        stream.writeIpv4Address(networkLsa.getAttachedRouters(i));
    }
}

void Ospfv2PacketSerializer::deserializeNetworkLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2NetworkLsa& networkLsa)
{
    networkLsa.setNetworkMask(stream.readIpv4Address());
    int numAttachedRouters = (B(networkLsa.getHeader().getLsaLength()) -
                              OSPFv2_LSA_HEADER_LENGTH - OSPFv2_NETWORKLSA_MASK_LENGTH).get() / OSPFv2_NETWORKLSA_ADDRESS_LENGTH.get();
    if (numAttachedRouters < 0)
        updatePacket->markIncorrect();
    else
        networkLsa.setAttachedRoutersArraySize(numAttachedRouters);
    for (int i = 0; i < numAttachedRouters; ++i) {
        networkLsa.setAttachedRouters(i, stream.readIpv4Address());
    }
}

void Ospfv2PacketSerializer::serializeSummaryLsa(MemoryOutputStream& stream, const Ospfv2SummaryLsa& summaryLsa)
{
    stream.writeIpv4Address(summaryLsa.getNetworkMask());
    stream.writeByte(0);
    stream.writeUint24Be(summaryLsa.getRouteCost());
    for (size_t i = 0; i < summaryLsa.getTosDataArraySize(); i++) {
        const auto& tos = summaryLsa.getTosData(i);
        stream.writeByte(tos.tos);
        stream.writeUint24Be(tos.tosMetric);
    }
}

void Ospfv2PacketSerializer::deserializeSummaryLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2SummaryLsa& summaryLsa)
{
    summaryLsa.setNetworkMask(stream.readIpv4Address());
    if (stream.readByte() != 0)
        updatePacket->markIncorrect();
    summaryLsa.setRouteCost(stream.readUint24Be());
    int numTos = (B(summaryLsa.getHeader().getLsaLength()) -
                  OSPFv2_LSA_HEADER_LENGTH - OSPFv2_NETWORKLSA_MASK_LENGTH - B(4)).get() / OSPFv2_TOS_LENGTH.get();
    if (numTos < 0)
        updatePacket->markIncorrect();
    else
        summaryLsa.setTosDataArraySize(numTos);
    for (int i = 0; i < numTos; i++) {
        auto *tos = new Ospfv2TosData();
        tos->tos = stream.readUint8();
        tos->tosMetric = stream.readUint24Be();
        summaryLsa.setTosData(i, *tos);
    }
}

void Ospfv2PacketSerializer::serializeAsExternalLsa(MemoryOutputStream& stream, const Ospfv2AsExternalLsa& asExternalLsa)
{
    auto& contents = asExternalLsa.getContents();
    stream.writeIpv4Address(contents.getNetworkMask());

    for (size_t i = 0; i < asExternalLsa.getContents().getExternalTOSInfoArraySize(); ++i) {
        const auto& exTos = contents.getExternalTOSInfo(i);
        stream.writeBit(exTos.E_ExternalMetricType);
        stream.writeNBitsOfUint64Be(exTos.tos, 7);
        stream.writeUint24Be(exTos.routeCost);
        stream.writeIpv4Address(exTos.forwardingAddress);
        stream.writeUint32Be(exTos.externalRouteTag);
    }
}

void Ospfv2PacketSerializer::deserializeAsExternalLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2AsExternalLsa& asExternalLsa)
{
    auto& contents = asExternalLsa.getContentsForUpdate();
    contents.setNetworkMask(stream.readIpv4Address());

    int numExternalTos = (B(asExternalLsa.getHeader().getLsaLength()) -
                          OSPFv2_LSA_HEADER_LENGTH - OSPFv2_ASEXTERNALLSA_HEADER_LENGTH).get() / OSPFv2_ASEXTERNALLSA_TOS_INFO_LENGTH.get();
    if (numExternalTos < 0)
        updatePacket->markIncorrect();
    else
        contents.setExternalTOSInfoArraySize(numExternalTos);
    for (int i = 0; i < numExternalTos; i++) {
        auto *extTos = new Ospfv2ExternalTosInfo();
        extTos->E_ExternalMetricType = stream.readBit();
        extTos->tos = stream.readNBitsToUint64Be(7);
        extTos->routeCost = stream.readUint24Be();
        extTos->forwardingAddress = stream.readIpv4Address();
        extTos->externalRouteTag = stream.readUint32Be();
        contents.setExternalTOSInfo(i, *extTos);
        delete extTos;
    }
}

void Ospfv2PacketSerializer::serializeLsa(MemoryOutputStream& stream, const Ospfv2Lsa& lsa)
{
    auto& lsaHeader = lsa.getHeader();
    serializeLsaHeader(stream, lsaHeader);
    Ospfv2LsaType type = lsaHeader.getLsType();
    switch (type) {
        case ROUTERLSA_TYPE: {
            const Ospfv2RouterLsa *routerLsa = static_cast<const Ospfv2RouterLsa *>(&lsa);
            serializeRouterLsa(stream, *routerLsa);
            break;
        }
        case NETWORKLSA_TYPE: {
            const Ospfv2NetworkLsa *networkLsa = static_cast<const Ospfv2NetworkLsa *>(&lsa);
            serializeNetworkLsa(stream, *networkLsa);
            break;
        }
        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            const Ospfv2SummaryLsa *summaryLsa = static_cast<const Ospfv2SummaryLsa *>(&lsa);
            serializeSummaryLsa(stream, *summaryLsa);
            break;
        }
        case AS_EXTERNAL_LSA_TYPE: {
            const Ospfv2AsExternalLsa *asExternalLsa = static_cast<const Ospfv2AsExternalLsa *>(&lsa);
            serializeAsExternalLsa(stream, *asExternalLsa);
            break;
        }
        default:
            throw cRuntimeError("Cannot serialize BGP packet: type %d not supported.", type);
    }
}

void Ospfv2PacketSerializer::deserializeLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, int i)
{
    Ospfv2LsaHeader *lsaHeader = new Ospfv2LsaHeader();
    deserializeLsaHeader(stream, *lsaHeader);
    Ospfv2LsaType type = lsaHeader->getLsType();
    switch (type) {
        case ROUTERLSA_TYPE: {
            Ospfv2RouterLsa *routerLsa = new Ospfv2RouterLsa();
            routerLsa->setHeader(*lsaHeader);
            deserializeRouterLsa(stream, updatePacket, *routerLsa);
            updatePacket->setOspfLSAs(i, routerLsa);
            break;
        }
        case NETWORKLSA_TYPE: {
            Ospfv2NetworkLsa *networkLsa = new Ospfv2NetworkLsa();
            networkLsa->setHeader(*lsaHeader);
            deserializeNetworkLsa(stream, updatePacket, *networkLsa);
            updatePacket->setOspfLSAs(i, networkLsa);
            break;
        }
        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
            Ospfv2SummaryLsa *summaryLsa = new Ospfv2SummaryLsa();
            summaryLsa->setHeader(*lsaHeader);
            deserializeSummaryLsa(stream, updatePacket, *summaryLsa);
            updatePacket->setOspfLSAs(i, summaryLsa);
            break;
        }
        case AS_EXTERNAL_LSA_TYPE: {
            Ospfv2AsExternalLsa *asExternalLsa = new Ospfv2AsExternalLsa();
            asExternalLsa->setHeader(*lsaHeader);
            deserializeAsExternalLsa(stream, updatePacket, *asExternalLsa);
            updatePacket->setOspfLSAs(i, asExternalLsa);
            break;
        }
        default:
            updatePacket->markIncorrect();
    }
}

void Ospfv2PacketSerializer::serializeOspfOptions(MemoryOutputStream& stream, const Ospfv2Options& options)
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

void Ospfv2PacketSerializer::deserializeOspfOptions(MemoryInputStream& stream, Ospfv2Options& options)
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

void Ospfv2PacketSerializer::copyHeaderFields(const Ptr<Ospfv2Packet> from, Ptr<Ospfv2Packet> to)
{
    to->setVersion(from->getVersion());
    to->setType(from->getType());
    to->setPacketLengthField(from->getPacketLengthField());
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

} // namespace ospfv2

} // namespace inet

