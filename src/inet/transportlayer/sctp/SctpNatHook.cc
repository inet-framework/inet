#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/common/ModuleAccess.h"
#include "inet/transportlayer/sctp/SctpNatHook.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"

namespace inet {

namespace sctp {
#if 0
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
    ipLayer = getModuleFromPar<IPv4>(par("networkProtocolModule"), this);
    natTable = getModuleFromPar<SctpNatTable>(par("natTableModule"), this);

    ipLayer->registerHook(0, this);
}

INetfilter::IHook::Result SctpNatHook::datagramForwardHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    SctpNatEntry *entry;
    SctpChunk *chunk;
    Ipv4Header *dgram;

    dgram = dynamic_cast<Ipv4Header *>(datagram);
    if (!dgram) {
        return INetfilter::IHook::ACCEPT;
    }
    if (SctpAssociation::getAddressLevel(dgram->getSrcAddress()) != 3) {
        return INetfilter::IHook::ACCEPT;
    }
    natTable->printNatTable();
    SctpHeader *sctpMsg = check_and_cast<SctpHeader *>(dgram->getEncapsulatedPacket());
    unsigned int numberOfChunks = sctpMsg->getChunksArraySize();
    if (numberOfChunks == 1)
        chunk = (SctpChunk *)(sctpMsg->peekFirstChunk());
    else
        chunk = (SctpChunk *)(sctpMsg->peekLastChunk());
    EV << "findEntry for " << dgram->getSrcAddress() << ":" << sctpMsg->getSrcPort() << " to " << dgram->getDestAddress() << ":" << sctpMsg->getDestPort() << " vTag=" << sctpMsg->getTag() << " natAddr=" << outIE->ipv4Data()->getIPAddress() << "\n";
    if (chunk->getChunkType() == INIT || chunk->getChunkType() == INIT_ACK || chunk->getChunkType() == ASCONF) {
        entry = new SctpNatEntry();
        entry->setLocalAddress(dgram->getSrcAddress());
        entry->setLocalPort(sctpMsg->getSrcPort());
        entry->setGlobalAddress(dgram->getDestAddress());
        entry->setGlobalPort(sctpMsg->getDestPort());
        entry->setNattedAddress(outIE->ipv4Data()->getIPAddress());
        entry->setNattedPort(sctpMsg->getSrcPort());
        entry->setGlobalVTag(sctpMsg->getTag());
        if (chunk->getChunkType() == INIT) {
            SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
            entry->setLocalVTag(initChunk->getInitTag());
        }
        else if (chunk->getChunkType() == INIT_ACK) {
            SctpInitAckChunk *initAckChunk = check_and_cast<SctpInitAckChunk *>(chunk);
            entry->setLocalVTag(initAckChunk->getInitTag());
        }
        else if (chunk->getChunkType() == ASCONF) {
            SctpAsconfChunk *asconfChunk = check_and_cast<SctpAsconfChunk *>(chunk);
            entry->setLocalVTag(asconfChunk->getPeerVTag());
        }
        dgram->setSrcAddress(outIE->ipv4Data()->getIPAddress());
        sctpMsg->setSrcPort(entry->getNattedPort());
        natTable->natEntries.push_back(entry);
    }
    else {
        EV << "other chunkType: " << (chunk->getChunkType() == COOKIE_ECHO ? "Cookie_Echo" : "other") << ", VTag=" << sctpMsg->getTag() << "\n";
        entry = natTable->findNatEntry(dgram->getSrcAddress(), sctpMsg->getSrcPort(), dgram->getDestAddress(), sctpMsg->getDestPort(), sctpMsg->getTag());
        if (entry == nullptr) {
            EV << "no entry found\n";
            entry = natTable->findNatEntry(dgram->getSrcAddress(), sctpMsg->getSrcPort(), dgram->getDestAddress(), sctpMsg->getDestPort(), 0);
            if (entry == nullptr) {
                EV << "send back error message dgram=" << dgram << "\n";
                sendBackError(dgram);
                nextHopAddr = dgram->getDestAddress();
                const InterfaceEntry *tmpIE = inIE;
                inIE = outIE;
                outIE = tmpIE;
                return INetfilter::IHook::ACCEPT;
            }
            else {
                EV << "VTag doesn't match: old VTag=" << entry->getLocalVTag() << ", new VTag=" << sctpMsg->getTag() << "\n";
                entry->setLocalVTag(sctpMsg->getTag());
                dgram->setSrcAddress(outIE->ipv4Data()->getIPAddress());
                sctpMsg->setSrcPort(entry->getNattedPort());
                EV << "srcAddress set to " << dgram->getSrcAddress() << ", srcPort set to " << sctpMsg->getSrcPort() << "\n";
            }
        }
        else {
            dgram->setSrcAddress(outIE->ipv4Data()->getIPAddress());
            sctpMsg->setSrcPort(entry->getNattedPort());
            EV << "srcAddress set to " << dgram->getSrcAddress() << ", srcPort set to " << sctpMsg->getSrcPort() << "\n";
        }
    }
    nattedPackets++;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SctpNatHook::datagramPreRoutingHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    SctpNatEntry *entry;
    SctpChunk *chunk;
    Ipv4Header *dgram;

