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
#include "inet/transportlayer/dtls/Dtls12RecordProtection.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

void DtlsAssociation::refuseRenegotiation12()
{
    auto& owner = engine;
    auto handle = id;
    auto eligible = [&owner, handle]() {
        auto it = owner.associations.find(handle);
        return it != owner.associations.end() && !it->second->closing && it->second->state == HandshakeState::ESTABLISHED;
    };
    auto delay = owner.processingDelay(handle, "protectOutDelay");
    if (!owner.enqueueProcessing(owner.outputProcessing.get(), handle, 39, delay, [&owner, handle]() {
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

bool Dtls::receiveRecord12(Packet *packet, const DtlsPath& path, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted)
{
    auto first=packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
    if (first<20 || first>24 || packet->getDataLength()<B(13))
        return false;
    auto header=packet->peekAtFront<DtlsPlaintextHeader>(B(13));
    AssociationId handle=0;
    bool chooseLegacy=false;
    for (const auto& entry : associations) {
        const auto& value=*entry.second;
        if (value.peerAddress!=path.peerAddress || value.peerPort!=path.peerPort ||
            (!value.localAddress.isUnspecified() && value.localAddress!=path.localAddress) ||
            (value.localPort>=0 && value.localPort!=path.localPort)) continue;
        if (value.isDtls12()) { handle=entry.first; break; }
        if (allowDtls12 && !value.serverSide && value.state==HandshakeState::CLIENT_WAIT_SERVER_HELLO &&
            !value.retried && first==22 && header->getEpoch()==0 && header->getLength()>=12 &&
            packet->getDataLength()>=B(25) && packet->peekDataAt<DtlsHandshakeHeader>(B(13),B(12))->getMessageType()==3) {
            handle=entry.first;
            chooseLegacy=true;
            break;
        }
        if (allowDtls12 && !value.serverSide && value.state==HandshakeState::CLIENT_WAIT_SERVER_HELLO &&
            !value.retried && first==22 && header->getEpoch()==0 && header->getLength()>=12 && packet->getDataLength()>=B(25)) {
            auto hs=packet->peekDataAt<DtlsHandshakeHeader>(B(13),B(12));
            if (hs->getMessageType()==2 && hs->getFragmentOffset()==0 && hs->getFragmentLength()==hs->getMessageLength() &&
                hs->getFragmentLength()+12<=header->getLength() && packet->getDataLength()>=B(25+hs->getFragmentLength())) {
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
    if (first==22 && header->getEpoch()==0 && header->getLength()>=12 && packet->getDataLength()>=B(25) &&
        current->second->serverSide && packet->peekDataAt<DtlsHandshakeHeader>(B(13),B(12))->getMessageType()==1)
        return false; // New attempts and duplicate initial Hellos use bounded prevalidation.
    packet->popAtFront<DtlsPlaintextHeader>(B(13));
    auto length=B(header->getLength());
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
    bool hvr=header->getEpoch()==0 && first==22 && (*retained)->getDataLength()>=B(12) &&
        (*retained)->peekAtFront<DtlsHandshakeHeader>(B(12))->getMessageType()==3;
    if (header->getLegacyVersion()!=Association::DTLS_12 && !(hvr && header->getLegacyVersion()==0xfeff)) {
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
    auto epoch=header->getEpoch();
    auto number=header->getSequenceNumber();
    bool protectedRecord=epoch==1;
    if (epoch>1 || (epoch==0 && (first==23 || first==24)) ||
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
    if (!enqueueProcessing(inputProcessing.get(),handle,header->getLength()+13,delay,[this,handle,first,epoch,number,protectedRecord,retained]() {
        auto it=associations.find(handle);
        if (it==associations.end()) return;
        auto& value=*it->second;
        using Phase=Association::LegacyHandshake::Phase;
        bool readActive=value.legacyHandshake->phase==Phase::CLIENT_WAIT_SERVER_FINISHED ||
            value.legacyHandshake->phase==Phase::SERVER_WAIT_CLIENT_FINISHED || value.legacyHandshake->phase==Phase::COMPLETE;
        if (protectedRecord) {
            if (!readActive) { emit(recordDroppedSignal,long(UNKNOWN_EPOCH)); return; }
            if (!value.receiveEpoch3->isAcceptable(number)) { emit(recordDroppedSignal,long(REPLAY)); return; }
            if (!Dtls12RecordProtection::unprotect(retained->get(),1,number)) { emit(recordDroppedSignal,long(MALFORMED_RECORD)); return; }
            value.receiveEpoch3->accept(number);
        }
        else if (readActive && first==21) {
            emit(recordDroppedSignal,long(UNKNOWN_EPOCH)); return;
        }
        emit(recordAcceptedSignal,long(epoch));
        it=associations.find(handle);
        if (it==associations.end()) return;
        try {
            if (first==20) {
                if (epoch!=0 || (*retained)->getDataLength()!=B(1)) { failAssociation(handle,"Invalid DTLS 1.2 CCS",50); return; }
                if ((*retained)->popAtFront<Dtls12ChangeCipherSpec>(B(1))->getValue() != 1) {
                    failAssociation(handle,"Invalid DTLS 1.2 CCS value",50);
                    return;
                }
                it->second->receiveChangeCipherSpec12();
            }
            else if (first==22)
                it->second->receiveHandshake12(retained->get(),epoch);
            else if (first==23) {
                if (it->second->state!=HandshakeState::ESTABLISHED) { emit(recordDroppedSignal,long(UNKNOWN_EPOCH)); return; }
                auto callback=it->second->callback;
                auto incoming=retained->release();
                emit(packetReceivedSignal,incoming);
                emit(packetSentToUpperSignal,incoming);
                drop(incoming);
                callback->dataArrived(handle,incoming);
            }
            else if (first==21) {
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
