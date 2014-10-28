#include "IPv4ControlInfo.h"
#include "IPv4.h"
#include "SCTPNatHook.h"
#include "RoutingTableAccess.h"
#include "IPv4InterfaceData.h"
#include "SCTPAssociation.h"

Define_Module(SCTPNatHook);

SCTPNatHook::SCTPNatHook()
{
    ipLayer = NULL;
}

SCTPNatHook::~SCTPNatHook()
{
    ipLayer = check_and_cast_nullable<IPv4*>(getModuleByPath("^.ip"));
    if (ipLayer)
        ipLayer->unregisterHook(0, this);
}

void SCTPNatHook::initialize()
{
    RoutingTableAccess routingTableAccess;
    InterfaceTableAccess interfaceTableAccess;

    ipLayer = check_and_cast<IPv4*>(getParentModule()->getSubmodule("networkLayer")->getSubmodule("ip"));
    rt = routingTableAccess.get();
    ift = interfaceTableAccess.get();
    natTable = new SCTPNatTable();
    nattedPackets = 0;

    ipLayer->registerHook(0, this);
}

INetfilter::IHook::Result SCTPNatHook::datagramForwardHook(IPv4Datagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr)
{
    SCTPNatEntry* entry;
    SCTPChunk* chunk;
    IPv4Datagram *dgram;

    dgram = dynamic_cast<IPv4Datagram *>(datagram);
    if (SCTPAssociation::getAddressLevel(dgram->getSrcAddress())!=3) {
        return INetfilter::IHook::ACCEPT;
    }
    natTable->printNatTable();
    SCTPMessage* sctpMsg = check_and_cast<SCTPMessage*>(dgram->getEncapsulatedPacket());
    unsigned int numberOfChunks=sctpMsg->getChunksArraySize();
    if (numberOfChunks==1)
        chunk = (SCTPChunk*)(sctpMsg->peekFirstChunk());
    else
        chunk = (SCTPChunk*)(sctpMsg->peekLastChunk());
    sctpEV3<<"findEntry for "<<dgram->getSrcAddress()<<":"<<sctpMsg->getSrcPort()<<" to "<<dgram->getDestAddress()<<":"<<sctpMsg->getDestPort()<<" vTag="<<sctpMsg->getTag()<<" natAddr="<<outIE->ipv4Data()->getIPAddress()<<"\n";
    if (chunk->getChunkType() == INIT || chunk->getChunkType() == INIT_ACK || chunk->getChunkType() == ASCONF)
    {
        entry = new SCTPNatEntry();
        entry->setLocalAddress(dgram->getSrcAddress());
        entry->setLocalPort(sctpMsg->getSrcPort());
        entry->setGlobalAddress(dgram->getDestAddress());
        entry->setGlobalPort(sctpMsg->getDestPort());
        entry->setNattedAddress(outIE->ipv4Data()->getIPAddress());
        entry->setNattedPort(sctpMsg->getSrcPort());
        entry->setGlobalVTag(sctpMsg->getTag());
        if (chunk->getChunkType() == INIT)
        {
            SCTPInitChunk* initChunk=check_and_cast<SCTPInitChunk*>(chunk);
            entry->setLocalVTag(initChunk->getInitTag());
        }
        else if (chunk->getChunkType() == INIT_ACK)
        {
            SCTPInitAckChunk* initAckChunk=check_and_cast<SCTPInitAckChunk*>(chunk);
            entry->setLocalVTag(initAckChunk->getInitTag());
        }
        else if (chunk->getChunkType() == ASCONF)
        {
            SCTPAsconfChunk* asconfChunk=check_and_cast<SCTPAsconfChunk*>(chunk);
            entry->setLocalVTag(asconfChunk->getPeerVTag());
        }
        dgram->setSrcAddress(outIE->ipv4Data()->getIPAddress());
        sctpMsg->setSrcPort(entry->getNattedPort());
        natTable->natEntries.push_back(entry);
    }
    else
    {
        sctpEV3<<"other chunkType: "<<(chunk->getChunkType()==COOKIE_ECHO?"Cookie_Echo":"other")<<", VTag="<<sctpMsg->getTag()<<"\n";
        entry = natTable->findNatEntry(dgram->getSrcAddress(),sctpMsg->getSrcPort(),dgram->getDestAddress(),sctpMsg->getDestPort(), sctpMsg->getTag());
        if (entry==NULL)
        {
            sctpEV3<<"no entry found\n";
            entry = natTable->findNatEntry(dgram->getSrcAddress(),sctpMsg->getSrcPort(),dgram->getDestAddress(),sctpMsg->getDestPort(), 0);
            if (entry==NULL)
            {
                sctpEV3<<"send back error message dgram=" << dgram <<"\n";
                sendBackError(dgram);
                nextHopAddr = dgram->getDestAddress();
                const InterfaceEntry* tmpIE = inIE;
                inIE = outIE;
                outIE = tmpIE;
                return INetfilter::IHook::ACCEPT;
            }
            else
            {
                sctpEV3<<"VTag doesn't match: old VTag="<<entry->getLocalVTag()<<", new VTag="<<sctpMsg->getTag()<<"\n";
                entry->setLocalVTag(sctpMsg->getTag());
                dgram->setSrcAddress(outIE->ipv4Data()->getIPAddress());
                sctpMsg->setSrcPort(entry->getNattedPort());
                sctpEV3<<"srcAddress set to "<<dgram->getSrcAddress()<<", srcPort set to "<<sctpMsg->getSrcPort()<<"\n";
            }
        }
        else
        {
            dgram->setSrcAddress(outIE->ipv4Data()->getIPAddress());
            sctpMsg->setSrcPort(entry->getNattedPort());
            sctpEV3<<"srcAddress set to "<<dgram->getSrcAddress()<<", srcPort set to "<<sctpMsg->getSrcPort()<<"\n";
        }

    }
    nattedPackets++;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SCTPNatHook::datagramPreRoutingHook(IPv4Datagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr)
{
    SCTPNatEntry* entry;
    SCTPChunk* chunk;
    IPv4Datagram *dgram;

    dgram = dynamic_cast<IPv4Datagram *>(datagram);
    if (SCTPAssociation::getAddressLevel(dgram->getSrcAddress())==3) {
        return INetfilter::IHook::ACCEPT;
    }
    natTable->printNatTable();
    bool local = ((rt->isLocalAddress(dgram->getDestAddress()) & SCTPAssociation::getAddressLevel(dgram->getSrcAddress()))==3);
    SCTPMessage* sctpMsg = check_and_cast<SCTPMessage*>(dgram->getEncapsulatedPacket());
    unsigned int numberOfChunks=sctpMsg->getChunksArraySize();
    if (numberOfChunks==1)
        chunk = (SCTPChunk*)(sctpMsg->peekFirstChunk());
    else
        chunk = (SCTPChunk*)(sctpMsg->peekLastChunk());
    if (!local)
    {
        entry = natTable->getEntry(dgram->getSrcAddress(),sctpMsg->getSrcPort(),dgram->getDestAddress(),sctpMsg->getDestPort(), sctpMsg->getTag());
        sctpEV3<<"getEntry for "<<dgram->getSrcAddress()<<":"<<sctpMsg->getSrcPort()<<" to "<<dgram->getDestAddress()<<":"<<sctpMsg->getDestPort()<<" peerVTag="<<sctpMsg->getTag()<<"\n";
        uint32 numberOfChunks = sctpMsg->getChunksArraySize();
        if (entry==NULL)
        {
            sctpEV3<<"no entry found\n";
            if (numberOfChunks==1)
                chunk = (SCTPChunk*)(sctpMsg->peekFirstChunk());
            else
                chunk = (SCTPChunk*)(sctpMsg->peekLastChunk());
            if (chunk->getChunkType()==INIT || chunk->getChunkType()==ASCONF)
            {
                sctpEV3<<"could be an Init collision\n";
                entry=natTable->getSpecialEntry(dgram->getSrcAddress(),sctpMsg->getSrcPort(),dgram->getDestAddress(),sctpMsg->getDestPort());
                if (entry!=NULL)
                {
                    if (chunk->getChunkType() == INIT)
                    {
                        SCTPInitChunk* initChunk=check_and_cast<SCTPInitChunk*>(chunk);
                        entry->setLocalVTag(initChunk->getInitTag());
                        sctpEV3<<"InitTag="<<initChunk->getInitTag()<<"\n";
                    }
                    else if (chunk->getChunkType() == ASCONF)
                    {
                        SCTPAsconfChunk* asconfChunk=check_and_cast<SCTPAsconfChunk*>(chunk);
                        entry->setLocalVTag(asconfChunk->getPeerVTag());
                    }
                    dgram->setDestAddress(entry->getLocalAddress().get4());
                    sctpMsg->setDestPort(entry->getLocalPort());
                    sctpEV3<<"destAddress set to "<<dgram->getDestAddress()<<", destPort set to "<<sctpMsg->getDestPort()<<"\n";
                }
                else
                {
                    return INetfilter::IHook::DROP;
                }
            }
            else
            {
                SCTPChunk* schunk;
                if (numberOfChunks>0)
                {
                    sctpEV3<<"number of chunks="<<numberOfChunks<<"\n";
                    for (uint32 i=0; i<numberOfChunks; i++)
                    {
                        schunk = (SCTPChunk*)(sctpMsg->removeChunk());
                        if (schunk->getChunkType()==DATA)
                            delete (SCTPSimpleMessage*)schunk->decapsulate();
                        sctpEV3<<"delete chunk "<<schunk->getName()<<"\n";
                        delete schunk;
                    }
                }
                return INetfilter::IHook::DROP;
            }
        }
        else
        {
            dgram->setDestAddress(entry->getLocalAddress().get4());
            sctpMsg->setDestPort(entry->getLocalPort());
            if (entry->getGlobalVTag()==0 && chunk->getChunkType()==INIT_ACK)
            {
                SCTPInitAckChunk* initAckChunk=check_and_cast<SCTPInitAckChunk*>(chunk);
                entry->setGlobalVTag(initAckChunk->getInitTag());
            }
            sctpEV3<<"destAddress set to "<<dgram->getDestAddress()<<", destPort set to "<<sctpMsg->getDestPort()<<"\n";
        }
    }
    else
    {
        if (chunk->getChunkType()==INIT)
        {
            sctpEV3<<"getLocALEntry for "<<dgram->getSrcAddress()<<":"<<sctpMsg->getSrcPort()<<" to "<<dgram->getDestAddress()<<":"<<sctpMsg->getDestPort()<<" peerVTag="<<sctpMsg->getTag()<<"\n";
            entry = natTable->getLocalInitEntry(dgram->getDestAddress(), sctpMsg->getSrcPort(), sctpMsg->getDestPort());
            if (entry == NULL)
            {
                entry = new SCTPNatEntry();
                entry->setLocalAddress(dgram->getSrcAddress());
                entry->setLocalPort(sctpMsg->getSrcPort());
                entry->setGlobalAddress(dgram->getDestAddress());
                entry->setGlobalPort(sctpMsg->getDestPort());
                entry->setNattedPort(sctpMsg->getSrcPort());
                entry->setNattedAddress(dgram->getDestAddress());
                SCTPInitChunk* initChunk=check_and_cast<SCTPInitChunk*>(chunk);
                entry->setGlobalVTag(initChunk->getInitTag());
                natTable->natEntries.push_back(entry);
                sctpEV3<<"added entry for local deliver\n";
                natTable->printNatTable();
                return INetfilter::IHook::DROP;
            }
            else
            {
                SCTPNatEntry *entry2 = new SCTPNatEntry();
                entry2->setLocalAddress(dgram->getSrcAddress());
                entry2->setLocalPort(sctpMsg->getSrcPort());
                entry2->setGlobalAddress(entry->getGlobalAddress());
                entry2->setGlobalPort(sctpMsg->getDestPort());
                entry2->setNattedPort(sctpMsg->getSrcPort());
                entry2->setNattedAddress(entry->getGlobalAddress());
                SCTPInitChunk* initChunk=check_and_cast<SCTPInitChunk*>(chunk);
                entry2->setGlobalVTag(initChunk->getInitTag());
                natTable->natEntries.push_back(entry2);
                dgram->setDestAddress(entry->getLocalAddress().get4());
                sctpMsg->setDestPort(entry->getLocalPort());
                dgram->setSrcAddress(entry->getGlobalAddress().get4());
                sctpMsg->setSrcPort(entry->getGlobalPort());
                sctpEV3<<"added additional entry for local deliver\n";
                natTable->printNatTable();
                sctpEV3<<"destAddress set to "<<dgram->getDestAddress()<<", destPort set to "<<sctpMsg->getDestPort()<<"\n";
            }
        }
        else
        {
            sctpEV3<<"no INIT: destAddr="<<dgram->getDestAddress()<<" destPort="<<sctpMsg->getDestPort()<<" srcPort="<<sctpMsg->getSrcPort()<<" vTag="<<sctpMsg->getTag()<<"\n";
            entry = natTable->getLocalEntry(dgram->getDestAddress(), sctpMsg->getSrcPort(), sctpMsg->getDestPort(), sctpMsg->getTag());
            if (entry!=NULL)
            {
                dgram->setDestAddress(entry->getLocalAddress().get4());
                sctpMsg->setDestPort(entry->getLocalPort());
                dgram->setSrcAddress(entry->getGlobalAddress().get4());
                sctpMsg->setSrcPort(entry->getGlobalPort());
            }
            else
            {
                sctpEV3<<"no entry found\n";
                return INetfilter::IHook::DROP;
            }
        }
    }
    nattedPackets++;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SCTPNatHook::datagramPostRoutingHook(IPv4Datagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SCTPNatHook::datagramLocalInHook(IPv4Datagram* datagram, const InterfaceEntry* inIE)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result SCTPNatHook::datagramLocalOutHook(IPv4Datagram* datagram, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}


void SCTPNatHook::sendBackError(IPv4Datagram* dgram)
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setByteLength(SCTP_COMMON_HEADER);
    SCTPErrorChunk* errorChunk = new SCTPErrorChunk("NatError");
    errorChunk->setChunkType(ERRORTYPE);
    errorChunk->setByteLength(4);
    SCTPSimpleErrorCauseParameter* cause = new SCTPSimpleErrorCauseParameter("Cause");
    cause->setParameterType(MISSING_NAT_ENTRY);
    cause->setByteLength(4);
    cause->encapsulate(dgram->dup());
    errorChunk->setMBit(true);
    errorChunk->setTBit(true);
    errorChunk->addParameters(cause);
    sctpmsg->addChunk(errorChunk);

    SCTPMessage *oldmsg = check_and_cast<SCTPMessage*>(dgram->decapsulate());
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
    dgram->setTransportProtocol(IP_PROT_SCTP);
    delete oldmsg;
}

void SCTPNatHook::finish()
{
    if (ipLayer)
        ipLayer->unregisterHook(0, this);
    ipLayer = NULL;
    delete natTable;
    std::cout<<getFullPath()<<": Natted packets: "<<nattedPackets<<"\n";
}
