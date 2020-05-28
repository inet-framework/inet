#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpNatHook.h"

namespace inet {
namespace sctp {

Define_Module(SctpNatHook);

SctpNatHook::SctpNatHook()
{
    ipLayer = nullptr;
    natTable = nullptr;
    rt = nullptr;
    ift = nullptr;
    nattedPackets = 0;
}

SctpNatHook::~SctpNatHook()
{
}

void SctpNatHook::initialize()
{
    rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
    ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
   // ipLayer = getModuleFromPar<IPv4>(par("networkProtocolModule"), this);
    natTable = getModuleFromPar<SctpNatTable>(par("natTableModule"), this);
    auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
    ipv4->registerHook(0, this);
}

/*INetfilter::IHook::Result SctpNatHook::datagramForwardHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)*/
INetfilter::IHook::Result SctpNatHook::datagramForwardHook(Packet *datagram)
{
    SctpNatEntry *entry;
    SctpChunk *chunk;
    const auto& tag = datagram->findTag<InterfaceReq>();
    const InterfaceEntry *outIE = (tag != nullptr ? ift->getInterfaceById(tag->getInterfaceId()) : nullptr);

    const auto& inIeTag = datagram->findTag<InterfaceInd>();
    const InterfaceEntry *inIE = (inIeTag != nullptr ? ift->getInterfaceById(inIeTag->getInterfaceId()) : nullptr);

    const auto& dgram = removeNetworkProtocolHeader<Ipv4Header>(datagram);

    if (!dgram) {
        insertNetworkProtocolHeader(datagram, Protocol::ipv4, dgram);
        return INetfilter::IHook::ACCEPT;
    }
    if (dgram->isFragment()) {
        //TODO process fragmented packets
        insertNetworkProtocolHeader(datagram, Protocol::ipv4, dgram);
        return INetfilter::IHook::ACCEPT;
    }
    if (SctpAssociation::getAddressLevel(dgram->getSrcAddress()) != 3) {
        insertNetworkProtocolHeader(datagram, Protocol::ipv4, dgram);
        return INetfilter::IHook::ACCEPT;
    }
    const auto& networkHeader = staticPtrCast<Ipv4Header>(dgram->dupShared());
    natTable->printNatTable();
    auto& sctpMsg = datagram->removeAtFront<SctpHeader>();
    Ptr<SctpHeader> sctp = staticPtrCast<SctpHeader>(sctpMsg->dupShared());
    unsigned int numberOfChunks = sctpMsg->getSctpChunksArraySize();
    if (numberOfChunks == 1) {
        chunk = sctpMsg->peekFirstChunk();
    } else {
        chunk = sctpMsg->peekLastChunk();
    }
    EV << "findEntry for " << dgram->getSrcAddress() << ":" << sctpMsg->getSrcPort() << " to " << dgram->getDestAddress() << ":" << sctpMsg->getDestPort() << " vTag=" << sctpMsg->getVTag() << "\n";
    if (chunk->getSctpChunkType() == INIT || chunk->getSctpChunkType() == INIT_ACK || chunk->getSctpChunkType() == ASCONF) {
        entry = new SctpNatEntry();
        entry->setLocalAddress(dgram->getSrcAddress());
        entry->setLocalPort(sctpMsg->getSrcPort());
        entry->setGlobalAddress(dgram->getDestAddress());
        entry->setGlobalPort(sctpMsg->getDestPort());
        entry->setNattedAddress(outIE->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        entry->setNattedPort(sctpMsg->getSrcPort());
        entry->setGlobalVTag(sctpMsg->getVTag());
        if (chunk->getSctpChunkType() == INIT) {
            SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
            entry->setLocalVTag(initChunk->getInitTag());
        }
        else if (chunk->getSctpChunkType() == INIT_ACK) {
            SctpInitAckChunk *initAckChunk = check_and_cast<SctpInitAckChunk *>(chunk);
            entry->setLocalVTag(initAckChunk->getInitTag());
        }
        else if (chunk->getSctpChunkType() == ASCONF) {
            SctpAsconfChunk *asconfChunk = check_and_cast<SctpAsconfChunk *>(chunk);
            entry->setLocalVTag(asconfChunk->getPeerVTag());
        }
        networkHeader->setSrcAddress(outIE->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        sctp->setSrcPort(entry->getNattedPort());
        natTable->natEntries.push_back(entry);
        natTable->printNatTable();
    } else {
        EV << "other chunkType: " << (chunk->getSctpChunkType() == COOKIE_ECHO ? "Cookie_Echo" : "other") << ", VTag=" << sctpMsg->getVTag() << "\n";
        entry = natTable->findNatEntry(networkHeader->getSrcAddress(), sctp->getSrcPort(), networkHeader->getDestAddress(), sctp->getDestPort(), sctp->getVTag());
        if (entry == nullptr) {
            EV << "no entry found\n";
            entry = natTable->findNatEntry(networkHeader->getSrcAddress(), sctp->getSrcPort(), networkHeader->getDestAddress(), sctp->getDestPort(), 0);
            if (entry == nullptr) {
                EV << "send back error message dgram=" << networkHeader << "\n";
                sendBackError(sctp.get());
                sctp->setSrcPort(sctpMsg->getDestPort());
                sctp->setDestPort(sctpMsg->getSrcPort());
                Ipv4Address tmpaddr = dgram->getDestAddress();
                networkHeader->setDestAddress(dgram->getSrcAddress());
                if (!tmpaddr.isUnspecified())
                    networkHeader->setSrcAddress(tmpaddr);
                networkHeader->setTotalLengthField(sctp->getChunkLength() + B(20));
                insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
                insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
                datagram->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(networkHeader->getDestAddress());
                datagram->addTagIfAbsent<InterfaceReq>()->setInterfaceId(CHK(inIE)->getInterfaceId());
                return INetfilter::IHook::ACCEPT;
            }
            else {
                EV << "VTag doesn't match: old VTag=" << entry->getLocalVTag() << ", new VTag=" << sctpMsg->getVTag() << "\n";
                entry->setLocalVTag(sctpMsg->getVTag());
                networkHeader->setSrcAddress(outIE->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
                sctp->setSrcPort(entry->getNattedPort());
                EV << "srcAddress set to " << dgram->getSrcAddress() << ", srcPort set to " << sctpMsg->getSrcPort() << "\n";
            }
        }
        else {
            networkHeader->setSrcAddress(outIE->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
            sctp->setSrcPort(entry->getNattedPort());
            EV << "srcAddress set to " << dgram->getSrcAddress() << ", srcPort set to " << sctpMsg->getSrcPort() << "\n";
        }
    }
    nattedPackets++;
    insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
    insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
    return INetfilter::IHook::ACCEPT;
}

/*INetfilter::IHook::Result SctpNatHook::datagramPreRoutingHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)*/
INetfilter::IHook::Result SctpNatHook::datagramPreRoutingHook(Packet *datagram)
{
    SctpNatEntry *entry;
    SctpChunk *chunk;

    const auto& dgram = removeNetworkProtocolHeader<Ipv4Header>(datagram);
    const auto& networkHeader = staticPtrCast<Ipv4Header>(dgram->dupShared());
    if (SctpAssociation::getAddressLevel(dgram->getSourceAddress()) == 3) {
        insertNetworkProtocolHeader(datagram, Protocol::ipv4, dgram);
        return INetfilter::IHook::ACCEPT;
    }

    if (dgram->isFragment()) {
        //TODO process fragmented packets
        insertNetworkProtocolHeader(datagram, Protocol::ipv4, dgram);
        return INetfilter::IHook::ACCEPT;
    }

    natTable->printNatTable();
    bool local = ((rt->isLocalAddress(dgram->getDestinationAddress())) && (SctpAssociation::getAddressLevel(dgram->getSourceAddress()) == 3));
    auto& sctpMsg = datagram->removeAtFront<SctpHeader>();
    Ptr<SctpHeader> sctp = staticPtrCast<SctpHeader>(sctpMsg->dupShared());
    unsigned int numberOfChunks = sctpMsg->getSctpChunksArraySize();
    if (numberOfChunks == 1)
        chunk = sctpMsg->peekFirstChunk();
    else
        chunk = sctpMsg->peekLastChunk();
    if (!local) {
        entry = natTable->getEntry(dgram->getSourceAddress(), sctpMsg->getSrcPort(), dgram->getDestinationAddress(), sctpMsg->getDestPort(), sctpMsg->getVTag());
        EV_INFO << "getEntry for " << dgram->getSourceAddress() << ":" << sctpMsg->getSrcPort() << " to " << dgram->getDestinationAddress() << ":" << sctpMsg->getDestPort() << " peerVTag=" << sctpMsg->getVTag() << "\n";
        uint32 numberOfChunks = sctpMsg->getSctpChunksArraySize();
        if (entry == nullptr) {
            EV_INFO << "no entry found\n";
            if (numberOfChunks == 1)
                chunk = sctpMsg->peekFirstChunk();
            else
                chunk = sctpMsg->peekLastChunk();
            if (chunk->getSctpChunkType() == INIT || chunk->getSctpChunkType() == ASCONF) {
                EV_INFO << "could be an Init collision\n";
                entry = natTable->getSpecialEntry(dgram->getSourceAddress(), sctpMsg->getSrcPort(), dgram->getDestinationAddress(), sctpMsg->getDestPort());
                if (entry != nullptr) {
                    if (chunk->getSctpChunkType() == INIT) {
                        SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                        entry->setLocalVTag(initChunk->getInitTag());
                        EV_INFO << "InitTag=" << initChunk->getInitTag() << "\n";
                    }
                    else if (chunk->getSctpChunkType() == ASCONF) {
                        SctpAsconfChunk *asconfChunk = check_and_cast<SctpAsconfChunk *>(chunk);
                        entry->setLocalVTag(asconfChunk->getPeerVTag());
                    }
                    networkHeader->setDestinationAddress(entry->getLocalAddress().toIpv4());
                    sctp->setDestPort(entry->getLocalPort());
                    EV_INFO << "destAddress set to " << dgram->getDestinationAddress() << ", destPort set to " << sctp->getDestPort() << "\n";
                }
                else {
                    insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
                    insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
                    return INetfilter::IHook::DROP;
                }
            }
            else {
               /* SctpChunk *schunk = nullptr;
                if (numberOfChunks > 0) {
                    EV << "number of chunks=" << numberOfChunks << "\n";
                    for (uint32 i = 0; i < numberOfChunks; i++) {
                        schunk = (SctpChunk *)(sctpMsg->removeChunk());
                        if (schunk->getSctpChunkType() == DATA)
                            delete (SctpSimpleMessage *)schunk->decapsulate();
                        EV << "delete chunk " << schunk->getName() << "\n";
                        delete schunk;
                    }
                }*/
                insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
                insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
                return INetfilter::IHook::DROP;
            }
        }
        else {
            networkHeader->setDestinationAddress(entry->getLocalAddress().toIpv4());
            sctp->setDestPort(entry->getLocalPort());
            if (entry->getGlobalVTag() == 0 && chunk->getSctpChunkType() == INIT_ACK) {
                SctpInitAckChunk *initAckChunk = check_and_cast<SctpInitAckChunk *>(chunk);
                entry->setGlobalVTag(initAckChunk->getInitTag());
            }
            EV << "destAddress set to " << dgram->getDestinationAddress() << ", destPort set to " << sctpMsg->getDestPort() << "\n";
        }
    }
    else {
        if (chunk->getSctpChunkType() == INIT) {
            EV << "getLocALEntry for " << dgram->getSourceAddress() << ":" << sctpMsg->getSrcPort() << " to " << dgram->getDestinationAddress() << ":" << sctpMsg->getDestPort() << " peerVTag=" << sctpMsg->getVTag() << "\n";
            entry = natTable->getLocalInitEntry(dgram->getDestinationAddress(), sctpMsg->getSrcPort(), sctpMsg->getDestPort());
            if (entry == nullptr) {
                entry = new SctpNatEntry();
                entry->setLocalAddress(dgram->getSourceAddress());
                entry->setLocalPort(sctpMsg->getSrcPort());
                entry->setGlobalAddress(dgram->getDestinationAddress());
                entry->setGlobalPort(sctpMsg->getDestPort());
                entry->setNattedPort(sctpMsg->getSrcPort());
                entry->setNattedAddress(dgram->getDestinationAddress());
                SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                entry->setGlobalVTag(initChunk->getInitTag());
                natTable->natEntries.push_back(entry);
                EV << "added entry for local deliver\n";
                natTable->printNatTable();
                insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
                insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
                return INetfilter::IHook::DROP;
            }
            else {
                SctpNatEntry *entry2 = new SctpNatEntry();
                entry2->setLocalAddress(dgram->getSourceAddress());
                entry2->setLocalPort(sctpMsg->getSrcPort());
                entry2->setGlobalAddress(entry->getGlobalAddress());
                entry2->setGlobalPort(sctpMsg->getDestPort());
                entry2->setNattedPort(sctpMsg->getSrcPort());
                entry2->setNattedAddress(entry->getGlobalAddress());
                SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                entry2->setGlobalVTag(initChunk->getInitTag());
                natTable->natEntries.push_back(entry2);
                networkHeader->setDestinationAddress(entry->getLocalAddress().toIpv4());
                sctp->setDestPort(entry->getLocalPort());
                networkHeader->setSourceAddress(entry->getGlobalAddress().toIpv4());
                sctp->setSrcPort(entry->getGlobalPort());
                EV << "added additional entry for local deliver\n";
                natTable->printNatTable();
                EV << "destAddress set to " << dgram->getDestinationAddress() << ", destPort set to " << sctpMsg->getDestPort() << "\n";
            }
        }
        else {
            EV << "no INIT: destAddr=" << dgram->getDestinationAddress() << " destPort=" << sctpMsg->getDestPort() << " srcPort=" << sctpMsg->getSrcPort() << " vTag=" << sctpMsg->getVTag() << "\n";
            entry = natTable->getLocalEntry(dgram->getDestinationAddress(), sctpMsg->getSrcPort(), sctpMsg->getDestPort(), sctpMsg->getVTag());
            if (entry != nullptr) {
                networkHeader->setDestinationAddress(entry->getLocalAddress().toIpv4());
                sctp->setDestPort(entry->getLocalPort());
                networkHeader->setSourceAddress(entry->getGlobalAddress().toIpv4());
                sctp->setSrcPort(entry->getGlobalPort());
            }
            else {
                EV << "no entry found\n";
                insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
                insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);

                return INetfilter::IHook::DROP;
            }
        }
    }

    nattedPackets++;
    insertTransportProtocolHeader(datagram, Protocol::sctp, sctp);
    insertNetworkProtocolHeader(datagram, Protocol::ipv4, networkHeader);
    return INetfilter::IHook::ACCEPT;
}


void SctpNatHook::sendBackError(SctpHeader* sctp)
{
  //  SctpHeader *sctpmsg = new SctpHeader();
    delete sctp->removeFirstChunk();
    sctp->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpErrorChunk *errorChunk = new SctpErrorChunk("NatError");
    errorChunk->setSctpChunkType(ERRORTYPE);
    errorChunk->setByteLength(4);
    SctpSimpleErrorCauseParameter *cause = new SctpSimpleErrorCauseParameter("Cause");
    cause->setParameterType(MISSING_NAT_ENTRY);
    cause->setByteLength(4);
   // cause->encapsulate(dgram->dup());
    errorChunk->setMBit(true);
    errorChunk->setTBit(true);
    errorChunk->addParameters(cause);
    sctp->insertSctpChunks(errorChunk);
}

void SctpNatHook::finish()
{
    auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
    if (isRegisteredHook(ipv4))
        ipv4->unregisterHook(this);
    ipLayer = nullptr;
    EV_INFO<< getFullPath() << ": Natted packets: " << nattedPackets << "\n";
}

} // namespace sctp
} // namespace inet

