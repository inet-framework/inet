// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include <limits>
#include "inet/common/Simsignals.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"
#include "inet/transportlayer/dtls/Dtls12CidHeader_m.h"
#include "inet/transportlayer/dtls/Dtls12RecordProtection.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

Ptr<Chunk> DtlsAssociation::makeRecordHeader12(uint8_t type, uint16_t epoch, uint64_t number, uint16_t length, const std::vector<uint8_t>& cid)
{
    if (epoch != 0 && !cid.empty()) {
        auto header = makeShared<Dtls12CidHeader>();
        header->setEpoch(epoch);
        header->setSequenceNumber(number);
        header->setLength(length);
        header->setConnectionIdArraySize(cid.size());
        for (size_t i = 0; i < cid.size(); ++i)
            header->setConnectionId(i, cid[i]);
        header->setChunkLength(B(13 + cid.size()));
        return header;
    }
    auto header = makeShared<DtlsPlaintextHeader>();
    header->setContentType(type);
    header->setEpoch(epoch);
    header->setSequenceNumber(number);
    header->setLength(length);
    return header;
}

void DtlsAssociation::refuseRenegotiation12()
{
    auto& owner = engine;
    auto handle = id;
    auto eligible = [&owner, handle]() {
        auto it = owner.associations.find(handle);
        return it != owner.associations.end() && !it->second->closing && it->second->state == HandshakeState::ESTABLISHED;
    };
    auto delay = owner.processingDelay(handle, "protectOutDelay");
    if (!owner.enqueueProcessing(owner.outputProcessing.get(), handle, recordOverhead() + 2, delay, [&owner, handle]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end())
            return;
        uint64_t number;
        if (!it->second->allocateApplicationRecord(number)) {
            owner.failAssociation(handle, "DTLS 1.2 warning record-use bound reached");
            return;
        }
        auto body = makeShared<DtlsAlert>();
        body->setLevel(1);
        body->setDescription(100);
        auto packet = new Packet("DTLS no_renegotiation", body);
        it->second->protectRecord(packet, 21, 1, number);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!it->second->localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
        if (it->second->localInterfaceId >= 0)
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(it->second->localInterfaceId);
        auto peer = it->second->peerAddress;
        auto port = it->second->peerPort;
        owner.emit(Dtls::packetSentToLowerSignal, packet);
        owner.sendUdp(packet, peer, port);
    }, eligible))
        owner.failAssociation(handle, "DTLS 1.2 warning output queue is full");
}