    dgram = check_and_cast<Ipv4Header *>(datagram);
    if (SctpAssociation::getAddressLevel(dgram->getSrcAddress()) == 3) {
        return INetfilter::IHook::ACCEPT;
    }
    natTable->printNatTable();
    bool local = ((rt->isLocalAddress(dgram->getDestAddress())) && (SctpAssociation::getAddressLevel(dgram->getSrcAddress()) == 3));
    SctpHeader *sctpMsg = check_and_cast<SctpHeader *>(dgram->getEncapsulatedPacket());
    unsigned int numberOfChunks = sctpMsg->getChunksArraySize();
    if (numberOfChunks == 1)
        chunk = (SctpChunk *)(sctpMsg->peekFirstChunk());
    else
        chunk = (SctpChunk *)(sctpMsg->peekLastChunk());
    if (!local) {
        entry = natTable->getEntry(dgram->getSrcAddress(), sctpMsg->getSrcPort(), dgram->getDestAddress(), sctpMsg->getDestPort(), sctpMsg->getTag());
        EV << "getEntry for " << dgram->getSrcAddress() << ":" << sctpMsg->getSrcPort() << " to " << dgram->getDestAddress() << ":" << sctpMsg->getDestPort() << " peerVTag=" << sctpMsg->getTag() << "\n";
        uint32 numberOfChunks = sctpMsg->getChunksArraySize();
        if (entry == nullptr) {
            EV << "no entry found\n";
            if (numberOfChunks == 1)
                chunk = (SctpChunk *)(sctpMsg->peekFirstChunk());
            else
                chunk = (SctpChunk *)(sctpMsg->peekLastChunk());
            if (chunk->getChunkType() == INIT || chunk->getChunkType() == ASCONF) {
                EV << "could be an Init collision\n";
                entry = natTable->getSpecialEntry(dgram->getSrcAddress(), sctpMsg->getSrcPort(), dgram->getDestAddress(), sctpMsg->getDestPort());
                if (entry != nullptr) {
                    if (chunk->getChunkType() == INIT) {
                        SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                        entry->setLocalVTag(initChunk->getInitTag());
                        EV << "InitTag=" << initChunk->getInitTag() << "\n";
                    }
                    else if (chunk->getChunkType() == ASCONF) {
                        SctpAsconfChunk *asconfChunk = check_and_cast<SctpAsconfChunk *>(chunk);
                        entry->setLocalVTag(asconfChunk->getPeerVTag());
                    }
                    dgram->setDestAddress(entry->getLocalAddress().toIPv4());
                    sctpMsg->setDestPort(entry->getLocalPort());
                    EV << "destAddress set to " << dgram->getDestAddress() << ", destPort set to " << sctpMsg->getDestPort() << "\n";
                }
                else {
                    return INetfilter::IHook::DROP;
                }
            }
            else {
                SctpChunk *schunk;
                if (numberOfChunks > 0) {
                    EV << "number of chunks=" << numberOfChunks << "\n";
                    for (uint32 i = 0; i < numberOfChunks; i++) {
                        schunk = (SctpChunk *)(sctpMsg->removeChunk());
                        if (schunk->getChunkType() == DATA)
                            delete (SctpSimpleMessage *)schunk->decapsulate();
                        EV << "delete chunk " << schunk->getName() << "\n";
                        delete schunk;
                    }
                }
                return INetfilter::IHook::DROP;
            }
        }
        else {
            dgram->setDestAddress(entry->getLocalAddress().toIPv4());
            sctpMsg->setDestPort(entry->getLocalPort());
            if (entry->getGlobalVTag() == 0 && chunk->getChunkType() == INIT_ACK) {
                SctpInitAckChunk *initAckChunk = check_and_cast<SctpInitAckChunk *>(chunk);
                entry->setGlobalVTag(initAckChunk->getInitTag());
            }
            EV << "destAddress set to " << dgram->getDestAddress() << ", destPort set to " << sctpMsg->getDestPort() << "\n";
        }
    }
    else {
        if (chunk->getChunkType() == INIT) {
            EV << "getLocALEntry for " << dgram->getSrcAddress() << ":" << sctpMsg->getSrcPort() << " to " << dgram->getDestAddress() << ":" << sctpMsg->getDestPort() << " peerVTag=" << sctpMsg->getTag() << "\n";
            entry = natTable->getLocalInitEntry(dgram->getDestAddress(), sctpMsg->getSrcPort(), sctpMsg->getDestPort());
            if (entry == nullptr) {
                entry = new SctpNatEntry();
                entry->setLocalAddress(dgram->getSrcAddress());
                entry->setLocalPort(sctpMsg->getSrcPort());
                entry->setGlobalAddress(dgram->getDestAddress());
                entry->setGlobalPort(sctpMsg->getDestPort());
                entry->setNattedPort(sctpMsg->getSrcPort());
                entry->setNattedAddress(dgram->getDestAddress());
                SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                entry->setGlobalVTag(initChunk->getInitTag());
                natTable->natEntries.push_back(entry);
                EV << "added entry for local deliver\n";
                natTable->printNatTable();
                return INetfilter::IHook::DROP;
            }
            else {
                SctpNatEntry *entry2 = new SctpNatEntry();
                entry2->setLocalAddress(dgram->getSrcAddress());
                entry2->setLocalPort(sctpMsg->getSrcPort());
                entry2->setGlobalAddress(entry->getGlobalAddress());
                entry2->setGlobalPort(sctpMsg->getDestPort());
                entry2->setNattedPort(sctpMsg->getSrcPort());
                entry2->setNattedAddress(entry->getGlobalAddress());
                SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                entry2->setGlobalVTag(initChunk->getInitTag());
                natTable->natEntries.push_back(entry2);
                dgram->setDestAddress(entry->getLocalAddress().toIPv4());
                sctpMsg->setDestPort(entry->getLocalPort());
                dgram->setSrcAddress(entry->getGlobalAddress().toIPv4());
                sctpMsg->setSrcPort(entry->getGlobalPort());
                EV << "added additional entry for local deliver\n";
                natTable->printNatTable();
                EV << "destAddress set to " << dgram->getDestAddress() << ", destPort set to " << sctpMsg->getDestPort() << "\n";
            }
        }
        else {
            EV << "no INIT: destAddr=" << dgram->getDestAddress() << " destPort=" << sctpMsg->getDestPort() << " srcPort=" << sctpMsg->getSrcPort() << " vTag=" << sctpMsg->getTag() << "\n";
            entry = natTable->getLocalEntry(dgram->getDestAddress(), sctpMsg->getSrcPort(), sctpMsg->getDestPort(), sctpMsg->getTag());
            if (entry != nullptr) {
                dgram->setDestAddress(entry->getLocalAddress().toIPv4());
                sctpMsg->setDestPort(entry->getLocalPort());
                dgram->setSrcAddress(entry->getGlobalAddress().toIPv4());
                sctpMsg->setSrcPort(entry->getGlobalPort());
            }
            else {
                EV << "no entry found\n";
                return INetfilter::IHook::DROP;
            }
        }
    }
    nattedPackets++;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SctpNatHook::datagramPostRoutingHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SctpNatHook::datagramLocalInHook(INetworkHeader *datagram, const InterfaceEntry *inIE)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SctpNatHook::datagramLocalOutHook(INetworkHeader *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

void SctpNatHook::sendBackError(Ipv4Header *dgram)
{
    SctpHeader *sctpmsg = new SctpHeader();
    sctpmsg->setByteLength(SCTP_COMMON_HEADER);
    SctpErrorChunk *errorChunk = new SctpErrorChunk("NatError");
    errorChunk->setChunkType(ERRORTYPE);
    errorChunk->setByteLength(4);
    SctpSimpleErrorCauseParameter *cause = new SctpSimpleErrorCauseParameter("Cause");
    cause->setParameterType(MISSING_NAT_ENTRY);
    cause->setByteLength(4);
    cause->encapsulate(dgram->dup());
    errorChunk->setMBit(true);
    errorChunk->setTBit(true);
    errorChunk->addParameters(cause);
    sctpmsg->insertSctpChunks(errorChunk);

    SctpHeader *oldmsg = check_and_cast<SctpHeader *>(dgram->decapsulate());
    sctpmsg->setSrcPort(oldmsg->getDestPort());
    sctpmsg->setDestPort(oldmsg->getSrcPort());
    sctpmsg->setTag(oldmsg->getTag());
    sctpmsg->setChecksumOk(true);
    dgram->removeControlInfo();
    dgram->setName(sctpmsg->getName());
    dgram->setByteLength(IP_HEADER_BYTES);
    dgram->encapsulate(sctpmsg);
    IPv4Address tmpaddr = dgram->getDestAddress();
    dgram->setDestAddress(dgram->getSrcAddress());
    if (!tmpaddr.isUnspecified())
        dgram->setSrcAddress(tmpaddr);
    dgram->setProtocolId(IP_PROT_SCTP);
    delete oldmsg;
}

void SctpNatHook::finish()
{
    if (isRegisteredHook())
        ipLayer->unregisterHook(this);
    ipLayer = nullptr;
    EV_INFO<< getFullPath() << ": Natted packets: " << nattedPackets << "\n";
}
#endif
} // namespace sctp

} // namespace inet