bool Dtls::receiveRecord12(Packet *packet, const DtlsPath& path, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted, AssociationId& datagramAssociation)
{
    auto first=packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
    bool cidRecord=first==25;
    auto headerBytes=cidRecord ? size_t(21) : size_t(13);
    if (first<20 || first>25)
        return false;
    if (packet->getDataLength()<B(headerBytes)) {
        if (!cidRecord) return false;
        packet->removeData(); emit(recordDroppedSignal,long(MALFORMED_RECORD)); return true;
    }
    uint16_t version,epoch,recordLength;
    uint64_t number;
    std::vector<uint8_t> cid;
    if (cidRecord) {
        auto header=packet->peekAtFront<Dtls12CidHeader>(B(headerBytes));
        version=header->getLegacyVersion(); epoch=header->getEpoch(); number=header->getSequenceNumber(); recordLength=header->getLength();
        for (size_t i=0;i<header->getConnectionIdArraySize();++i) cid.push_back(header->getConnectionId(i));
    }
    else {
        auto header=packet->peekAtFront<DtlsPlaintextHeader>(B(13));
        version=header->getLegacyVersion(); epoch=header->getEpoch(); number=header->getSequenceNumber(); recordLength=header->getLength();
    }
    AssociationId handle=0;
    bool chooseLegacy=false;
    if (cidRecord) {
        auto index=cidIndex.find(cid);
        if (index!=cidIndex.end()) {
            auto value=associations.find(index->second);
            if (value!=associations.end() && value->second->isDtls12()) handle=value->first;
        }
        if (handle==0) {
            auto bytes=B(headerBytes+recordLength);
            if (bytes<=packet->getDataLength()) packet->removeAtFront(bytes); else packet->removeData();
            emit(recordDroppedSignal,long(UNKNOWN_CID)); return true;
        }
    }
    else for (const auto& entry : associations) {
        const auto& value=*entry.second;
        if (value.peerAddress!=path.peerAddress || value.peerPort!=path.peerPort ||
            (!value.localAddress.isUnspecified() && value.localAddress!=path.localAddress) ||
            (value.localPort>=0 && value.localPort!=path.localPort)) continue;
        if (value.isDtls12()) { handle=entry.first; break; }
        if (allowDtls12 && !value.serverSide && value.state==HandshakeState::CLIENT_WAIT_SERVER_HELLO &&
            !value.retried && first==22 && epoch==0 && recordLength>=12 &&
            packet->getDataLength()>=B(25) && packet->peekDataAt<DtlsHandshakeHeader>(B(13),B(12))->getMessageType()==3) {
            handle=entry.first;
            chooseLegacy=true;
            break;
        }
        if (allowDtls12 && !value.serverSide && value.state==HandshakeState::CLIENT_WAIT_SERVER_HELLO &&
            !value.retried && first==22 && epoch==0 && recordLength>=12 && packet->getDataLength()>=B(25)) {
            auto hs=packet->peekDataAt<DtlsHandshakeHeader>(B(13),B(12));
            if (hs->getMessageType()==2 && hs->getFragmentOffset()==0 && hs->getFragmentLength()==hs->getMessageLength() &&
                hs->getFragmentLength()+12<=recordLength && packet->getDataLength()>=B(25+hs->getFragmentLength())) {
                auto hello=packet->peekDataAt<tls::TlsServerHello>(B(25),B(hs->getFragmentLength()));
                auto suite=hello->getCipherSuite();
                if (hello->getLegacyVersion()==Association::DTLS_12 &&
                    (suite==Association::PSK_AES_128_GCM_SHA256 || suite==Association::ECDHE_ECDSA_AES_128_GCM_SHA256) &&
                    std::find(value.offer.cipherSuites.begin(),value.offer.cipherSuites.end(),suite)!=value.offer.cipherSuites.end()) {
                    handle=entry.first;
                    chooseLegacy=true;
                    break;
                }
            }
        }
    }
    if (handle==0)
        return false;
    auto current=associations.find(handle);
    if (first==22 && epoch==0 && recordLength>=12 && packet->getDataLength()>=B(25) &&
        current->second->serverSide && packet->peekDataAt<DtlsHandshakeHeader>(B(13),B(12))->getMessageType()==1)
        return false; // New attempts and duplicate initial Hellos use bounded prevalidation.
    if (datagramAssociation!=0 && datagramAssociation!=handle) {
        packet->removeData(); emit(recordDroppedSignal,long(MALFORMED_RECORD)); return true;
    }
    datagramAssociation=handle;
    packet->removeAtFront(B(headerBytes));
    auto length=B(recordLength);
    if (length>packet->getDataLength()) {
        packet->removeData();
        emit(recordDroppedSignal,long(MALFORMED_RECORD));
        return true;
    }
    auto offset=packet->getFrontOffset();
    auto tags=packet->getRegionTags();
    auto retained=std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS 1.2 record"));
    if (length>b(0)) {
        (*retained)->insertData(packet->removeAtFront(length));
        (*retained)->getRegionTags().copyTags(tags,offset,(*retained)->getFrontOffset(),length);
    }
    bool hvr=epoch==0 && first==22 && (*retained)->getDataLength()>=B(12) &&
        (*retained)->peekAtFront<DtlsHandshakeHeader>(B(12))->getMessageType()==3;
    if (version!=Association::DTLS_12 && !(hvr && version==0xfeff)) {
        emit(recordDroppedSignal,long(MALFORMED_RECORD));
        return true;
    }
    if (chooseLegacy) {
        current->second->initializeDtls12(false,0);
        current=associations.find(handle);
        if (current==associations.end()) return true;
    }
    auto& association=*current->second;
    if (association.localAddress.isUnspecified()) association.localAddress=path.localAddress;
    if (association.localPort<0) association.localPort=path.localPort;
    if (association.serverSide && !*creditCounted) {
        auto maximum=std::numeric_limits<size_t>::max();
        association.receivedBytes=datagramBytes>maximum-association.receivedBytes ? maximum : association.receivedBytes+datagramBytes;
        *creditCounted=true;
    }
    bool protectedRecord=epoch==1;
    if (epoch>1 || (epoch==0 && (first==23 || first==24 || cidRecord)) ||
        (epoch==0 && first==22 && association.state==HandshakeState::ESTABLISHED)) {
        emit(recordDroppedSignal,long(UNKNOWN_EPOCH));
        return true;
    }
    if (length>B(Dtls12RecordProtection::MAX_CONTENT_BYTES+(protectedRecord?24:0))) {
        emit(recordDroppedSignal,long(MALFORMED_RECORD));
        return true;
    }
    auto delay=protectedRecord?processingDelay(handle,"protectInDelay"):SIMTIME_ZERO;
    auto eligible=[this,handle]() { auto it=associations.find(handle); return it!=associations.end() && !it->second->closing; };
    if (!enqueueProcessing(inputProcessing.get(),handle,recordLength+headerBytes,delay,[this,handle,first,epoch,number,protectedRecord,retained,cidRecord,path,headerBytes,recordLength]() {
        auto it=associations.find(handle);
        if (it==associations.end()) return;
        auto& value=*it->second;
        using Phase=Association::LegacyHandshake::Phase;
        bool readActive=value.legacyHandshake->phase==Phase::CLIENT_WAIT_SERVER_FINISHED ||
            value.legacyHandshake->phase==Phase::SERVER_WAIT_CLIENT_FINISHED || value.legacyHandshake->phase==Phase::COMPLETE;
        uint8_t contentType=first;
        bool fresh=protectedRecord && value.isNewerRecord(epoch,number);
        bool changed=!sameTuple(path,value.currentPath());
        if (protectedRecord) {
            if (cidRecord ? !value.cidNegotiated || value.localCid.empty() : value.cidNegotiated && !value.localCid.empty()) {
                emit(recordDroppedSignal,long(UNKNOWN_CID)); return;
            }
            if (!readActive) { emit(recordDroppedSignal,long(UNKNOWN_EPOCH)); return; }
            if (!value.receiveEpoch3->isAcceptable(number)) { emit(recordDroppedSignal,long(REPLAY)); return; }
            bool valid=cidRecord ? Dtls12RecordProtection::unprotectCid(retained->get(),1,number,contentType) :
                Dtls12RecordProtection::unprotect(retained->get(),1,number);
            if (!valid) { emit(recordDroppedSignal,long(MALFORMED_RECORD)); return; }
            if (changed) {
                if (!cidRecord || !value.considerPath(path,fresh)) { emit(recordDroppedSignal,long(UNSUPPORTED_PATH)); return; }
                it=associations.find(handle);
                if (it==associations.end()) return;
            }
            if (contentType!=24) it->second->receiveEpoch3->accept(number);
        }
        else if (readActive && first==21) {
            emit(recordDroppedSignal,long(UNKNOWN_EPOCH)); return;
        }
        emit(recordAcceptedSignal,long(epoch));
        it=associations.find(handle);
        if (it==associations.end()) return;
        if (contentType==24) {
            if (it->second->state==HandshakeState::ESTABLISHED)
                it->second->receiveHeartbeat(epoch,number,retained->get(),path,fresh,changed,headerBytes+recordLength);
            return;
        }
        try {
            if (contentType==20) {
                if (epoch!=0 || (*retained)->getDataLength()!=B(1)) { failAssociation(handle,"Invalid DTLS 1.2 CCS",50); return; }
                if ((*retained)->popAtFront<Dtls12ChangeCipherSpec>(B(1))->getValue() != 1) {
                    failAssociation(handle,"Invalid DTLS 1.2 CCS value",50);
                    return;
                }
                it->second->receiveChangeCipherSpec12();
            }
            else if (contentType==22)
                it->second->receiveHandshake12(retained->get(),epoch);
            else if (contentType==23) {
                if (it->second->state!=HandshakeState::ESTABLISHED) { emit(recordDroppedSignal,long(UNKNOWN_EPOCH)); return; }
                auto callback=it->second->callback;
                auto incoming=retained->release();
                emit(packetReceivedSignal,incoming);
                emit(packetSentToUpperSignal,incoming);
                drop(incoming);
                callback->dataArrived(handle,incoming);
            }
            else if (contentType==21) {
                if ((*retained)->getDataLength()!=B(2)) { failAssociation(handle,"Invalid DTLS 1.2 alert",50); return; }
                auto alert=(*retained)->popAtFront<DtlsAlert>(B(2));
                if (alert->getDescription()==0) beginClose(handle);
                else if (alert->getLevel()==2) failAssociation(handle,"Peer DTLS 1.2 fatal alert");
            }
            else
                emit(recordDroppedSignal,long(UNKNOWN_EPOCH));
        }
        catch (const cRuntimeError&) { failAssociation(handle,"Malformed DTLS 1.2 record",50); }
    },eligible))
        failAssociation(handle,"DTLS 1.2 input processor is full");
    return true;
}

} // namespace inet::dtls
